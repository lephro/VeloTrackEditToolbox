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

  PrefabData selectedPrefab = veloTrack->getPrefab(ui->replacePrefabComboBox->currentData().toUInt());
  ui->replacePrefabWithComboBox->clear();
  for (QVector<PrefabData>::iterator i = veloTrack->getPrefabs()->begin(); i != veloTrack->getPrefabs()->end(); ++i) {
    if (i->gate == selectedPrefab.gate) {
      ui->replacePrefabWithComboBox->addItem(i->name, i->id);      
      if (i->id == selectedPrefab.id) {
        ui->replacePrefabWithComboBox->setCurrentIndex(ui->replacePrefabWithComboBox->count() - 1);
      }
    }
  }
}

void MainWindow::on_replacePushButton_released()
{
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
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      if (selectedTrack.id > 0) {
        mergeTrack1 = selectedTrack;
        ui->mergeTrack1Name->setText(selectedTrack.name);
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
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      if (selectedTrack.id > 0) {
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
  if ((mergeTrack1.id == 0) || (mergeTrack2.id == 0)) {
    QMessageBox::critical(this, tr("Merge error"), tr("Not enough tracks selected\nPlease select two tracks, that you want to merged."));
    return;
  }

  if ((mergeTrack1.assignedDatabase == mergeTrack2.assignedDatabase) && (mergeTrack1.id == mergeTrack2.id)) {
    QMessageBox::critical(this, tr("Merge error"), tr("You are trying to merge a track into itself. Thats a nope!"));
    return;
  }

  VeloTrack* newVeloTrack = new VeloTrack();
  try {
    newVeloTrack->setPrefabs(getDatabase(mergeTrack1.assignedDatabase)->getPrefabs());
    newVeloTrack->mergeJsonData(&mergeTrack1.value, ui->mergeTrack1BarriersCheckBox->isChecked(), ui->mergeTrack1GatesCheckBox->isChecked());
    newVeloTrack->mergeJsonData(&mergeTrack2.value, ui->mergeTrack2BarriersCheckBox->isChecked(), ui->mergeTrack2GatesCheckBox->isChecked());
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  TrackData newTrack;
  newTrack.id = 0;
  newTrack.name = ui->mergeTrackNewTrackNameLineEdit->text();
  newTrack.sceneId = mergeTrack1.sceneId;
  newTrack.assignedDatabase = mergeTrack1.assignedDatabase;
  newTrack.protectedTrack = 0;
  newTrack.value = *newVeloTrack->exportAsJsonData();

  try {
   getDatabase(newTrack.assignedDatabase)->saveTrack(newTrack);
  } catch (VeloToolkitException& e) {
    e.Message();

    delete newVeloTrack;

    return;
  }

  mergeTrack1 = TrackData();
  mergeTrack2 = TrackData();
  ui->mergeTrack1Name->setText(tr("None"));
  ui->mergeTrack2Name->setText(tr("None"));
  ui->mergeTrackNewTrackNameLineEdit->setText("");

  QMessageBox::information(this, "Merge succeeded!", "The tracks got successfully merged.\nThe new track \"" + newTrack.name + "\" has been written to the database.");

  delete newVeloTrack;
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
  VeloDb* database = getDatabase();

  ui->archiveTreeWidget->clear();

  int row = 0;
  foreach(TrackData track, archive->getTracks()) {
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();
    trackItem->setText(0, track.name);

    for (QVector<SceneData>::iterator i = database->getScenes()->begin(); i != database->getScenes()->end(); ++i) {
      if (i->id == track.sceneId) {
        trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
        break;
      }
    }

    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    ui->archiveTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTreeWidget->setHeaderLabels(labels);
  ui->archiveTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}

void MainWindow::loadDatabaseForArchive(VeloDb* database)
{
  ui->archiveTrackSelectionTreeWidget->clear();

  if (database == nullptr)
    return;

  database->queryAll();

  int row = 0;
  foreach(TrackData track, *database->getTracks()) {
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();
    trackItem->setText(0, track.name);

    for (QVector<SceneData>::iterator i = database->getScenes()->begin(); i != database->getScenes()->end(); ++i) {
      if (i->id == track.sceneId) {
        trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
        break;
      }
    }

    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    ui->archiveTrackSelectionTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTrackSelectionTreeWidget->setHeaderLabels(labels);
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
  QList<QTreeWidgetItem*> selection = ui->archiveTrackSelectionTreeWidget->selectedItems();

  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to transfer into your archive."));
    return;
  }

  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();
    try {
      archive->archiveTrack(track);
    } catch (VeloToolkitException& e) {
      e.Message();
    }
  }

  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
}

void MainWindow::on_archiveRestoreTrackPushButton_released()
{
  QList<QTreeWidgetItem*> selection = ui->archiveTreeWidget->selectedItems();

  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to restore from your archive."));
    return;
  }

  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();
    QString selectedDbStr = ui->archiveDatabaseSelectionComboBox->currentText();
    VeloDb* selectedDbInArchive = nullptr;
    if (selectedDbStr == tr("Production")) {
      selectedDbInArchive = productionDb;
    } else if (selectedDbStr == tr("Beta")) {
      selectedDbInArchive = betaDb;
    } else if (selectedDbStr == tr("Custom")) {
      selectedDbInArchive = customDb;
    } else {
      // ToDo: Exception
      return;
    }
    try {
      TrackData restoredTrack = archive->restoreTrack(track);
      restoredTrack.assignedDatabase = selectedDbInArchive->getDatabaseType();
      selectedDbInArchive->saveTrack(restoredTrack);
      //selectedDbInArchive->queryAll();
      loadDatabaseForArchive(selectedDbInArchive);
    } catch (VeloToolkitException& e) {
      e.Message();
    }
  }

  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
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
  GeodesicDome dome(this, 2);
  TrackData track;
  track.sceneId = 16;
  track.name = "Geodome Test";
  track.value = dome.getVeloTrackData();
  track.protectedTrack = 0;
  track.assignedDatabase = getDatabase()->getDatabaseType();
  track.id = getDatabase()->saveTrack(track);
  loadTrack(track);
}

void MainWindow::on_navListWidget_currentRowChanged(int currentRow)
{
  switch (currentRow) {
  case NavRows::AboutRow:
    ui->aboutLicensePushButton->setVisible(true);
    ui->aboutPatchLogPushButton->setVisible(true);
    ui->aboutPushButton->setVisible(false);
    ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
    break;
  case NavRows::ArchiveRow:
    maybeCreateOrCreateArchive();
    break;
  }
}


