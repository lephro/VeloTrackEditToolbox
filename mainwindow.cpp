#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  productionDb = new VeloDb(Database::Production);
  betaDb = new VeloDb(Database::Beta);
  customDb = new VeloDb(Database::Custom);

  readSettings();

  dataParser = new VeloDataParser();

  ui->treeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, dataParser));

  updateDatabaseOptionsDatabaseStatus();
}

void MainWindow::closeTrack()
{
  loadedTrack = Track();
  dataParser->getModel()->clear();
}

void MainWindow::on_buildTypeComboBox_currentIndexChanged(int index)
{
  setDatabaseOptionsDatabaseFilenames(Database(index));
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

void MainWindow::on_openTrackPushButton_released()
{
  openTrack();
}

void MainWindow::on_replacePrefabComboBox_currentIndexChanged(int index)
{
  Q_UNUSED(index)

  Prefab selectedPrefab = dataParser->getPrefab(ui->replacePrefabComboBox->currentData().toUInt());
  ui->replacePrefabWithComboBox->clear();
  for (QVector<Prefab>::iterator i = dataParser->getPrefabs()->begin(); i != dataParser->getPrefabs()->end(); ++i) {
    if ((i->id != selectedPrefab.id) && (i->gate == selectedPrefab.gate)) {
      ui->replacePrefabWithComboBox->addItem(i->name, i->id);
    }
  }
}

void MainWindow::on_replacePushButton_released()
{
  replacePrefab();
}

void MainWindow::on_savePushButton_released()
{
  saveTrackToFile();
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
        Track selectedTrack = openTrackDialog.getSelectedTrack();
        if (selectedTrack.id > 0) {
          loadTrack(selectedTrack);
        }
      }
    }
  } catch (VeloToolkitException& e) {
    e.Message();
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
  settings->endGroup();

  productionDb->setSettingsDbFilename(productionSettingsDbFilename);
  productionDb->setUserDbFilename(productionUserDbFilename);
  betaDb->setSettingsDbFilename(betaSettingsDbFilename);
  betaDb->setUserDbFilename(betaUserDbFilename);
  customDb->setSettingsDbFilename(customSettingsDbFilename);
  customDb->setUserDbFilename(customUserDbFilename);

  setDatabaseOptionsDatabaseFilenames(Database::Production);
}

void MainWindow::replacePrefab()
{
  QModelIndex searchIndex;
  switch (ui->replacePrefabWhereComboBox->currentIndex()) {

  case 0: // All nodes
    searchIndex = dataParser->getRootIndex();
    break;

  case 1: // Barriers
    searchIndex = dataParser->getRootIndex();
    for (int i = 0; i < dataParser->getModel()->rowCount(searchIndex); ++i) {
      QModelIndex childIndex = dataParser->getModel()->index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "barriers") {
        searchIndex = childIndex;
        break;
      }
    }

    if (!searchIndex.isValid())
      return;

    break;

  case 2: // Gates
    searchIndex = dataParser->getRootIndex();
    for (int i = 0; i < dataParser->getModel()->rowCount(searchIndex); ++i) {
      QModelIndex childIndex = dataParser->getModel()->index(i, 0, searchIndex);
      if (childIndex.data(Qt::DisplayRole).toString() ==  "gates") {
        searchIndex = childIndex;
        break;
      }
    }

    if (!searchIndex.isValid())
      return;

    break;

  case 3: // Selected Node
    searchIndex = ui->treeView->currentIndex();

    if (!searchIndex.isValid())
      return;

    break;
  }
  uint changedPrefabCount = dataParser->replacePrefab(searchIndex,
                                                      ui->replacePrefabComboBox->currentData().toUInt(),
                                                      ui->replacePrefabWithComboBox->currentData().toUInt());
  QString changedPrefabInfo = tr("%1 occurence(s) replaced");
  QMessageBox::information(this, tr("Replace successfull"), changedPrefabInfo.arg(changedPrefabCount, 0, 10));

  updateReplacePrefabComboBox();
}

bool MainWindow::maybeSave()
{
    if (!trackModified)
        return true;
    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this,
                               tr("Application"),
                               tr("The track has been modified.\n"
                                  "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return saveTrackToDb();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

bool MainWindow::saveTrackToDb()
{
  return true;
}

bool MainWindow::saveTrackToFile()
{
  QFile *file = new QFile("track.json");
  file->remove();
  file->open(QFile::ReadWrite);
  file->write(*dataParser->exportTrackDataFromModel());
  file->close();
  return true;
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
  settings->endGroup();
}

void MainWindow::loadTrack(const Track& track)
{
  closeTrack();

  trackModified = true;

  VeloDb* veloDb = getDatabase();

  for (QVector<Track>::iterator i = veloDb->getTracks()->begin(); i != veloDb->getTracks()->end(); ++i) {
    if (i->name == track.name) {
      loadedTrack = track;

      dataParser->setPrefabs(veloDb->getPrefabs());
      if (dataParser->importTrackDataToModel(&i->value) == 0) {
        ui->treeView->setModel(dataParser->getModel());
      }

      for (QVector<Scene>::iterator i = veloDb->getScenes()->begin(); i != veloDb->getScenes()->end(); ++i) {
        ui->sceneComboBox->addItem(i->title, i->id);
        if (i->id == loadedTrack.sceneId)
          ui->sceneComboBox->setCurrentText(i->title);
      }

      ui->treeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);
      ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);

      updateReplacePrefabComboBox();

      return;
    }
  }
}

void MainWindow::updateReplacePrefabComboBox()
{
  ui->replacePrefabComboBox->clear();
  QVector<Prefab>* prefabsInUse = dataParser->getPrefabsInUse();
  for (QVector<Prefab>::iterator i = prefabsInUse->begin(); i != prefabsInUse->end(); ++i)
  {
    ui->replacePrefabComboBox->addItem(i->name, i->id);
  }
}

VeloDb* MainWindow::getDatabase()
{
  return getDatabase(loadedTrack.database);
}

VeloDb* MainWindow::getDatabase(Database database)
{
  if (database == Database::Production)
    return productionDb;
  else if (database == Database::Beta)
    return betaDb;
  else if (database == Database::Custom)
    return customDb;
  else
    return productionDb;
}

void MainWindow::setDatabaseOptionsDatabaseFilenames(const Database database)
{
  databaseOptionsSelectedDb = database;

  switch (database) {
  case Database::Production:
    ui->userDbLineEdit->setText(productionUserDbFilename);
    ui->settingsDbLineEdit->setText(productionSettingsDbFilename);
    break;

  case Database::Beta:
    ui->userDbLineEdit->setText(betaUserDbFilename);
    ui->settingsDbLineEdit->setText(betaSettingsDbFilename);
    break;

  case Database::Custom:
    ui->userDbLineEdit->setText(customUserDbFilename);
    ui->settingsDbLineEdit->setText(customSettingsDbFilename);
    break;
  }  
}

void MainWindow::setDatabaseOptionsSettingsDb(const QString& settingsDbFilename)
{
  QSettings* settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (databaseOptionsSelectedDb) {
  case Database::Production:
    if (settingsDbFilename != productionSettingsDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        productionSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/productionSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case Database::Beta:
    if (settingsDbFilename != betaSettingsDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        betaSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/betaSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case Database::Custom:
    if (settingsDbFilename != customSettingsDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        customSettingsDbFilename = settingsDbFilename;
        settings->setValue("database/customSettingsDbFilename", settingsDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
  }

  getDatabase(databaseOptionsSelectedDb)->setSettingsDbFilename(settingsDbFilename);

  delete settings;
}

void MainWindow::setDatabaseOptionsUserDb(const QString& userDbFilename)
{
  QSettings* settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (databaseOptionsSelectedDb)
  {
  case Database::Production:
    if (userDbFilename != productionUserDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        productionUserDbFilename = userDbFilename;
        settings->setValue("database/productionUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case Database::Beta:
    if (userDbFilename != betaUserDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        betaUserDbFilename = userDbFilename;
        settings->setValue("database/betaUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;

  case Database::Custom:
    if (userDbFilename != customUserDbFilename) {
      if ((loadedTrack.database == databaseOptionsSelectedDb) && (!maybeSave())) {
        setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDb);
      } else {
        customUserDbFilename = userDbFilename;
        settings->setValue("database/customUserDbFilename", userDbFilename);

        updateDatabaseOptionsDatabaseStatus();

        closeTrack();
      }
    }
    break;
  }

  getDatabase(databaseOptionsSelectedDb)->setUserDbFilename(userDbFilename);

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

QString MainWindow::browseDatabaseFile() const
{
  return QFileDialog::getOpenFileName(nullptr,
                                      tr("Choose Database"),
                                      "C:/Users/lephro/AppData/LocalLow/VelociDrone/",
                                      tr("Database Files (*.db)"));
}
