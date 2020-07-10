#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::beginNodeEdit()
{
  // Everytime we start to do a lot of changes to the tree, we memorize if the root node was expanded
  // and collapse it for performance reasons (so any redraw / update call is processed way faster)
  if (nodeEditStarted)
    return;

  nodeEditStarted = true;

  if (ui->nodeTreeView->verticalScrollBar() != nullptr && ui->nodeTreeView->verticalScrollBar()->maximum() > 0)
    nodeEditLastScrollbarPos = float(ui->nodeTreeView->verticalScrollBar()->value()) / float(ui->nodeTreeView->verticalScrollBar()->maximum());

  QModelIndex index;
  for (int i = 0; i < nodeEditor.getStandardModel().invisibleRootItem()->rowCount(); ++i) {
    index = nodeEditor.getFilteredModel().mapFromSource(nodeEditor.getStandardModel().invisibleRootItem()->child(i, NodeTreeColumns::KeyColumn)->index());
    nodeEditTreeExpansionStates.append(index.isValid() && ui->nodeTreeView->isExpanded(index));
    ui->nodeTreeView->collapse(index);
  }
}

void MainWindow::closeTrack()
{
  // Set the default window title
  setWindowTitle(defaultWindowTitle);

  // Load an empty track as a placeholder
  loadedTrack = TrackData();

  // Clear all track related controls
  nodeEditor.getStandardModel().clear();
  ui->searchValueComboBox->clear();
  ui->sceneComboBox->clear();
  ui->replacePrefabComboBox->clear();
  ui->replacePrefabWithComboBox->clear();

  // Clear the last search result
  lastSearchResult.clear();

  // Reset the node counter and upadte the status bar
  updateStatusBar();
}

void MainWindow::endNodeEdit()
{
  // Once we have finished our changes we restore the original view and move the scrollbar to its previous location
  if (!nodeEditStarted)
    return;

  nodeEditStarted = false;

  QModelIndex index;
  for (int i = 0; i < nodeEditor.getStandardModel().invisibleRootItem()->rowCount(); ++i) {
    if (i >= nodeEditTreeExpansionStates.count())
      break;

    index = nodeEditor.getFilteredModel().mapFromSource(nodeEditor.getStandardModel().invisibleRootItem()->child(i, NodeTreeColumns::KeyColumn)->index());
    if (nodeEditTreeExpansionStates[i])
      ui->nodeTreeView->expand(index);
  }

  if (ui->nodeTreeView->verticalScrollBar() != nullptr && ui->nodeTreeView->verticalScrollBar()->maximum() > 0)
    ui->nodeTreeView->verticalScrollBar()->setValue(int(std::round(nodeEditLastScrollbarPos * ui->nodeTreeView->verticalScrollBar()->maximum())));

  nodeEditTreeExpansionStates.clear();
}

void MainWindow::loadTrack(const TrackData& track)
{
  // Close any currently loaded track
  closeTrack();

  // Set the loaded track and get its assigned database
  loadedTrack = track;
  VeloDb* veloDb = getDatabase();

  updateWindowTitle();

  // Load the prefabs from the database to the track
  nodeEditor.setPrefabs(veloDb->getPrefabs());

  // Import the json from the loaded track
  try {
    nodeEditor.importJsonData(&loadedTrack.value);
  } catch (VeloToolkitException& e) {
    e.Message();
  }

  // Pass the model/items from the velo track to the tree view
  // and tell it to resize its columns to its content
  // and hide its type column if the setting is enabled
  ui->nodeTreeView->setModel(&nodeEditor.getFilteredModel());
  ui->nodeTreeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);
  if (!settingViewTypeColumn)
    ui->nodeTreeView->hideColumn(NodeTreeColumns::TypeColumn);

  // Load the scenes into the combo box
  foreach(SceneData scene, veloDb->getScenes()) {
    ui->sceneComboBox->addItem(scene.title, scene.id);
    if (scene.id == loadedTrack.sceneId)
      ui->sceneComboBox->setCurrentText(scene.title);
  }

  // Update the prefab controls so only the prefabs of the track are shown
  updatePrefabComboBoxes();  

  // Update filter if any set
  updateSearchFilter();

  // Update the status bar
  updateStatusBar();

  // Expand the root nodes
  for (int i = 0; i < nodeEditor.getStandardModel().invisibleRootItem()->rowCount(); ++i) {
    ui->nodeTreeView->expand(nodeEditor.getFilteredModel().mapFromSource(nodeEditor.getStandardModel().invisibleRootItem()->child(i, NodeTreeColumns::KeyColumn)->index()));
  }

  statusBar()->showMessage(tr("Track loaded successfully."), 2000);
}

void MainWindow::toolsReplaceObject() {
  // Do we got a valid track loaded?
  if (loadedTrack.id == 0)
    return;

  uint changedPrefabCount = 0;

  // Get the starting index for the search according to the user selection
  // Call the velo track replace function with the search index or prefabs and scaling
  switch (ui->toolsTargetComboBox->currentIndex()) {
  case 0: // All nodes
  case 1: { // Selected Node
    beginNodeEdit();
    changedPrefabCount = nodeEditor.replacePrefabs(ui->toolsTargetComboBox->currentIndex() == 0 ?
                                                     nodeEditor.getRootIndex() :
                                                     ui->nodeTreeView->currentIndex(),
                                                   ui->replacePrefabComboBox->currentData().toUInt(),
                                                   ui->replacePrefabWithComboBox->currentData().toUInt());
    endNodeEdit();
    break;
  }
  case 2: // Filtered Nodes
    beginNodeEdit();
    changedPrefabCount = nodeEditor.replacePrefabs(lastSearchResult,
                                                   ui->replacePrefabComboBox->currentData().toUInt(),
                                                   ui->replacePrefabWithComboBox->currentData().toUInt());
    endNodeEdit();
    break;

  default:
    return;
  }

  // Update the prefab combo boxes, because we may have added new prefab or removed one completely
  updatePrefabComboBoxes();

  QString changedPrefabInfo = tr("%1 occurence(s) replaced");
  QMessageBox::information(this, tr("Replace successfull"), changedPrefabInfo.arg(changedPrefabCount, 0, 10));
}

void MainWindow::saveTrackToDb()
{
  if (loadedTrack.id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  // Write the selected scene and any node changes to the currently loaded track
  loadedTrack.sceneId = ui->sceneComboBox->currentData().toUInt();
  loadedTrack.value = *nodeEditor.exportAsJsonData();

  // Message for later output
  QString message = tr("The track was saved successfully to the database!");

  // Modify the message in case we are saving as a new track
  if (settingSaveAsNew) {
    loadedTrack.name += "-new";
    message += tr("\n\nNew track name: ") + loadedTrack.name;

    // Since we got a new track name we need to update the window title
    updateWindowTitle();
  }

  try {
    // Write the track into the database and retrieves its id
    loadedTrack.id = getDatabase()->saveTrack(loadedTrack, settingSaveAsNew);

    // Reset the modified flat
    beginNodeEdit();
    nodeEditor.resetModifiedFlags();
    endNodeEdit();

    // Inform the user about the success
    QMessageBox::information(nullptr, tr("Save Track"), message);
    statusBar()->showMessage(tr("Track saved!"), 2000);
  } catch (VeloToolkitException& e) {
    // Something gone wrong while saving the track to the database
    // Message the user
    e.Message();
    return;
  }
}

void MainWindow::saveTrackToFile()
{
  if (loadedTrack.id == 0)
    return;

  // Create a new file (remove any already existing) and export the velo track json into it
  QFile *file = new QFile("track.json");
  file->remove();
  file->open(QFile::ReadWrite);
  file->write(*nodeEditor.exportAsJsonData());
  file->close();
}

void MainWindow::updateDynamicTabControlSize(int index)
{
  // Set the policy of all items inside the tab widget to be ignored, except the ones inside the currently selected tab
  // and tell the tab control to resize itself and adjust its size.
  // Since all other tabs controls are beeing ignored, the tab control size adjusts itself to the current tab only.
  for(int i=0;i<ui->nodeEditorToolsTabWidget->count();i++)
    if(i!=index)
      ui->nodeEditorToolsTabWidget->widget(i)->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

  ui->nodeEditorToolsTabWidget->widget(index)->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  ui->nodeEditorToolsTabWidget->widget(index)->resize(ui->nodeEditorToolsTabWidget->widget(index)->minimumSizeHint());
  ui->nodeEditorToolsTabWidget->widget(index)->adjustSize();
}

void MainWindow::updatePrefabComboBoxes()
{
  // Clear the combobox and insert all prefabs from the velo track
  ui->replacePrefabComboBox->clear();
  QVector<PrefabData> prefabsInUse = nodeEditor.getPrefabsInUse(true);
  foreach(PrefabData prefab, prefabsInUse)
  {
    ui->replacePrefabComboBox->addItem(prefab.name, prefab.id);
    ui->searchValueComboBox->addItem(prefab.name, prefab.id);
  }
}

void MainWindow::on_deleteTrackPushButton_released()
{
  // Do we got any track loaded?
  if (loadedTrack.id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  // Create a warning and double check
  QString message = tr("Do you really want to delete the track \"%1\"?\nMAKE SURE TO ENABLE YOUR BRAIN NOW!");
  QMessageBox::StandardButtons ret = QMessageBox::question(this, tr("Are you sure?"), message.arg(loadedTrack.name));
  if (ret != QMessageBox::Yes)
    return;

  // Delete the track
  try {
    getDatabase()->deleteTrack(loadedTrack);
  } catch (VeloToolkitException& e) {
    // Catch any funky stuff happening
    e.Message();
    closeTrack();
    return;
  }

  // We deleted it, so we probably don't want to have it still opened
  closeTrack();

  statusBar()->showMessage(tr("Track deleted."), 2000);
}

void MainWindow::on_geoGenTestPushButton_released()
{
  // Code left over from a (so far) failed try to create a geodesic dome with neon stripes
  for (int i = 0; i < getDatabase()->getTracks().count(); ++i) {
    if (getDatabase()->getTracks().at(i).name == "Geodome Test") {
      TrackData oldTrack = getDatabase()->getTracks().at(i);
      getDatabase()->deleteTrack(oldTrack);
    }
  }
  GeodesicDome dome(this, 2);
  TrackData track;
  track.sceneId = 16;
  track.name = "Geodome Test";
  track.value = dome.getVeloTrackDataTest();
  track.protectedTrack = 0;
  track.assignedDatabase = getDatabase()->getDatabaseType();
  track.id = getDatabase()->saveTrack(track);
  loadTrack(track);
}

void MainWindow::on_openTrackPushButton_released()
{
  try {
    // Show the open track dialog and get the user selection,
    // check for unwritten changes and eventually load the new track
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      if (maybeSave()) {
        TrackData selectedTrack = openTrackDialog.getSelectedTrack();
        if (selectedTrack.id > 0) {
          loadTrack(selectedTrack);
        }
      }
    }
  } catch (VeloToolkitException& e) {
    // Something weird happend. Close the track and give the user some info
    closeTrack();
    e.Message();
  }
}


void MainWindow::on_replacePrefabComboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  // Get the selected Prefab
  PrefabData selectedPrefab = nodeEditor.getPrefab(ui->replacePrefabComboBox->currentData().toUInt());

  // Clear the replacePrefabWithComboBox
  ui->replacePrefabWithComboBox->clear();

  // Load all prefabs into replacePrefabWithComboBox...
  foreach(PrefabData prefab, *nodeEditor.getPrefabData()) {
    // ... but only if its the same type, so you cant replace a gate with a barrier aso, which would probably break the track
    if (prefab.gate == selectedPrefab.gate) {
      ui->replacePrefabWithComboBox->addItem(prefab.name, prefab.id);

      // Preselect the with combo box with this prefab if its the same as the prefab we want to replace
      if (prefab.id == selectedPrefab.id) {
        ui->replacePrefabWithComboBox->setCurrentIndex(ui->replacePrefabWithComboBox->count() - 1);
      }
    }
  }
}

void MainWindow::on_toolsApplyPushButton_released()
{
  bool byPercent = ui->transformByComboBox->currentIndex() == 0;

  int toolTypeIndex = ui->toolsTypeComboBox->currentIndex();
  if (toolTypeIndex == 1) // Move
    toolTypeIndex = ToolTypes::Move + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 2) // Rotate
    toolTypeIndex = ToolTypes::AddRotation + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 3) // Scale
    toolTypeIndex = ToolTypes::Scale + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 4) // Mirror
    toolTypeIndex = ToolTypes::Mirror;

  qDebug() << "toolsTypeComboBox->currentIndex:" << ui->toolsTypeComboBox->currentIndex();
  qDebug() << "toolsSubtypeComboBox->currentIndex:" << ui->toolsSubtypeComboBox->currentIndex();
  qDebug() << "transformByComboBox->currentIndex:" << ui->transformByComboBox->currentIndex();
  qDebug() << "Transform Method Index: " << toolTypeIndex;

  QVariant value;
  switch (toolTypeIndex) {
  case ToolTypes::Replace:
    toolsReplaceObject();
    return;
  case ToolTypes::Move:
  case ToolTypes::IncreasingPosition:
  case ToolTypes::ReplacePosition:
  case ToolTypes::Scale:
  case ToolTypes::IncreasingScale:
  case ToolTypes::ReplaceScaling:
    value = QVector3D(float(ui->transformRDoubleSpinBox->value()),
                      float(ui->transformGDoubleSpinBox->value()),
                      float(ui->transformBDoubleSpinBox->value()));
    break;
  case ToolTypes::AddRotation:
  case ToolTypes::IncreasingRotation:
    value = QQuaternion::fromEulerAngles(ui->transformRotationRValueSpinBox->value(),
                                         ui->transformRotationGValueSpinBox->value(),
                                         ui->transformRotationBValueSpinBox->value());
    qDebug() << "Rotation from euler :" << ui->transformRotationRValueSpinBox->value() <<
        ui->transformRotationGValueSpinBox->value() <<
        ui->transformRotationBValueSpinBox->value() << "Quat:" << value;
    break;
  case ToolTypes::ReplaceRotation:
    value = QVector4D(float(ui->transformRotationWValueSpinBox->value()),
                      float(ui->transformRotationXValueSpinBox->value()),
                      float(ui->transformRotationYValueSpinBox->value()),
                      float(ui->transformRotationZValueSpinBox->value()));
    break;
  case ToolTypes::Mirror:
    break;
  default:
    return;    
  }

  uint changedNodeCount = 0;
  switch (ui->toolsTargetComboBox->currentIndex()) {
  case 0: // All nodes
    beginNodeEdit();
    changedNodeCount = nodeEditor.transformPrefab(nodeEditor.getRootIndex(), ToolTypes(toolTypeIndex), value, byPercent);
    endNodeEdit();
    break;

  case 1: // Selected Nodes
    beginNodeEdit();
    changedNodeCount = nodeEditor.transformPrefab(ui->nodeTreeView->selectionModel()->selectedIndexes(), ToolTypes(toolTypeIndex), value, byPercent);
    endNodeEdit();
    break;

  case 2: // Filtered Nodes
    beginNodeEdit();
    changedNodeCount = nodeEditor.transformPrefab(lastSearchResult, ToolTypes(toolTypeIndex), value, byPercent);
    endNodeEdit();
    break;

  default:
    return;
  }

  QString changedPrefabInfo = tr("%1 occurence(s) transformed");
  QMessageBox::information(this, tr("Transformation successfull"), changedPrefabInfo.arg(changedNodeCount, 0, 10));
}

void MainWindow::on_toolsSubtypeComboBox_currentIndexChanged(int index)
{
  switch(index) {
  case 0: // Add
  case 1: // Replace
    ui->transformByComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 2: // Increment
    ui->transformByComboBox->hide();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  default:
    return;
  }
}

void MainWindow::on_toolsTypeComboBox_currentIndexChanged(int index)
{
  switch(index) {
  case 0: // Replace
    ui->toolsValuesStackedWidget->setCurrentIndex(0);
    ui->toolsSubtypeLabel->hide();
    ui->toolsSubtypeComboBox->hide();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 1: // Move
    ui->toolsValuesStackedWidget->setCurrentIndex(1);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 2: // Rotate
    ui->toolsValuesStackedWidget->setCurrentIndex(2);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 3: // Scale
    ui->toolsValuesStackedWidget->setCurrentIndex(1);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 4: // Mirror
    ui->toolsValuesStackedWidget->setCurrentIndex(3);
    ui->toolsSubtypeLabel->hide();
    ui->toolsSubtypeComboBox->hide();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  default:
    return;
  }
}

void MainWindow::on_savePushButton_released()
{
  saveTrackToDb();
}

void MainWindow::onNodeEditorContextMenu(const QPoint &point)
{
  // Check if we can get a index on the click point
  QModelIndex index = ui->nodeTreeView->indexAt(point);
  if (!index.isValid())
    return;

  // Map the point to global space and open the context menu at that point
  nodeEditorContextMenu.exec(ui->nodeTreeView->viewport()->mapToGlobal(point));
}

void MainWindow::onNodeEditorContextMenuAddToFilterAction()
{
  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = ui->nodeTreeView->selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    QModelIndex index = selectedIndexes.takeLast();
    if (index.column() == NodeTreeColumns::TypeColumn || index.column() ==  NodeTreeColumns::ValueColumn)
      continue;

    // Get the prefab that corresponds to that index
    PrefabItem prefab(&nodeEditor);
    if (!prefab.parseIndex(nodeEditor.getFilteredModel().mapToSource(index)))
      continue;

    // Create a new filter and add it to the layout
    searchFilterLayout->addFilter(prefab.getIndex());    
  }

  updateSearchFilter();
  updateStatusBar();
}

void MainWindow::onNodeEditorContextMenuDeleteAction()
{
  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = ui->nodeTreeView->selectionModel()->selectedIndexes();
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    QModelIndex index = selectedIndexes.takeLast();
    if (index.column() == NodeTreeColumns::TypeColumn || index.column() ==  NodeTreeColumns::ValueColumn)
      continue;

    // Get the prefab that corresponds to that index
    PrefabItem prefab(&nodeEditor);
    if (!prefab.parseIndex(nodeEditor.getFilteredModel().mapToSource(index)))
      continue;

    // Delete the root node of the prefab
    nodeEditor.deleteNode(prefab.getIndex());
  }

  updateSearchFilter();
  updateStatusBar();
}

void MainWindow::onNodeEditorContextMenuDublicateAction()
{
  // Go through each user selection, determinate the corresponding prefab and dublicate it
  foreach(QModelIndex index, ui->nodeTreeView->selectionModel()->selectedIndexes()) {
    if (index.column() == NodeTreeColumns::TypeColumn || index.column() ==  NodeTreeColumns::ValueColumn)
      continue;

    // The index refeers to the filtered model, so we need to map it to the source and
    // create a new prefab from it
    PrefabItem prefab(&nodeEditor);
    if (!prefab.parseIndex(nodeEditor.getFilteredModel().mapToSource(index)))
      continue;

    nodeEditor.dublicatePrefab(&prefab);
  }

  // Update the status bar and filter markings, cause they could have changed
  updateSearchFilter();
  updateStatusBar();

  // In case we duplicated a gate, the gate count has increased,
  // which means we also have to re-evaluate the add-gate-filter spinbox
  // in case its type is currently selected. Otherwise you couldn't select the
  // new gate as a filter.
  on_searchTypeComboBox_currentIndexChanged(ui->searchTypeComboBox->currentText());
}

void MainWindow::onNodeEditorContextMenuMassDublicateAction()
{
  // Get the amount of dublicates we want to create
  bool ok;
  int amount = QInputDialog::getInt(this, tr("Mass dublicate"), tr("Dublicates"), 1, 1, 1000, 1, &ok);
  if (!ok)
    return;

  // Go through each user selection, determinate the corresponding prefab and dublicate it
  foreach(QModelIndex index, ui->nodeTreeView->selectionModel()->selectedIndexes()) {
    if (index.column() == NodeTreeColumns::TypeColumn || index.column() ==  NodeTreeColumns::ValueColumn)
      continue;

    // The index refeers to the filtered model, so we need to map it to the source and
    // create a new prefab from it
    PrefabItem prefab(&nodeEditor);
    if (!prefab.parseIndex(nodeEditor.getFilteredModel().mapToSource(index)))
      continue;

    for (int i = 0; i < amount; ++i) {
     nodeEditor.dublicatePrefab(&prefab);
    }
  }

  // Update the status bar and filter markings, cause they could have changed
  updateSearchFilter();
  updateStatusBar();

  // In case we duplicated a gate, the gate count has increased,
  // which means we also have to re-evaluate the add-gate-filter spinbox
  // in case its type is currently selected. Otherwise you couldn't select the
  // new gate as a filter.
  on_searchTypeComboBox_currentIndexChanged(ui->searchTypeComboBox->currentText());
}






