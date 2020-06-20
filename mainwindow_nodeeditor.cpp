#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::closeTrack()
{
  // Set the default window title
  setWindowTitle(defaultWindowTitle);

  // Reset the node counter and upadte the status bar
  nodeCount = 0;
  prefabCount = 0;
  splineCount = 0;
  gateCount = 0;
  updateStatusBar();

  // Load an empty track as a placeholder
  loadedTrack = TrackData();

  // Clear all track related controls
  veloTrack.getStandardItemModel().clear();
  ui->searchValueComboBox->clear();
  ui->sceneComboBox->clear();
  ui->replacePrefabComboBox->clear();
  ui->replacePrefabWithComboBox->clear();

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
  veloTrack.setPrefabs(veloDb->getPrefabs());
  try {
    veloTrack.importJsonData(&loadedTrack.value);
  } catch (VeloToolkitException& e) {
    e.Message();
  }

  // Pass the model/items from the velo track to the tree view
  // and tell it to resize its columns to its content
  // and hide its type column if the setting is enabled
  ui->treeView->setModel(&veloTrack.getStandardItemModel());
  ui->treeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);
  if (!settingViewTypeColumn)
    ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);

  // Load the scenes into the combo box
  foreach(SceneData scene, veloDb->getScenes()) {
    ui->sceneComboBox->addItem(scene.title, scene.id);
    if (scene.id == loadedTrack.sceneId)
      ui->sceneComboBox->setCurrentText(scene.title);
  }

  // Update the prefab controls so only the prefabs of the track are shown
  updateReplacePrefabComboBox();

  // Gather the status bar information and update the status bar
  gateCount = veloTrack.getGateCount();
  prefabCount = veloTrack.getPrefabCount();
  nodeCount = veloTrack.getNodeCount();
  splineCount = veloTrack.getSplineCount();
  updateStatusBar();

  statusBar()->showMessage(tr("Track loaded successfully."), 2000);
}

void MainWindow::openTrack()
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

void MainWindow::replacePrefab()
{
  // Get the starting index for the search according to the user selection
  QModelIndex searchIndex;
  switch (ui->replacePrefabWhereComboBox->currentIndex()) {

  case 0: // All nodes
    searchIndex = veloTrack.getRootIndex();
    break;

  case 1: // Barriers
    searchIndex = veloTrack.getRootIndex();
    for (int i = 0; i < veloTrack.getStandardItemModel().rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack.getStandardItemModel().index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "barriers")
        searchIndex = childIndex;
    }
    break;

  case 2: // Gates
    searchIndex = veloTrack.getRootIndex();
    for (int i = 0; i < veloTrack.getStandardItemModel().rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack.getStandardItemModel().index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "gates")
        searchIndex = childIndex;
    }
    break;

  case 3: // Selected Node
    searchIndex = ui->treeView->currentIndex();
    break;
  }

  // Call the velo track replace function with the search index, the prefabs and scaling
  // This is where the replacing really happens
  uint changedPrefabCount = veloTrack.replacePrefab(searchIndex,
                                                     ui->replacePrefabComboBox->currentData().toUInt(),
                                                     ui->replacePrefabWithComboBox->currentData().toUInt(),
                                                     QVector3D(float(ui->replaceScalingXDoubleSpinBox->value()),
                                                               float(ui->replaceScalingYDoubleSpinBox->value()),
                                                               float(ui->replaceScalingZDoubleSpinBox->value())));

  // Update the prefab combo boxes, because we may have added new prefab or removed one completely
  updateReplacePrefabComboBox();

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
  loadedTrack.value = *veloTrack.exportAsJsonData();

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
    veloTrack.resetModified();

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
  file->write(*veloTrack.exportAsJsonData());
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

void MainWindow::updateReplacePrefabComboBox()
{
  // Clear the combobox and insert all prefabs from the velo track
  ui->replacePrefabComboBox->clear();
  QVector<PrefabData> prefabsInUse = veloTrack.getPrefabsInUse();
  for (QVector<PrefabData>::iterator i = prefabsInUse.begin(); i != prefabsInUse.end(); ++i)
  {
    ui->replacePrefabComboBox->addItem(i->name, i->id);
    ui->searchValueComboBox->addItem(i->name, i->id);
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
  openTrack();
}


void MainWindow::on_replacePrefabComboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  // Get the selected Prefab
  PrefabData selectedPrefab = veloTrack.getPrefab(ui->replacePrefabComboBox->currentData().toUInt());

  // Clear the replacePrefabWithComboBox
  ui->replacePrefabWithComboBox->clear();

  // Load all prefabs into replacePrefabWithComboBox...
  for (QVector<PrefabData>::const_iterator i = veloTrack.getPrefabs()->begin(); i != veloTrack.getPrefabs()->end(); ++i) {
    // ... but only if its the same type, so you cant replace a gate with a barrier aso, which would probably break the track
    if (i->gate == selectedPrefab.gate) {
      ui->replacePrefabWithComboBox->addItem(i->name, i->id);

      // Preselect the with combo box with this prefab if its the same as the prefab we want to replace
      if (i->id == selectedPrefab.id) {
        ui->replacePrefabWithComboBox->setCurrentIndex(ui->replacePrefabWithComboBox->count() - 1);
      }
    }
  }
}

void MainWindow::on_replacePushButton_released()
{
  // Fire the replace prefab function if we have a track loaded
  if (loadedTrack.id > 0)
    replacePrefab();
}

void MainWindow::on_replaceScalingResetPushButton_released()
{
  // Set the X, Y, Z values to 1
  ui->replaceScalingXDoubleSpinBox->setValue(1.0);
  ui->replaceScalingYDoubleSpinBox->setValue(1.0);
  ui->replaceScalingZDoubleSpinBox->setValue(1.0);
}

void MainWindow::on_savePushButton_released()
{
  saveTrackToDb();
}

void MainWindow::on_searchAddFilterPushButton_released()
{
  QString displayValue;
  int value = 0;

  // Get the display-value and value for the filter from the controls
  // depending on the filter type
  switch (FilterTypes(ui->searchTypeComboBox->currentIndex())) {
  case FilterTypes::Object:
    displayValue = ui->searchValueComboBox->currentText();
    value = int(ui->searchValueComboBox->currentData().toUInt());
    break;
  case FilterTypes::Position:
  case FilterTypes::Rotation:
  case FilterTypes::Scaling:
  case FilterTypes::GateNo:
    displayValue = QString("%1").arg(ui->searchValueSpinBox->value());
    value = ui->searchValueSpinBox->value();
    break;
  default:
    break;
  }

  // Create the filter widget and add it to the layout
  searchFilterLayout->addFilter(FilterTypes(ui->searchTypeComboBox->currentIndex()), value, displayValue);
  searchFilterLayout->update();
}

void MainWindow::on_searchClearFilterPushButton_released()
{
  searchFilterLayout->Clear();
}

void MainWindow::on_searchPushButton_released()
{
  // Get the starting index for the search according to the user selection
  QModelIndex searchIndex;
  switch (ui->replacePrefabWhereComboBox->currentIndex()) {

  case 0: // All nodes
    searchIndex = veloTrack.getRootIndex();
    break;

  case 1: // Barriers
    searchIndex = veloTrack.getRootIndex();
    for (int i = 0; i < veloTrack.getStandardItemModel().rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack.getStandardItemModel().index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "barriers")
        searchIndex = childIndex;
    }
    break;

  case 2: // Gates
    searchIndex = veloTrack.getRootIndex();
    for (int i = 0; i < veloTrack.getStandardItemModel().rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack.getStandardItemModel().index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "gates")
        searchIndex = childIndex;
    }
    break;

  case 3: // Selected Node
    searchIndex = ui->treeView->currentIndex();
    break;
  }

  veloTrack.search(searchIndex, searchFilterLayout->getFilterList());
}

void MainWindow::on_searchTypeComboBox_currentIndexChanged(int index)
{
  // Switch the visibilty of the filter-value controls according to the filter-type selected
  switch (index) {
  case FilterTypes::Object:
    ui->searchValueComboBox->show();
    ui->searchValueSpinBox->hide();
    break;
  case FilterTypes::Position:
  case FilterTypes::Rotation:
  case FilterTypes::Scaling:    
  case FilterTypes::GateNo:
    ui->searchValueSpinBox->setMinimum(index == FilterTypes::GateNo ? 1 : -99999);
    ui->searchValueComboBox->hide();
    ui->searchValueSpinBox->show();
    break;
  case FilterTypes::IsOnSpline:
  case FilterTypes::IsDublicate:
    ui->searchValueComboBox->setVisible(false);
    ui->searchValueSpinBox->setVisible(false);
    break;
  }
}

