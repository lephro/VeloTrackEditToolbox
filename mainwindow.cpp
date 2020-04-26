#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  defaultWindowTitle = QString(windowTitle());

  nodeCountLabel = new QLabel(this);
  prefabCountLabel = new QLabel(this);
  gateCountLabel = new QLabel(this);
  splineCountLabel = new QLabel(this);
  updateStatusBar();
  statusBar()->addPermanentWidget(nodeCountLabel);
  statusBar()->addPermanentWidget(prefabCountLabel);
  statusBar()->addPermanentWidget(gateCountLabel);
  statusBar()->addPermanentWidget(splineCountLabel);

  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
  ui->aboutPushButton->setVisible(false);

  QRegularExpression regEx("^[A-Za-z][\\w\\s\\-]+");
  QRegularExpressionValidator* trackNameValidator = new QRegularExpressionValidator(regEx, this);
  ui->mergeTrackNewTrackNameLineEdit->setValidator(trackNameValidator);

  productionDb = new VeloDb(DatabaseType::Production);
  betaDb = new VeloDb(DatabaseType::Beta);
  customDb = new VeloDb(DatabaseType::Custom);
  veloTrack = new VeloTrack();   
  archive = new TrackArchive(this);

  try {
    readSettings();
  } catch (VeloToolkitException& e) {
    e.Message();
  }    

  ui->treeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, veloTrack));

  // Load settings into controls
  updateDatabaseOptionsDatabaseStatus();
  ui->archiveMoveToArchiveCheckBox->setChecked(settingMoveToArchive);
  ui->saveAsNewCheckbox->setChecked(settingSaveAsNew);

  if (productionDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(0, tr("Production"));

  if (betaDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(1, tr("Beta"));

  if (customDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(2, tr("Custom"));

  ui->trackArchiveSettingsFilepathLineEdit->setText(archiveDbFileName);
  ui->archiveDatabaseSelectionComboBox->setVisible(ui->archiveDatabaseSelectionComboBox->count() > 1);

  if (ui->archiveDatabaseSelectionComboBox->count() == 0)
    QMessageBox::information(this, "No databases found!", "The VeloTrackToolkit could not find any databases.\nPlease go to the options page and select the nescessary database files.");

  ui->navListWidget->setCurrentRow(NavRows::NodeEditorRow);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  if(maybeSave()) {
    writeSettings();
    e->accept();
  } else
    e->ignore();
}

void MainWindow::on_buildTypeComboBox_currentIndexChanged(int index)
{
  setDatabaseOptionsDatabaseFilenames(DatabaseType(index));
  updateDatabaseOptionsDatabaseStatus();
}

void MainWindow::on_browseUserDbToolButton_released()
{
  QString result = browseDatabaseFile();
  if (result != "") {
    setDatabaseOptionsUserDb(result);
    ui->userDbLineEdit->setText(result);
  }
  updateDatabaseOptionsDatabaseStatus();
}

void MainWindow::on_browseSettingsDbToolButton_released()
{
  QString result = browseDatabaseFile();
  if (result != "") {
    setDatabaseOptionsSettingsDb(result);
    ui->settingsDbLineEdit->setText(result);
  }
  updateDatabaseOptionsDatabaseStatus();
}

void MainWindow::on_deleteTrackPushButton_released()
{
  if (loadedTrack.id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  QString message = tr("Do you really want to delete the track \"%1\"?\nMAKE SURE TO ENABLE YOUR BRAIN NOW!");
  QMessageBox::StandardButtons ret = QMessageBox::question(this, tr("Are you sure?"), message.arg(loadedTrack.name));
  if (ret != QMessageBox::Yes)
    return;

  try {
    getDatabase()->deleteTrack(loadedTrack);
  } catch (VeloToolkitException& e) {
    e.Message();
    closeTrack();
    return;
  }

  closeTrack();

  statusBar()->showMessage(tr("Track deleted."), 2000);
}

void MainWindow::on_openTrackPushButton_released()
{
  openTrack();
}

void MainWindow::on_replacePrefabComboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  // Get the selected Prefab
  PrefabData selectedPrefab = veloTrack->getPrefab(ui->replacePrefabComboBox->currentData().toUInt());
  // Clear the replacePrefabWithComboBox
  ui->replacePrefabWithComboBox->clear();
  // Load prefabs into replacePrefabWithComboBox
  for (QVector<PrefabData>::iterator i = veloTrack->getPrefabs()->begin(); i != veloTrack->getPrefabs()->end(); ++i) {
    // Only add if its the same type
    if (i->gate == selectedPrefab.gate) {
      ui->replacePrefabWithComboBox->addItem(i->name, i->id);      
      // Do a preselection of the currently selected item in the replacePrefabWithComboBox
      if (i->id == selectedPrefab.id) {
        ui->replacePrefabWithComboBox->setCurrentIndex(ui->replacePrefabWithComboBox->count() - 1);
      }
    }
  }
}

void MainWindow::on_replacePushButton_released()
{
  if (loadedTrack.id > 0)
    replacePrefab();
}

void MainWindow::on_saveAsNewCheckbox_stateChanged(int arg1)
{
  if (arg1 && !maybeDontBecauseItsBeta())
    return;

  settingSaveAsNew = bool(arg1);

  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->setValue("database/saveTrackAsNew", settingSaveAsNew);
}

void MainWindow::on_savePushButton_released()
{
  saveTrackToDb();
}

void MainWindow::on_settingsDbLineEdit_textChanged(const QString &arg1)
{
  setDatabaseOptionsSettingsDb(arg1);
}

void MainWindow::on_userDbLineEdit_textChanged(const QString &arg1)
{
  setDatabaseOptionsUserDb(arg1);
}

void MainWindow::on_mergeTrack1SelectPushButton_released()
{
  try {
    // Execute the open track dialog
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      // Get the selection
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      // Check if a track was selected
      if (selectedTrack.id > 0) {
        // Put the selected track into the merge selection
        mergeTrack1 = selectedTrack;
        ui->mergeTrack1Name->setText(selectedTrack.name);
        // Set the track name of the new track
        if (ui->mergeTrackNewTrackNameLineEdit->text() == "")
          ui->mergeTrackNewTrackNameLineEdit->setText(selectedTrack.name + "-merged");
      }
    }
  } catch (VeloToolkitException& e) {
    e.Message();
  }
}

void MainWindow::on_mergeTrack2SelectPushButton_released()
{
  try {
    // Execute the open track dialog
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      // Get the selection
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      // Check if a track was selected
      if (selectedTrack.id > 0) {
        // Put the selected track into the merge selection
        mergeTrack2 = selectedTrack;
        ui->mergeTrack2Name->setText(selectedTrack.name);
      }
    }
  } catch (VeloToolkitException& e) {
    e.Message();
  }
}

void MainWindow::on_mergeTrackPushButton_released()
{
  // Check if two tracks are selected
  if ((mergeTrack1.id == 0) || (mergeTrack2.id == 0)) {
    QMessageBox::critical(this, tr("Merge error"), tr("Not enough tracks selected\nPlease select two tracks, that you want to merged."));
    return;
  }

  // Check if both are different :)
  if ((mergeTrack1.assignedDatabase == mergeTrack2.assignedDatabase) && (mergeTrack1.id == mergeTrack2.id)) {
    QMessageBox::critical(this, tr("Merge error"), tr("You are trying to merge a track into itself. Thats a nope!"));
    return;
  }

  // Create a new velo track manager
  VeloTrack newVeloTrack;
  try {
    // Set the prefabs
    newVeloTrack.setPrefabs(getDatabase(mergeTrack1.assignedDatabase)->getPrefabs());

    // Merge the json data
    newVeloTrack.mergeJsonData(&mergeTrack1.value, ui->mergeTrack1BarriersCheckBox->isChecked(), ui->mergeTrack1GatesCheckBox->isChecked());
    newVeloTrack.mergeJsonData(&mergeTrack2.value, ui->mergeTrack2BarriersCheckBox->isChecked(), ui->mergeTrack2GatesCheckBox->isChecked());
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  // Create a new track
  TrackData newTrack;
  newTrack.id = 0;
  newTrack.name = ui->mergeTrackNewTrackNameLineEdit->text();
  newTrack.sceneId = mergeTrack1.sceneId;
  newTrack.assignedDatabase = mergeTrack1.assignedDatabase;
  newTrack.protectedTrack = 0;
  newTrack.value = *newVeloTrack.exportAsJsonData();

  // Save the track into the database
  try {
    getDatabase(newTrack.assignedDatabase)->saveTrack(newTrack);
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  // Reset the selection
  mergeTrack1 = TrackData();
  mergeTrack2 = TrackData();
  ui->mergeTrack1Name->setText(tr("None"));
  ui->mergeTrack2Name->setText(tr("None"));
  ui->mergeTrackNewTrackNameLineEdit->setText("");

  // Message the user
  QMessageBox::information(this, "Merge succeeded!", "The tracks got successfully merged.\nThe new track \"" + newTrack.name + "\" has been written to the database.");

  // Show a status bar message
  statusBar()->showMessage(tr("Track deleted."), 2000);
}

void MainWindow::on_aboutPushButton_released()
{
  ui->aboutGroupBox->setTitle(tr("About"));
  ui->aboutLicensePushButton->setVisible(true);
  ui->aboutPatchLogPushButton->setVisible(true);
  ui->aboutPushButton->setVisible(false);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
}

void MainWindow::on_aboutPatchLogPushButton_released()
{
  ui->aboutGroupBox->setTitle(tr("Patch Log"));
  ui->aboutLicensePushButton->setVisible(true);
  ui->aboutPatchLogPushButton->setVisible(false);
  ui->aboutPushButton->setVisible(true);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::PatchLogPage);
}

void MainWindow::on_aboutLicensePushButton_released()
{
  ui->aboutGroupBox->setTitle(tr("License"));
  ui->aboutLicensePushButton->setVisible(false);
  ui->aboutPatchLogPushButton->setVisible(true);
  ui->aboutPushButton->setVisible(true);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::LicensePage);
}

void MainWindow::on_viewNodeTypeColumn_stateChanged(int arg1)
{
  settingViewTypeColumn = bool(arg1);
  if (settingViewTypeColumn) {
    ui->treeView->showColumn(NodeTreeColumns::TypeColumn);
  } else {
    ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);
  }
}

void MainWindow::on_archiveDatabaseSelectionComboBox_currentIndexChanged(const QString &arg1)
{
  ui->archiveTrackSelectionTreeWidget->clear();

  VeloDb* selectedDb = nullptr;
  if (arg1 == tr("Production")) {
    selectedDb = productionDb;
  } else if (arg1 == tr("Beta")) {
    selectedDb = betaDb;
  } else if (arg1 == tr("Custom")) {
    selectedDb = customDb;
  }

  loadDatabaseForArchive(selectedDb);
}

void MainWindow::loadArchive()
{
  // Get an available database to pull scenes from
  VeloDb* database = getDatabase();    

  // Clear the archive tree view
  ui->archiveTreeWidget->clear();

  // Insert the tracks into the archive tree view
  int row = 0;
  foreach(TrackData track, archive->getTracks()) {
    // Create a new tree view item
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();

    // Set the track name as item text
    trackItem->setText(0, track.name);

    // Get the scene of the track if we got a database
    if (database != nullptr) {
      for (QVector<SceneData>::iterator i = database->getScenes()->begin(); i != database->getScenes()->end(); ++i) {
        if (i->id == track.sceneId) {
          trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
          break;
        }
      }
    } else {
      // if we dont have a database use the scene id as fallback
      trackItem->setText(TrackTreeColumns::SceneColumn, QString("%1").arg(track.sceneId, 0, 10));
    }

    // Insert the track data into the tree view item as user data
    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    // Add the tree view item to the tree view
    ui->archiveTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  // Create and set the labels for the tree view
  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTreeWidget->setHeaderLabels(labels);

  // Resize columns to their content
  ui->archiveTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}

void MainWindow::loadDatabaseForArchive(VeloDb* database)
{
  // Clear the archive track selection tree view
  ui->archiveTrackSelectionTreeWidget->clear();

  if (database == nullptr)
    return;

  // Query all data from the database
  database->queryAll();

  // Insert tracks into the archive track selection tree view
  int row = 0;
  foreach(TrackData track, *database->getTracks()) {
    // Create a new tree view item
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();

    // Set the track name as item text
    trackItem->setText(0, track.name);

    // Get the scene of the track
    for (QVector<SceneData>::iterator i = database->getScenes()->begin(); i != database->getScenes()->end(); ++i) {
      if (i->id == track.sceneId) {
        // Set the scene title in the scene column
        trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
        break;
      }
    }

    // Insert the track data into the tree view item as user data
    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    // Add the tree view item to the tree view
    ui->archiveTrackSelectionTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  // Create and set the labels for the tree view
  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTrackSelectionTreeWidget->setHeaderLabels(labels);

  // Resize columns to their content
  ui->archiveTrackSelectionTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}

bool MainWindow::maybeDontBecauseItsBeta()
{
  const QMessageBox::StandardButton ret
      = QMessageBox::warning(this,
                             defaultWindowTitle,
                             tr("This is still a beta build.\nI know this option may sound tempting,\nbut since this is still a beta and things might not always go as planned.\nDo at least a test with an unimportant track,\nbefore you throw your secret unbackuped gems on it."),
                             QMessageBox::Ok | QMessageBox::Abort);

  QString result = "";

  switch (ret) {
  case QMessageBox::Ok:
    return true;
  case QMessageBox::Abort:
    return false;
  default:
    break;
  }
  return true;
}

void MainWindow::on_archiveAddTrackPushButton_released()
{
  // Get selected tracks
  QList<QTreeWidgetItem*> selection = ui->archiveTrackSelectionTreeWidget->selectedItems();

  // Check if a selection was made
  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to transfer into your archive."));
    return;
  }

  // If we move the track we have to delete it from the origin database
  // Gets the origin database
  QString selectedDbStr = ui->archiveDatabaseSelectionComboBox->currentText();
  VeloDb* selectedDbInArchive = nullptr;
  if (settingMoveToArchive) {
    if (selectedDbStr == tr("Production")) {
      selectedDbInArchive = productionDb;
    } else if (selectedDbStr == tr("Beta")) {
      selectedDbInArchive = betaDb;
    } else if (selectedDbStr == tr("Custom")) {
      selectedDbInArchive = customDb;
    } else {
      // ToDo: Errorhandling
      return;
    }
  }

  // Archive all selected tracks
  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();
    // If we move the track we have to delete it from the origin database
    // Deletes the track from the origin database
    if (settingMoveToArchive) {
      try {
        selectedDbInArchive->deleteTrack(track);
      } catch (VeloToolkitException& e) {
        e.Message();
        return;
      }
    }

    try {
      archive->archiveTrack(track);
    } catch (VeloToolkitException& e) {
      e.Message();
      return;
    }
  }

  // Reload the archive and selected database
  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
  on_archiveDatabaseSelectionComboBox_currentIndexChanged(ui->archiveDatabaseSelectionComboBox->currentText());

  // Show a status bar message
  statusBar()->showMessage(tr("Track archived."), 2000);
}

void MainWindow::on_archiveRestoreTrackPushButton_released()
{
  // Get selected tracks
  QList<QTreeWidgetItem*> selection = ui->archiveTreeWidget->selectedItems();

  // Check if a selection was made
  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to restore from your archive."));
    return;
  }

  // Restore each selected track
  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    // Get the trackdata
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();

    // Get the selected database in which the track has to be restored
    QString selectedDbStr = ui->archiveDatabaseSelectionComboBox->currentText();
    VeloDb* selectedDbInArchive = nullptr;
    if (selectedDbStr == tr("Production")) {
      selectedDbInArchive = productionDb;
    } else if (selectedDbStr == tr("Beta")) {
      selectedDbInArchive = betaDb;
    } else if (selectedDbStr == tr("Custom")) {
      selectedDbInArchive = customDb;
    } else {
      // ToDo: Errorhandling
      return;
    }

    // Restore the track
    try {
      TrackData restoredTrack = archive->restoreTrack(track);
      restoredTrack.assignedDatabase = selectedDbInArchive->getDatabaseType();
      selectedDbInArchive->saveTrack(restoredTrack);
      //selectedDbInArchive->queryAll();
      loadDatabaseForArchive(selectedDbInArchive);
    } catch (VeloToolkitException& e) {
      e.Message();
      return;
    }
  }

  // Reload the archive and selected database
  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
  on_archiveDatabaseSelectionComboBox_currentIndexChanged(ui->archiveDatabaseSelectionComboBox->currentText());

  // Show a status bar message
  statusBar()->showMessage(tr("Track restored."), 2000);
}

void MainWindow::on_trackArchiveSettingsBrowseToolButton_released()
{
  QString result = QFileDialog::getSaveFileName(this,
                                                tr("Choose or create an archive"),
                                                QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(),
                                                tr("Database Files (*.db)"),
                                                nullptr,
                                                QFileDialog::Option::DontConfirmOverwrite);
  if (result == "")
    return;

  ui->trackArchiveSettingsFilepathLineEdit->setText(result);
}

void MainWindow::on_trackArchiveSettingsFilepathLineEdit_textChanged(const QString &arg1)
{
  try {
    archiveDbFileName = arg1;
    archive->setFileName(arg1);
    loadArchive();
    writeSettings();
  } catch (VeloToolkitException& e) {
    e.Message();
  }
}

void MainWindow::on_geoGenTestPushButton_released()
{
  for (int i = 0; i < getDatabase()->getTracks()->count(); ++i) {
    if (getDatabase()->getTracks()->at(i).name == "Geodome Test") {
      TrackData oldTrack = getDatabase()->getTracks()->at(i);
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

void MainWindow::on_navListWidget_currentRowChanged(int currentRow)
{
  // Switch the selected page
  switch (currentRow) {
  // About Page
  case NavRows::AboutRow:
    // Reset the buttons and view
    ui->aboutLicensePushButton->setVisible(true);
    ui->aboutPatchLogPushButton->setVisible(true);
    ui->aboutPushButton->setVisible(false);
    ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
    break;
  // Archive page
  case NavRows::ArchiveRow:
    // Check if an archive is set and ask for it
    maybeCreateOrSelectArchive();
    break;
  }
}

void MainWindow::on_replaceScalingResetPushButton_released()
{
  // Set the X, Y, Z values to 1
  ui->replaceScalingXDoubleSpinBox->setValue(1.0);
  ui->replaceScalingYDoubleSpinBox->setValue(1.0);
  ui->replaceScalingZDoubleSpinBox->setValue(1.0);
}

void MainWindow::on_archiveMoveToArchiveCheckBox_stateChanged(int arg1)
{
  // Set the setting
  settingMoveToArchive = bool(arg1);

  // Write into config
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->setValue("archive/moveToArchive", settingSaveAsNew);
}
