#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  defaultWindowTitle = QString(windowTitle());

  productionDb = new VeloDb(DatabaseType::Production);
  betaDb = new VeloDb(DatabaseType::Beta);
  customDb = new VeloDb(DatabaseType::Custom);
  veloTrack = new VeloTrack();

  try {
    readSettings();
  } catch (VeloToolkitException& e) {
    e.Message();
  }

  ui->treeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, veloTrack));

  ui->saveAsNewCheckbox->setChecked(saveAsNew);

  updateDatabaseOptionsDatabaseStatus();
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
  if (loadedTrack.id == 0)
    return;

  QMessageBox::StandardButtons ret = QMessageBox::question(this, tr("Are you sure?"), tr("Do you really want to delete this track?\nMAKE SURE TO ENABLE YOUR BRAIN NOW!"));
  if (ret != QMessageBox::Yes)
    return;

  try {
   getDatabase()->deleteTrack(loadedTrack);
  } catch (VeloToolkitException& e) {
    e.Message();
    closeTrack();
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
  saveAsNew = bool(arg1);

  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->setValue("database/saveTrackAsNew", saveAsNew);
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

void MainWindow::readSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  //settings->clear();

  if (settings->value("database/productionUserDbFilename", "").toString() == "") {
    settings->clear();
    writeDefaultSettings();
    settings->sync();
  }

  settings->beginGroup("database");
  productionSettingsDbFilename = settings->value("productionSettingsDbFilename", "").toString();
  productionUserDbFilename = settings->value("productionUserDbFilename", "").toString();
  betaSettingsDbFilename = settings->value("betaSettingsDbFilename", "").toString();
  betaUserDbFilename = settings->value("betaUserDbFilename", "").toString();
  customSettingsDbFilename = settings->value("customSettingsDbFilename", "").toString();
  customUserDbFilename = settings->value("customUserDbFilename", "").toString();
  saveAsNew = settings->value("saveTrackAsNew", true).toBool();
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

  if (!searchIndex.isValid())
    return;

  uint changedPrefabCount = veloTrack->replacePrefab(searchIndex,
                                                      ui->replacePrefabComboBox->currentData().toUInt(),
                                                      ui->replacePrefabWithComboBox->currentData().toUInt());

  QString changedPrefabInfo = tr("%1 occurence(s) replaced");
  QMessageBox::information(this, tr("Replace successfull"), changedPrefabInfo.arg(changedPrefabCount, 0, 10));

  updateReplacePrefabComboBox();
}

bool MainWindow::maybeSave()
{
    if (!veloTrack->isModified())
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this,
                               defaultWindowTitle,
                               tr("The track has been modified.\n"
                                  "Do you want to save your changes?"),
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
  if (loadedTrack.id == 0)
    return;

  loadedTrack.sceneId = ui->sceneComboBox->currentData().toUInt();
  loadedTrack.value = *veloTrack->exportTrackDataFromModel();

  QString message = tr("The track was saved successfully to the database!");
  if (saveAsNew) {
    loadedTrack.name += "-new";
    message += tr("\n\nNew track name: ") + loadedTrack.name;

    updateWindowTitle();
  }

  try {
    loadedTrack.id = getDatabase()->saveTrack(loadedTrack, saveAsNew);

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
  file->write(*veloTrack->exportTrackDataFromModel());
  file->close();
}

void MainWindow::writeDefaultSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->beginGroup("database");
  settings->setValue("productionUserDbFilename", defaultProductionUserDbFilename);
  settings->setValue("productionSettingsDbFilename", defaultProductionSettingsDbFilename);
  settings->setValue("betaUserDbFilename", defaultBetaUserDbFilename);
  settings->setValue("betaSettingsDbFilename", defaultBetaSettingsDbFilename);
  settings->setValue("customUserDbFilename", "");
  settings->setValue("customSettingsDbFilename", "");
  settings->setValue("saveTrackAsNew", true);
  settings->endGroup();
}

void MainWindow::writeSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->beginGroup("database");
  settings->setValue("productionUserDbFilename", productionUserDbFilename);
  settings->setValue("productionSettingsDbFilename", productionSettingsDbFilename);
  settings->setValue("betaUserDbFilename", betaUserDbFilename);
  settings->setValue("betaSettingsDbFilename", betaSettingsDbFilename);
  settings->setValue("customUserDbFilename", customUserDbFilename);
  settings->setValue("customSettingsDbFilename", customSettingsDbFilename);
  settings->setValue("saveTrackAsNew", saveAsNew);
  settings->endGroup();
}

void MainWindow::loadTrack(const TrackData& track)
{
  closeTrack();

  VeloDb* veloDb = getDatabase();

  loadedTrack = track;

  updateWindowTitle();

  veloTrack->setPrefabs(veloDb->getPrefabs());
  veloTrack->importTrackDataToModel(&loadedTrack.value);
  ui->treeView->setModel(veloTrack->getStandardItemModel());

  for (QVector<SceneData>::iterator i = veloDb->getScenes()->begin(); i != veloDb->getScenes()->end(); ++i) {
    ui->sceneComboBox->addItem(i->title, i->id);
    if (i->id == loadedTrack.sceneId)
      ui->sceneComboBox->setCurrentText(i->title);
  }

  ui->treeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);
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
  return getDatabase(loadedTrack.database);
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
  }  
}

void MainWindow::setDatabaseOptionsSettingsDb(const QString& settingsDbFilename)
{
  QSettings* settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (databaseOptionsSelectedDbType) {
  case DatabaseType::Production:
    if (settingsDbFilename != productionSettingsDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
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
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
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
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        customSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/customSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
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
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
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
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
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
      if ((loadedTrack.database == databaseOptionsSelectedDbType) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
      } else {
        customUserDbFilename = userDbFilename;
        settings->setValue("database/customUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
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
  QString databaseStr = (loadedTrack.database == DatabaseType::Production ? "Production" : ((loadedTrack.database == DatabaseType::Beta) ? "Beta" : "Custom"));
  setWindowTitle(loadedTrack.name + " @ " + databaseStr + " - " + defaultWindowTitle);
}

QString MainWindow::browseDatabaseFile() const
{
  return QFileDialog::getOpenFileName(nullptr,
                                      tr("Choose Database"),
                                      "C:/Users/lephro/AppData/LocalLow/VelociDrone/",
                                      tr("Database Files (*.db)"));
}
