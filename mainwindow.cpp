#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Create the flow layout for the search filter
  searchFilterLayout = new SearchFilterLayout();
  ui->searchFilterGroupBox->setLayout(searchFilterLayout);

  // Set the default window title
  defaultWindowTitle = QString(windowTitle());

  // Hook up the dynamic tab control height update function to the tab change event
  connect(ui->nodeEditorToolsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateDynamicTabControlSize(int)));

  // Create the labels for the status bar and add them to it
  updateStatusBar();
  statusBar()->addPermanentWidget(&nodeCountLabel);
  statusBar()->addPermanentWidget(&prefabCountLabel);
  statusBar()->addPermanentWidget(&gateCountLabel);
  statusBar()->addPermanentWidget(&splineCountLabel);

  // Setup our initial view for the search tab
  ui->searchTypeComboBox->setCurrentIndex(FilterTypes::Object);
  ui->searchValueSpinBox->setVisible(false);  

  // Set the about page as default and hide its button
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
  ui->aboutPushButton->setVisible(false);

  // Set the regex that validates the track name
  QRegularExpression regEx("^[A-Za-z][\\w\\s\\-]+");
  QRegularExpressionValidator* trackNameValidator = new QRegularExpressionValidator(regEx, this);
  ui->mergeTrackNewTrackNameLineEdit->setValidator(trackNameValidator);

  // Create our database and track objects
  productionDb = new VeloDb(DatabaseType::Production);
  betaDb = new VeloDb(DatabaseType::Beta);
  customDb = new VeloDb(DatabaseType::Custom);
  archive = new TrackArchive(this);

  try {
    // Read the config
    readSettings();
  } catch (VeloToolkitException& e) {
    e.Message();
  }    

  // Hook up our delegation for the node value control
  ui->treeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, &veloTrack));

  // Update our database status indicator in the setup page
  updateDatabaseOptionsDatabaseStatus();

  // Set config values to its according controls
  ui->archiveMoveToArchiveCheckBox->setChecked(settingMoveToArchive);
  ui->saveAsNewCheckbox->setChecked(settingSaveAsNew);
  ui->trackArchiveSettingsFilepathLineEdit->setText(archiveDbFileName);

  // Insert every found database into the database selection combo box of the archive
  if (productionDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(0, tr("Production"));

  if (betaDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(1, tr("Beta"));

  if (customDb->isValid())
    ui->archiveDatabaseSelectionComboBox->insertItem(2, tr("Custom"));

  // Show the database selection combo box of the archive only if we got multiple databases
  ui->archiveDatabaseSelectionComboBox->setVisible(ui->archiveDatabaseSelectionComboBox->count() > 1);

  // If no databases were present output a warning
  if (ui->archiveDatabaseSelectionComboBox->count() == 0)
    QMessageBox::information(this, "No databases found!", "The VeloTrackToolkit could not find any databases.\nPlease go to the options page and select the nescessary database files.");

  // Set the node editor as the default page
  ui->navListWidget->setCurrentRow(NavRows::NodeEditorRow);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  // Check if we got unwritten changes and warn the user before closing
  if(maybeSave()) {
    writeSettings();
    e->accept();
  } else
    e->ignore();
}

VeloDb* MainWindow::getDatabase()
{
  // return the database of the loaded track
  return getDatabase(loadedTrack.assignedDatabase);
}

VeloDb* MainWindow::getDatabase(DatabaseType databaseType)
{
  // return the database according to the type given
  if (databaseType == DatabaseType::Production)
    return productionDb;
  else if (databaseType == DatabaseType::Beta)
    return betaDb;
  else if (databaseType == DatabaseType::Custom)
    return customDb;
  else
    return productionDb;
}

QString MainWindow::getDefaultPath()
{
  // 4 Windoze only
  // fuck apple
  return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first().replace("AppData/Local", "AppData/LocalLow/VelociDrone");
}

bool MainWindow::maybeSave()
{
  // if the track wasn't modified and the scene didn't change there is nothing to do here
  if (!veloTrack.isModified() && (loadedTrack.sceneId == ui->sceneComboBox->currentData().toUInt()))
    return true;

  // Ask the user if he wants to save his changes or cancel the process
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

void MainWindow::readSettings()
{
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  //settings->clear();

  // Try to read the config, if default config. Otherwise load the defaults
  if (settings->value("database/productionUserDbFilename", "").toString() == "") {
    settings->clear();
    writeDefaultSettings();
    settings->sync();
  }

  // Read the settings from the config file to its according variables
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

  // Update the database filename settings controls
  setDatabaseOptionsDatabaseFilenames(DatabaseType::Production);
}

void MainWindow::updateStatusBar()
{
  // Parse the count values into their templates and update the labels with it
  nodeCountLabel.setText(nodeCountLabelText.arg(nodeCount, 0, 10));
  prefabCountLabel.setText(prefabCountLabelText.arg(prefabCount, 0, 10));
  gateCountLabel.setText(gateCountLabelText.arg(gateCount, 0, 10));
  splineCountLabel.setText(splineCountLabelText.arg(splineCount, 0, 10));
}

void MainWindow::updateWindowTitle()
{
  // Determinate the database and set the window title
  QString databaseStr = (loadedTrack.assignedDatabase == DatabaseType::Production ? "Production" : ((loadedTrack.assignedDatabase == DatabaseType::Beta) ? "Beta" : "Custom"));
  setWindowTitle(loadedTrack.name + " @ " + databaseStr + " - " + defaultWindowTitle);
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
  // Write all settings into the config file
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

void MainWindow::on_navListWidget_currentRowChanged(int currentRow)
{
  // Do stuff depending on which page the user switches to
  switch (currentRow) {
  case NavRows::AboutRow:
    // Reset the about buttons and view
    ui->aboutLicensePushButton->setVisible(true);
    ui->aboutPatchLogPushButton->setVisible(true);
    ui->aboutPushButton->setVisible(false);
    ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
    break;
  // Archive page
  case NavRows::ArchiveRow:
    // Check if an archive is set, otherwise ask for it
    maybeCreateOrSelectArchive();
    break;
  }
}






