#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::addFilter(const QModelIndex& index)
{
  currentCacheId++;
  searchFilterLayout->addFilter(index);
}

void MainWindow::addFilter(const FilterTypes filterType, const FilterMethods filterMethod, int value, const QString displayValue, const QModelIndex& customIndex)
{
  currentCacheId++;
  searchFilterLayout->addFilter(filterType, filterMethod, value, displayValue, customIndex);
}

void MainWindow::closeTrack()
{
  closeTrack(ui->nodeEditorTabWidget->currentIndex());
}

void MainWindow::closeTrack(const int index)
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor(index);
  if (nodeEditor == nullptr)
    return;

  // Set the default window title
//  setWindowTitle(defaultWindowTitle);

  // Clear all track related controls
  ui->searchValueComboBox->clear();
//  // ToDo: ui->sceneComboBox->clear();
  ui->replacePrefabComboBox->clear();
  ui->replacePrefabWithComboBox->clear();

  // Close the editor and remove the tab
  nodeEditorManager->closeEditor(index);
  ui->nodeEditorTabWidget->removeTab(index);

  // Reset the node counter and upadte the status bar
  updateStatusBar();
}

void MainWindow::loadTrack(const TrackData& track)
{
  // Get the assigned database of the track
  VeloDb* veloDb = getDatabase(track.assignedDatabase);
  if (veloDb == nullptr)
    return;

  nodeEditorManager->addEditor(veloDb->getPrefabs(), track);

  // Load the scenes into the combo box
  bool loaded = false;
  foreach(SceneData scene, veloDb->getScenes()) {
    loaded = false;
    for (int i = 0; i < ui->sceneComboBox->count(); ++i) {
      if (ui->sceneComboBox->itemData(i) != scene.id)
        continue;

      loaded = true;
      break;
    }

    if (loaded)
      continue;

    ui->sceneComboBox->addItem(scene.title, scene.id);
    if (scene.id == track.sceneId)
      ui->sceneComboBox->setCurrentText(scene.title);
  }

  // Update the prefab controls so only the prefabs of the track are shown
  updatePrefabComboBoxes();  

  // Update filter if any set
  updateSearch();

  // Update the status bar
  updateStatusBar();

  statusBar()->showMessage(tr("Track loaded successfully."), 2000);
}

void MainWindow::toolsReplaceObject() {
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // Do we got a valid track loaded?
  if (nodeEditor->getTrackData().id == 0)
    return;

  uint changedPrefabCount = 0;   

  // Get the starting index for the search according to the user selection
  // Call the velo track replace function with the search index or prefabs and scaling
  nodeEditor->beginNodeEdit();
  if (ui->toolsTargetComboBox->currentIndex() < 2) {
    changedPrefabCount = nodeEditor->replacePrefabs(ui->toolsTargetComboBox->currentIndex() == 0 ?
                                                     nodeEditor->getRootIndex() :
                                                     nodeEditor->getTreeView().currentIndex(),
                                                   ui->replacePrefabComboBox->currentData().toUInt(),
                                                   ui->replacePrefabWithComboBox->currentData().toUInt());
  } else {
    changedPrefabCount = nodeEditor->replacePrefabs(nodeEditor->getSearchResult(),
                                                   ui->replacePrefabComboBox->currentData().toUInt(),
                                                   ui->replacePrefabWithComboBox->currentData().toUInt());
  }
  nodeEditor->endNodeEdit();

  // Update the prefab combo boxes, if we added or removed a new prefab
  if (changedPrefabCount > 0)
    updatePrefabComboBoxes();

  QString changedPrefabInfo = tr("%1 occurence(s) replaced");
  QMessageBox::information(this, tr("Replace successfull"), changedPrefabInfo.arg(changedPrefabCount, 0, 10));
}

void MainWindow::saveTrackToDb()
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr || nodeEditor->getTrackData().id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  TrackData& track = nodeEditor->getTrackData();

  // Write the selected scene and any node changes to the currently loaded track
  track.sceneId = ui->sceneComboBox->currentData().toUInt();
  track.value = *nodeEditor->exportAsJsonData();

  // Message for later output
  QString message = tr("The track was saved successfully to the database!");

  // Modify the message in case we are saving as a new track
  const bool saveAsNew = ui->saveAsNewCheckbox->checkState() == Qt::Checked;
  if (saveAsNew) {
    track.name += "-new";
    message += tr("\n\nNew track name: ") + track.name;

    // Since we got a new track name we need to update the window title
    updateWindowTitle();
  }

  try {
    // Write the track into the database and retrieves its id
    track.id = getDatabase()->saveTrack(track, saveAsNew);

    // Reset the modified flat
    nodeEditor->beginNodeEdit();
    nodeEditor->clearModifiedFlag();
    nodeEditor->endNodeEdit();

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
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  if (nodeEditor->getTrackData().id == 0)
    return;

  // Create a new file (remove any already existing) and export the velo track json into it
  QFile *file = new QFile("track.json");
  file->remove();
  file->open(QFile::ReadWrite);
  file->write(*nodeEditor->exportAsJsonData());
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
  const NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // Clear the combobox and insert all prefabs from the velo track
  ui->replacePrefabComboBox->clear();
  ui->searchValueComboBox->clear();
  const QVector<PrefabData> prefabsInUse = nodeEditor->getPrefabsInUse(true);
  foreach(PrefabData prefab, prefabsInUse)
  {
    ui->replacePrefabComboBox->addItem(prefab.name, prefab.id);
    ui->searchValueComboBox->addItem(prefab.name, prefab.id);
  }

  if (nodeEditor->getTrack()->getAvailablePrefabCount() != ui->replacePrefabWithComboBox->count()) {
    ui->replacePrefabWithComboBox->clear();

    foreach(PrefabData prefab, nodeEditor->getAllPrefabData()) {
      ui->replacePrefabWithComboBox->addItem(prefab.name, prefab.id);
    }
  }
}

void MainWindow::on_deleteTrackPushButton_released()
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // Do we got any track loaded?
  if (nodeEditor->getTrackData().id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  // Create a warning and double check
  QString message = tr("Do you really want to delete the track \"%1\"?\nMAKE SURE TO ENABLE YOUR BRAIN NOW!");
  QMessageBox::StandardButtons ret = QMessageBox::question(this, tr("Are you sure?"), message.arg(nodeEditor->getTrackData().name));
  if (ret != QMessageBox::Yes)
    return;

  // Delete the track
  try {
    getDatabase()->deleteTrack(nodeEditor->getTrackData());
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
//  // Code left over from a (so far) failed try to create a geodesic dome with neon stripes
//  for (int i = 0; i < getDatabase()->getTracks().count(); ++i) {
//    if (getDatabase()->getTracks().at(i).name == "Geodome Test") {
//      TrackData oldTrack = getDatabase()->getTracks().at(i);
//      getDatabase()->deleteTrack(oldTrack);
//    }
//  }
//  GeodesicDome dome(this, 2);
//  TrackData track;
//  track.sceneId = 16;
//  track.name = "Geodome Test";
//  track.value = dome.getVeloTrackDataTest();
//  track.protectedTrack = 0;
//  track.assignedDatabase = getDatabase()->getDatabaseType();
//  track.id = getDatabase()->saveTrack(track);
//  loadTrack(track);
}

void MainWindow::on_openTrackPushButton_released()
{
  try {
    // Show the open track dialog and get the user selection,
    // check for unwritten changes and eventually load the new track
    if (openTrackDialog->exec()) {
      if (maybeSave()) {
        TrackData selectedTrack = openTrackDialog->getSelectedTrack();
        if (selectedTrack.id > 0) {
          loadTrack(selectedTrack);
        }
      }
    }
  } catch (VeloToolkitException& e) {
    // Something weird happend. Close the track and give the user some info
    e.Message();
    nodeEditorManager->closeEditor(ui->nodeEditorTabWidget->currentIndex());
  }
}

void MainWindow::on_nodeEditorTabWidget_currentChanged(int index)
{
  Q_UNUSED(index)
  updatePrefabComboBoxes();
  updateSearch();
  updateStatusBar();
  updateWindowTitle();
}

void MainWindow::on_nodeEditorTabWidget_tabCloseRequested(int index)
{
  closeTrack(index);
}

void MainWindow::on_refreshTrackPushButton_released()
{  
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  if (nodeEditor->getTrackData().id == 0)
    return;

  VeloDb* veloDb = getDatabase();
  if (veloDb == nullptr)
    return;

  try {
    veloDb->queryTracks();
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  QVector<TrackData> tracks = veloDb->getTracks();

  foreach(TrackData track, tracks) {
    if (track.id != nodeEditor->getTrackData().id)
      continue;

    //nodeEditor->beginNodeEdit();
    try {
      // ToDo
      //loadTrack(track);
    } catch (VeloToolkitException& e) {
      closeTrack();
      e.Message();
      return;
    }
    //nodeEditor->endNodeEdit();
    return;
  }
}


void MainWindow::on_replacePrefabComboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // Get the selected Prefab
  PrefabData selectedPrefab = nodeEditor->getPrefabData(ui->replacePrefabComboBox->currentData().toUInt());

  // Clear the replacePrefabWithComboBox
  ui->replacePrefabWithComboBox->clear();

  // Load all prefabs into replacePrefabWithComboBox...
  foreach(PrefabData prefab, nodeEditor->getAllPrefabData()) {
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
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  const bool byPercent = ui->transformByComboBox->currentIndex() == 0;

  int toolTypeIndex = ui->toolsTypeComboBox->currentIndex();
  if (toolTypeIndex == 1) // Move
    toolTypeIndex = ToolTypes::Move + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 2) // Rotate
    toolTypeIndex = ToolTypes::AddRotation + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 3) // Scale
    toolTypeIndex = ToolTypes::Scale + ui->toolsSubtypeComboBox->currentIndex();
  else if (toolTypeIndex == 4) // Mirror
    toolTypeIndex = ToolTypes::Mirror;

  //qDebug() << "toolsTypeComboBox->currentIndex:" << ui->toolsTypeComboBox->currentIndex();
  //qDebug() << "toolsSubtypeComboBox->currentIndex:" << ui->toolsSubtypeComboBox->currentIndex();
  //qDebug() << "transformByComboBox->currentIndex:" << ui->transformByComboBox->currentIndex();
  //qDebug() << "Transform Method Index: " << toolTypeIndex;

  QVariant value;
  switch (toolTypeIndex) {
  case ToolTypes::Replace:
    // We got our own routine to replace objects
    toolsReplaceObject();
    return;
  case ToolTypes::Move:
  case ToolTypes::IncreasingPosition:
  case ToolTypes::ReplacePosition:
  case ToolTypes::Scale:
  case ToolTypes::IncreasingScale:
  case ToolTypes::ReplaceScaling:
  case ToolTypes::MultiplyPosition:
  case ToolTypes::MultiplyScaling:
    value = QVector3D(float(ui->transformRDoubleSpinBox->value()),
                      float(ui->transformGDoubleSpinBox->value()),
                      float(ui->transformBDoubleSpinBox->value()));
    break;
  case ToolTypes::AddRotation:
  case ToolTypes::IncreasingRotation:
    value = QQuaternion::fromEulerAngles(ui->transformRotationRValueSpinBox->value(),
                                         ui->transformRotationGValueSpinBox->value(),
                                         ui->transformRotationBValueSpinBox->value());
    //qDebug() << "Rotation from euler :" << ui->transformRotationRValueSpinBox->value() <<
    //    ui->transformRotationGValueSpinBox->value() <<
    //    ui->transformRotationBValueSpinBox->value() << "Quat:" << value;
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
    nodeEditor->beginNodeEdit();
    changedNodeCount = nodeEditor->transformPrefab(nodeEditor->getRootIndex(),
                                                  ToolTypes(toolTypeIndex),
                                                  value,
                                                  ToolTypeTargets(ui->toolsSubtypeTargetComboBox->currentIndex()),
                                                  byPercent);
    nodeEditor->endNodeEdit();
    break;

  case 1: // Selected Nodes
    nodeEditor->beginNodeEdit();
    changedNodeCount = nodeEditor->transformPrefab(
          nodeEditor->getTreeView().selectionModel()->selectedIndexes(),
          ToolTypes(toolTypeIndex),
          value,
          ToolTypeTargets(ui->toolsSubtypeTargetComboBox->currentIndex()),
          byPercent);
    nodeEditor->endNodeEdit();
    break;

  case 2: // Filtered Nodes
    nodeEditor->beginNodeEdit();
    changedNodeCount = nodeEditor->transformPrefab(
          nodeEditor->getSearchResult(),
          ToolTypes(toolTypeIndex),
          value,
          ToolTypeTargets(ui->toolsSubtypeTargetComboBox->currentIndex()),
          byPercent);
    nodeEditor->endNodeEdit();
    break;

  default:
    return;
  }

  const QString changedPrefabInfo = tr("%1 occurence(s) transformed");
  QMessageBox::information(this, tr("Transformation successfull"), changedPrefabInfo.arg(changedNodeCount, 0, 10));
}

void MainWindow::on_toolsSubtypeComboBox_currentIndexChanged(int index)
{
  switch(index) {
  case 0: // Add
  case 1: // Replace
  case 3: // Multiply
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

void MainWindow::on_toolsSubtypeTargetComboBox_currentIndexChanged(int index)
{
  switch(index) {
  case 1: // R
    ui->transformRDoubleResetPushButton->show();
    ui->transformRDoubleSpinBox->show();
    ui->transformRLabel->show();

    ui->transformGDoubleResetPushButton->hide();
    ui->transformGDoubleSpinBox->hide();
    ui->transformGLabel->hide();

    ui->transformBDoubleResetPushButton->hide();
    ui->transformBDoubleSpinBox->hide();
    ui->transformBLabel->hide();
    break;

  case 2: // G
    ui->transformRDoubleResetPushButton->hide();
    ui->transformRDoubleSpinBox->hide();
    ui->transformRLabel->hide();

    ui->transformGDoubleResetPushButton->show();
    ui->transformGDoubleSpinBox->show();
    ui->transformGLabel->show();

    ui->transformBDoubleResetPushButton->hide();
    ui->transformBDoubleSpinBox->hide();
    ui->transformBLabel->hide();
    break;

  case 3: // B
    ui->transformRDoubleResetPushButton->hide();
    ui->transformRDoubleSpinBox->hide();
    ui->transformRLabel->hide();

    ui->transformGDoubleResetPushButton->hide();
    ui->transformGDoubleSpinBox->hide();
    ui->transformGLabel->hide();

    ui->transformBDoubleResetPushButton->show();
    ui->transformBDoubleSpinBox->show();
    ui->transformBLabel->show();
    break;

  default:
    ui->transformRDoubleResetPushButton->show();
    ui->transformRDoubleSpinBox->show();
    ui->transformRLabel->show();

    ui->transformGDoubleResetPushButton->show();
    ui->transformGDoubleSpinBox->show();
    ui->transformGLabel->show();

    ui->transformBDoubleResetPushButton->show();
    ui->transformBDoubleSpinBox->show();
    ui->transformBLabel->show();
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
    ui->toolsSubtypeTargetComboBox->hide();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 1: // Position
    ui->toolsValuesStackedWidget->setCurrentIndex(1);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    if (ui->toolsSubtypeComboBox->count() < 4)
      ui->toolsSubtypeComboBox->addItem(tr("Multiply"));
    ui->toolsSubtypeTargetComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 2: // Rotate
    ui->toolsValuesStackedWidget->setCurrentIndex(2);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    if (ui->toolsSubtypeComboBox->count() > 3)
      ui->toolsSubtypeComboBox->removeItem(3);
    ui->toolsSubtypeTargetComboBox->hide();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 3: // Scale
    ui->toolsValuesStackedWidget->setCurrentIndex(1);
    ui->toolsSubtypeLabel->show();
    ui->toolsSubtypeComboBox->show();
    if (ui->toolsSubtypeComboBox->count() < 4)
      ui->toolsSubtypeComboBox->addItem(tr("Multiply"));
    ui->toolsSubtypeTargetComboBox->show();
    ui->transformByComboBox->setCurrentIndex(1);
    break;
  case 4: // Mirror
    ui->toolsValuesStackedWidget->setCurrentIndex(3);
    ui->toolsSubtypeLabel->hide();
    ui->toolsSubtypeComboBox->hide();
    ui->toolsSubtypeTargetComboBox->hide();
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








