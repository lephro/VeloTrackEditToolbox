#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  readSettings();

  productionDb = new VeloDb(Database::Production, &productionUserDbFilename, &productionSettingsDbFilename);
  betaDb = new VeloDb(Database::Beta, &betaUserDbFilename, &betaSettingsDbFilename);
  customDb = new VeloDb(Database::Custom, &customUserDbFilename, &customSettingsDbFilename);

  dataParser = new VeloDataParser();

  if (productionDb->queryAll()) {
    //Todo: Error Handling
  }

  dataParser->setPrefabs(productionDb->getPrefabs());

  for (QVector<Scene>::iterator i = productionDb->getScenes()->begin(); i != productionDb->getScenes()->end(); ++i) {
    ui->sceneComboBox->addItem(i->title, i->id);
  }

  ui->treeView->setItemDelegateForColumn(1, new JsonTreeViewItemDelegate(nullptr, dataParser));

  updateDatabaseStatus();
}


void MainWindow::on_buildTypeComboBox_currentIndexChanged(int index)
{
  switchCurrentDbSelection(Database(index));
  updateDatabaseStatus();
}

void MainWindow::on_browseUserDbToolButton_released()
{
  QString result = browseDatabaseFile();
  if (result != "") {
    setUserDb(result);
    ui->userDbLineEdit->setText(result);
  }
  updateDatabaseStatus();
}

void MainWindow::on_browseSettingsDbToolButton_released()
{
  QString result = browseDatabaseFile();
  if (result != "") {
    setSettingsDb(result);
    ui->settingsDbLineEdit->setText(result);
  }
  updateDatabaseStatus();
}


void MainWindow::on_openTrackPushButton_released()
{
  openTrack();
  //refreshDbInfo();
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
  setSettingsDb(arg1);
}

void MainWindow::on_userDbLineEdit_textChanged(const QString &arg1)
{
  setUserDb(arg1);
}

void MainWindow::openTrack()
{
  OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
  if (openTrackDialog.exec()) {
    Track selectedTrack = openTrackDialog.getSelectedTrack();
    if (selectedTrack.id > 0) {
      currentTrack = selectedTrack;
      refreshDbInfo();
    }
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
  productionUserDbFilename = settings->value("productionUserDbFilename", "").toString();
  productionSettingsDbFilename = settings->value("productionSettingsDbFilename", "").toString();
  betaUserDbFilename = settings->value("betaUserDbFilename", "").toString();
  betaSettingsDbFilename = settings->value("betaSettingsDbFilename", "").toString();
  customUserDbFilename = settings->value("customUserDbFilename", "").toString();
  customSettingsDbFilename = settings->value("customSettingsDbFilepath", "").toString();
  settings->endGroup();

  switchCurrentDbSelection(Database(0));
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

  refreshPrefabsInUse();
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

void MainWindow::refreshDbInfo()
{
  productionDb->queryAll();

  for (QVector<Track>::iterator i = productionDb->getTracks()->begin(); i != productionDb->getTracks()->end(); ++i) {
    if (i->name == currentTrack.name) {
      if (dataParser->importTrackDataToModel(&i->value) == 0) {
        ui->treeView->setModel(dataParser->getModel());
      }
      ui->treeView->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
      //ui->treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
      ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);

      refreshPrefabsInUse();

      return;
    }
  }
}

void MainWindow::refreshPrefabsInUse()
{
  ui->replacePrefabComboBox->clear();
  QVector<Prefab>* prefabsInUse = dataParser->getPrefabsInUse();
  for (QVector<Prefab>::iterator i = prefabsInUse->begin(); i != prefabsInUse->end(); ++i)
  {
    ui->replacePrefabComboBox->addItem(i->name, i->id);
  }
}

void MainWindow::switchCurrentDbSelection(const Database database)
{
  currentDbSelection = database;

  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);

  switch (database)
  {
  case Database::Production:
    settings->setValue("database/productionUserDbFilename", productionUserDbFilename);
    settings->setValue("database/productionSettingsDbFilename", productionSettingsDbFilename);
    ui->userDbLineEdit->setText(productionUserDbFilename);
    ui->settingsDbLineEdit->setText(productionSettingsDbFilename);
    break;

  case Database::Beta:
    settings->setValue("database/betaUserDbFilename", betaUserDbFilename);
    settings->setValue("database/betaSettingsDbFilename", betaSettingsDbFilename);
    ui->userDbLineEdit->setText(betaUserDbFilename);
    ui->settingsDbLineEdit->setText(betaSettingsDbFilename);
    break;

  case Database::Custom:
    settings->setValue("database/customUserDbFilename", customUserDbFilename);
    settings->setValue("database/customSettingsDbFilename", customSettingsDbFilename);
    ui->userDbLineEdit->setText(customUserDbFilename);
    ui->settingsDbLineEdit->setText(customSettingsDbFilename);
    break;
  }

  delete settings;
}

void MainWindow::setUserDb(const QString userDbFilename)
{
  switch (currentDbSelection)
  {
  case Database::Production:
    if (userDbFilename != productionUserDbFilename) {
      ui->userDbLineEdit->setText(userDbFilename);
      productionUserDbFilename = userDbFilename;
    }
    break;

  case Database::Beta:
    if (userDbFilename != betaUserDbFilename) {
      ui->userDbLineEdit->setText(userDbFilename);
      betaUserDbFilename = userDbFilename;
    }
    break;

  case Database::Custom:
    if (userDbFilename != customUserDbFilename) {
      ui->userDbLineEdit->setText(userDbFilename);
      customUserDbFilename = userDbFilename;
    }
    break;
  }
}

void MainWindow::setSettingsDb(const QString settingsDbFilename)
{
  switch (currentDbSelection)
  {
  case Database::Production:
    if (settingsDbFilename != productionUserDbFilename) {
      ui->settingsDbLineEdit->setText(settingsDbFilename);
      productionSettingsDbFilename = settingsDbFilename;
    }
    break;

  case Database::Beta:
    if (settingsDbFilename != productionUserDbFilename) {
      ui->settingsDbLineEdit->setText(settingsDbFilename);
      betaSettingsDbFilename = settingsDbFilename;
    }
    break;

  case Database::Custom:
    if (settingsDbFilename != productionUserDbFilename) {
      ui->settingsDbLineEdit->setText(settingsDbFilename);
      customSettingsDbFilename = settingsDbFilename;
    }
    break;
  }
}

void MainWindow::updateDatabaseStatus()
{
  QPalette palette = ui->databaseStatusValueLabel->palette();
  if (((currentDbSelection == Database::Production) && (productionDb->isValid())) ||
      ((currentDbSelection == Database::Beta) && (betaDb->isValid())) ||
      ((currentDbSelection == Database::Custom) && (customDb->isValid())))
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
