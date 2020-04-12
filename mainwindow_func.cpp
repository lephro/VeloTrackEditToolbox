#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::closeTrack()
{
  setWindowTitle(defaultWindowTitle);

  nodeCount = 0;
  prefabCount = 0;
  splineCount = 0;
  gateCount = 0;

  loadedTrack = TrackData();

  veloTrack->getStandardItemModel()->clear();
  ui->sceneComboBox->clear();
  ui->replacePrefabComboBox->clear();
  ui->replacePrefabWithComboBox->clear();
  updateStatusBar();
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
                             tr("You haven't set up any trackarchive, yet\nDo you want to create/open an archive file now?"),
                             QMessageBox::Yes | QMessageBox::No);

  QString result = "";

  switch (ret) {
  case QMessageBox::Yes:
    result = QFileDialog::getSaveFileName(this,
                                          tr("Choose or create an archive"),
                                          QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(),
                                          tr("Database Files (*.db)"),
                                          nullptr,
                                          QFileDialog::Option::DontConfirmOverwrite);

    if (result == "")
      return false;

    ui->trackArchiveSettingsFilepathLineEdit->setText(result);
    archiveDbFileName = result;
    writeSettings();
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

  QString message = tr("The track \"%1\" was saved successfully to the database!");

  if (settingSaveAsNew) {
    loadedTrack.name += "-new";
    message += tr("\n\nNew track name: ") + loadedTrack.name;

    updateWindowTitle();
  }

  try {
    loadedTrack.id = getDatabase()->saveTrack(loadedTrack, settingSaveAsNew);

    veloTrack->resetModified();

    QMessageBox::information(nullptr, tr("Save Track"), message);
    statusBar()->showMessage(tr("Track saved!"), 2000);
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

  gateCount = veloTrack->getGateCount();
  prefabCount = veloTrack->getPrefabCount();
  nodeCount = veloTrack->getNodeCount();
  splineCount = veloTrack->getSplineCount();

  for (QVector<SceneData>::iterator i = veloDb->getScenes()->begin(); i != veloDb->getScenes()->end(); ++i) {
    ui->sceneComboBox->addItem(i->title, i->id);
    if (i->id == loadedTrack.sceneId)
      ui->sceneComboBox->setCurrentText(i->title);
  }

  ui->treeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);

  if (!settingViewTypeColumn)
    ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);

  updateReplacePrefabComboBox();

  updateStatusBar();

  statusBar()->showMessage(tr("Track loaded successfully."), 2000);
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

void MainWindow::updateStatusBar()
{
  nodeCountLabel->setText(nodeCountLabelText.arg(nodeCount, 0, 10));
  prefabCountLabel->setText(prefabCountLabelText.arg(prefabCount, 0, 10));
  gateCountLabel->setText(gateCountLabelText.arg(gateCount, 0, 10));
  splineCountLabel->setText(splineCountLabelText.arg(splineCount, 0, 10));
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
