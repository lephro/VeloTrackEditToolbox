#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  defaultWindowTitle = QString(windowTitle());

  ui->aboutLicenseTextEdit->setVisible(false);

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
}

void MainWindow::closeTrack()
{
  setWindowTitle(defaultWindowTitle);

  loadedTrack = TrackData();

  veloTrack->getStandardItemModel()->clear();
  ui->sceneComboBox->clear();
  ui->replacePrefabComboBox->clear();
  ui->replacePrefabWithComboBox->clear();
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
    if ((i->id != selectedPrefab.id) && (i->gate == selectedPrefab.gate)) {
      ui->replacePrefabWithComboBox->addItem(i->name, i->id);
    }
  }
}

void MainWindow::on_replacePushButton_released()
{
  replacePrefab();
}

void MainWindow::on_saveAsNewCheckbox_stateChanged(int arg1)
{
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

void MainWindow::openTrack()
{
  try {
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
    e.Message();
    closeTrack();
  }
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  if(maybeSave()) {
    writeSettings();
    e->accept();
  } else
    e->ignore();
}

QString MainWindow::getDefaultPath()
{
  // 4 Windoze
  return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first().replace("AppData/Local", "AppData/LocalLow/VelociDrone");
}

bool MainWindow::maybeCreateOrCreateArchive()
{

  if (archive->getFileName() != "")
      return true;
  const QMessageBox::StandardButton ret
      = QMessageBox::warning(this,
                             defaultWindowTitle,
                             tr("You haven't set up any trackarchive, yet\n Do you want to create/open an archive file now?"),
                             QMessageBox::Yes | QMessageBox::No);
  switch (ret) {
  case QMessageBox::Yes:
      saveTrackToDb();
      return true;
  case QMessageBox::No:
      return false;
  default:
      break;
  }
  return true;
}

void MainWindow::readSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  //settings->clear();

  if (settings->value("database/productionUserDbFilename", "").toString() == "") {
    settings->clear();
    writeDefaultSettings();
    settings->sync();
  }

  settings->beginGroup("general");
  settingSaveAsNew = settings->value("saveTrackAsNew", true).toBool();
  settingViewTypeColumn = settings->value("viewTypeColumn", false).toBool();
  settings->endGroup();

  settings->beginGroup("database");
  productionSettingsDbFilename = settings->value("productionSettingsDbFilename", "").toString();
  productionUserDbFilename = settings->value("productionUserDbFilename", "").toString();
  betaSettingsDbFilename = settings->value("betaSettingsDbFilename", "").toString();
  betaUserDbFilename = settings->value("betaUserDbFilename", "").toString();
  customSettingsDbFilename = settings->value("customSettingsDbFilename", "").toString();
  customUserDbFilename = settings->value("customUserDbFilename", "").toString();
  settings->endGroup();

  settings->beginGroup("archive");
  settingMoveToArchive = settings->value("moveToArchive", false).toBool();
  archiveDbFileName = settings->value("filename", "").toString();
  settings->endGroup();

  productionDb->setSettingsDbFilename(productionSettingsDbFilename);
  productionDb->setUserDbFilename(productionUserDbFilename);
  betaDb->setSettingsDbFilename(betaSettingsDbFilename);
  betaDb->setUserDbFilename(betaUserDbFilename);
  customDb->setSettingsDbFilename(customSettingsDbFilename);
  customDb->setUserDbFilename(customUserDbFilename);

  setDatabaseOptionsDatabaseFilenames(DatabaseType::Production);
}

void MainWindow::replacePrefab()
{
  QModelIndex searchIndex;
  switch (ui->replacePrefabWhereComboBox->currentIndex()) {

  case 0: // All nodes
    searchIndex = veloTrack->getRootIndex();
    break;

  case 1: // Barriers
    searchIndex = veloTrack->getRootIndex();
    for (int i = 0; i < veloTrack->getStandardItemModel()->rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack->getStandardItemModel()->index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "barriers")
        searchIndex = childIndex;
    }
    break;

  case 2: // Gates
    searchIndex = veloTrack->getRootIndex();
    for (int i = 0; i < veloTrack->getStandardItemModel()->rowCount(searchIndex); ++i) {
      QModelIndex childIndex = veloTrack->getStandardItemModel()->index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "gates")
        searchIndex = childIndex;
    }
    break;

  case 3: // Selected Node
    searchIndex = ui->treeView->currentIndex();
    break;
  }

  uint changedPrefabCount = veloTrack->replacePrefab(searchIndex,
                                                     ui->replacePrefabComboBox->currentData().toUInt(),
                                                     ui->replacePrefabWithComboBox->currentData().toUInt());

  QString changedPrefabInfo = tr("%1 occurence(s) replaced");
  QMessageBox::information(this, tr("Replace successfull"), changedPrefabInfo.arg(changedPrefabCount, 0, 10));

  updateReplacePrefabComboBox();
}

bool MainWindow::maybeSave()
{
    bool sceneChanged = (loadedTrack.sceneId != ui->sceneComboBox->currentData().toUInt());
    if (!veloTrack->isModified() && !sceneChanged)
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this,
                               defaultWindowTitle,
                               tr("Do you want to save your changes on \"") + loadedTrack.name + tr("\"?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        saveTrackToDb();
        return true;
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::saveTrackToDb()
{
  if (loadedTrack.id == 0) {
    QMessageBox::information(this, tr("No track loaded"), tr("You have to load a track first."));
    return;
  }

  loadedTrack.sceneId = ui->sceneComboBox->currentData().toUInt();
  loadedTrack.value = *veloTrack->exportAsJsonData();

  QString message = tr("The track was saved successfully to the database!");
  if (settingSaveAsNew) {
    loadedTrack.name += "-new";
    message += tr("\n\nNew track name: ") + loadedTrack.name;

    updateWindowTitle();
  }

  try {
    loadedTrack.id = getDatabase()->saveTrack(loadedTrack, settingSaveAsNew);

    veloTrack->resetModified();

    QMessageBox::information(nullptr, tr("Save Track"), message);
  } catch (VeloToolkitException& e) {
    e.Message();
  }  
}

void MainWindow::saveTrackToFile()
{
  if (loadedTrack.id == 0)
    return;

  QFile *file = new QFile("track.json");
  file->remove();
  file->open(QFile::ReadWrite);
  file->write(*veloTrack->exportAsJsonData());
  file->close();
}

void MainWindow::writeDefaultSettings()
{
  QString defaultFolder = getDefaultPath();
  if (defaultFolder != "") {
    productionSettingsDbFilename = defaultFolder + "/VelociDrone/settings.db";
    productionUserDbFilename = defaultFolder + "/VelociDrone/user11.db";
    betaSettingsDbFilename = defaultFolder + "/VelociDroneBeta/settings.db";
    betaUserDbFilename = defaultFolder + "/VelociDroneBeta/user11.db";
    customSettingsDbFilename = "";
    customUserDbFilename = "";

    writeSettings();
    return;
  }

  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->beginGroup("general");
  settings->setValue("viewTypeColumn", false);
  settings->setValue("saveTrackAsNew", true);
  settings->endGroup();

  settings->beginGroup("database");
  settings->setValue("productionUserDbFilename", defaultProductionUserDbFilename);
  settings->setValue("productionSettingsDbFilename", defaultProductionSettingsDbFilename);
  settings->setValue("betaUserDbFilename", defaultBetaUserDbFilename);
  settings->setValue("betaSettingsDbFilename", defaultBetaSettingsDbFilename);
  settings->setValue("customUserDbFilename", "");
  settings->setValue("customSettingsDbFilename", "");
  settings->endGroup();

  settings->beginGroup("archive");
  settings->setValue("moveToArchive", false);
  settings->setValue("filename", "");
  settings->endGroup();
}

void MainWindow::writeSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->beginGroup("general");
  settings->setValue("viewTypeColumn", settingViewTypeColumn);
  settings->setValue("saveTrackAsNew", settingSaveAsNew);
  settings->endGroup();

  settings->beginGroup("database");
  settings->setValue("productionUserDbFilename", productionUserDbFilename);
  settings->setValue("productionSettingsDbFilename", productionSettingsDbFilename);
  settings->setValue("betaUserDbFilename", betaUserDbFilename);
  settings->setValue("betaSettingsDbFilename", betaSettingsDbFilename);
  settings->setValue("customUserDbFilename", customUserDbFilename);
  settings->setValue("customSettingsDbFilename", customSettingsDbFilename);
  settings->endGroup();

  settings->beginGroup("archive");
  settings->setValue("moveToArchive", settingMoveToArchive);
  settings->setValue("filename", archiveDbFileName);
  settings->endGroup();
}

void MainWindow::loadTrack(const TrackData& track)
{
  closeTrack();

  VeloDb* veloDb = getDatabase();

  loadedTrack = track;

  updateWindowTitle();

  veloTrack->setPrefabs(veloDb->getPrefabs());
  try {
    veloTrack->importJsonData(&loadedTrack.value);
  } catch (VeloToolkitException& e) {
    e.Message();
  }

  ui->treeView->setModel(veloTrack->getStandardItemModel());

  for (QVector<SceneData>::iterator i = veloDb->getScenes()->begin(); i != veloDb->getScenes()->end(); ++i) {
    ui->sceneComboBox->addItem(i->title, i->id);
    if (i->id == loadedTrack.sceneId)
      ui->sceneComboBox->setCurrentText(i->title);
  }

  ui->treeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);

  if (!settingViewTypeColumn)
    ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);

  updateReplacePrefabComboBox();
}

void MainWindow::updateReplacePrefabComboBox()
{
  ui->replacePrefabComboBox->clear();
  QVector<PrefabData>* prefabsInUse = veloTrack->getPrefabsInUse();
  for (QVector<PrefabData>::iterator i = prefabsInUse->begin(); i != prefabsInUse->end(); ++i)
  {
    ui->replacePrefabComboBox->addItem(i->name, i->id);
  }
}

VeloDb* MainWindow::getDatabase()
{
  return getDatabase(loadedTrack.assignedDatabase);
}

VeloDb* MainWindow::getDatabase(DatabaseType databaseType)
{
  if (databaseType == DatabaseType::Production)
    return productionDb;
  else if (databaseType == DatabaseType::Beta)
    return betaDb;
  else if (databaseType == DatabaseType::Custom)
    return customDb;
  else
    return productionDb;
}

void MainWindow::setDatabaseOptionsDatabaseFilenames(const DatabaseType databaseType)
{
  databaseOptionsSelectedDbType = databaseType;

  switch (databaseType) {
  case DatabaseType::Production:
    ui->userDbLineEdit->setText(productionUserDbFilename);
    ui->settingsDbLineEdit->setText(productionSettingsDbFilename);
    break;

  case DatabaseType::Beta:
    ui->userDbLineEdit->setText(betaUserDbFilename);
    ui->settingsDbLineEdit->setText(betaSettingsDbFilename);
    break;

  case DatabaseType::Custom:
    ui->userDbLineEdit->setText(customUserDbFilename);
    ui->settingsDbLineEdit->setText(customSettingsDbFilename);
    break;
  default:
    return;
  }  
}

void MainWindow::setDatabaseOptionsSettingsDb(const QString& settingsDbFilename)
{
  QSettings* settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (databaseOptionsSelectedDbType) {
  case DatabaseType::Production:
    if (settingsDbFilename != productionSettingsDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        productionSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/productionSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case DatabaseType::Beta:
    if (settingsDbFilename != betaSettingsDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        betaSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/betaSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case DatabaseType::Custom:
    if (settingsDbFilename != customSettingsDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        customSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/customSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
  default:
    return;
  }

  getDatabase(databaseOptionsSelectedDbType)->setSettingsDbFilename(settingsDbFilename);

  delete settings;
}

void MainWindow::setDatabaseOptionsUserDb(const QString& userDbFilename)
{
  QSettings* settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (databaseOptionsSelectedDbType)
  {
  case DatabaseType::Production:
    if (userDbFilename != productionUserDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        productionUserDbFilename = userDbFilename;
        settings->setValue("database/productionUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case DatabaseType::Beta:
    if (userDbFilename != betaUserDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        betaUserDbFilename = userDbFilename;
        settings->setValue("database/betaUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case DatabaseType::Custom:
    if (userDbFilename != customUserDbFilename) {
      if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        customUserDbFilename = userDbFilename;
        settings->setValue("database/customUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
  default:
    return;
  }

  getDatabase(databaseOptionsSelectedDbType)->setUserDbFilename(userDbFilename);

  delete settings;
}

void MainWindow::updateDatabaseOptionsDatabaseStatus()
{
  QPalette palette = ui->databaseStatusValueLabel->palette();
  VeloDb* veloDb = getDatabase();
  if (veloDb->isValid())
  {
    palette.setColor(QPalette::WindowText, Qt::darkGreen);
    ui->databaseStatusValueLabel->setText(tr("Found"));
  } else {
    palette.setColor(QPalette::WindowText, Qt::darkRed);
    ui->databaseStatusValueLabel->setText(tr("Not Found"));
  }
  ui->databaseStatusValueLabel->setPalette(palette);
}

void MainWindow::updateWindowTitle()
{
  QString databaseStr = (loadedTrack.assignedDatabase == DatabaseType::Production ? "Production" : ((loadedTrack.assignedDatabase == DatabaseType::Beta) ? "Beta" : "Custom"));
  setWindowTitle(loadedTrack.name + " @ " + databaseStr + " - " + defaultWindowTitle);
}

QString MainWindow::browseDatabaseFile() const
{
  return QFileDialog::getOpenFileName(nullptr,
                                      tr("Choose Database"),
                                      "C:/Users/lephro/AppData/LocalLow/VelociDrone/",
                                      tr("Database Files (*.db)"));
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

void MainWindow::on_aboutLicensePushButton_released()
{
  if (ui->aboutLicenseTextEdit->isVisible()) {
    ui->aboutTextEdit->setVisible(true);
    ui->aboutLicenseTextEdit->setVisible(false);
    ui->aboutLicensePushButton->setText("License");
  } else {
    ui->aboutTextEdit->setVisible(false);
    ui->aboutLicenseTextEdit->setVisible(true);
    ui->aboutLicensePushButton->setText(tr("About"));
  }
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
  archiveDbFileName = result;
  writeSettings();
}

void MainWindow::on_trackArchiveSettingsFilepathLineEdit_textChanged(const QString &arg1)
{
  try {
    archive->setFileName(arg1);
    loadArchive();
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

}
