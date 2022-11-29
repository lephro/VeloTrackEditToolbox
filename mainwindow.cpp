#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  // Set the default window title
  defaultWindowTitle = QString(windowTitle());

  nodeEditorManager = new EditorManager(*this, *ui->nodeEditorTabWidget);

  // Create the labels for the status bar and add them to it
  //updateStatusBar();
  statusBar()->addPermanentWidget(&filterCountLabel);
  statusBar()->addPermanentWidget(&splineCountLabel);
  statusBar()->addPermanentWidget(&gateCountLabel);
  statusBar()->addPermanentWidget(&objectCountLabel);
  statusBar()->addPermanentWidget(&nodeCountLabel);

  // Create the flow layout for the search filter
  searchFilterLayout = new SearchFilterLayout();
  ui->searchFilterGroupBox->setLayout(searchFilterLayout);  
  connect(searchFilterLayout, SIGNAL(filterChanged()), this, SLOT(onSearchFilterChanged()));
  ui->searchFilterGroupBox->hide();
  ui->searchClearFilterPushButton->hide();

  // Hook up the dynamic tab control height update function to the tab change event
  //connect(ui->nodeEditorToolsTabWidget, SIGNAL(currentChanged(int)), this, SLOT(updateDynamicTabControlSize(int)));

  // Setup our initial view for the search tab
  ui->nodeEditorToolsTabWidget->setCurrentIndex(0);
  ui->searchFilterValueStackedWidget->setCurrentIndex(0);
  on_searchTypeComboBox_currentIndexChanged(NodeFilter::getDescriptionFromFilterType(FilterTypes::Object));
  on_toolsTypeComboBox_currentIndexChanged(0);

  // Set the about page as default and hide its button
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
  ui->aboutPushButton->hide();

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

  openTrackDialog = new OpenTrackDialog(this, productionDb, betaDb, customDb);

  // Hook up our delegation for the node value control
  //ui->nodeTreeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, &nodeEditor));

  // Update our database status indicator in the setup page
  updateDatabaseOptionsDatabaseStatus();

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
    QMessageBox::information(this, tr("No databases found!"), tr("The VeloTrackToolkit could not find any databases.\nPlease go to the options page and select the nescessary database files."));

  // Set the node editor as the default page
  ui->navListWidget->setCurrentRow(NavRows::NodeEditorRow);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  // Check if we got unwritten changes and warn the user before closing
  if(maybeSave()) {
    e->accept();
  } else
    e->ignore();
}

VeloDb* MainWindow::getDatabase()
{
  // return the database of the loaded track
  NodeEditor* editor = nodeEditorManager->getEditor();
  if (editor != nullptr)
    return getDatabase(editor->getTrackData().assignedDatabase);

  // No track loaded. Get the first initialized
  return getDatabase(DatabaseType::Production);
}

VeloDb* MainWindow::getDatabase(const DatabaseType databaseType)
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
#ifdef Q_OS_WIN
  return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first().replace("AppData/Local", "AppData/LocalLow/VelociDrone");
#else
  return "";
#endif
}

bool MainWindow::maybeSave()
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return true;

  // if the track wasn't modified and the scene didn't change there is nothing to do here
  if (!nodeEditor->isModified() && (nodeEditor->getTrackData().sceneId == ui->sceneComboBox->currentData().toUInt()))
    return true;

  // Ask the user if he wants to save his changes or cancel the process
  const QMessageBox::StandardButton ret
    = QMessageBox::warning(this,
                           defaultWindowTitle,
                           tr("Do you want to save your changes on \"") + nodeEditor->getTrackData().name + tr("\"?"),
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
  QSettings settings("settings.ini", QSettings::IniFormat);
  //settings->clear();  

  const QString defaultFolder = getDefaultPath();
  if (defaultFolder != "") {
    defaultProductionSettingsDbFilename = defaultFolder + "/VelociDrone/settings.db";
    defaultProductionUserDbFilename = defaultFolder + "/VelociDrone/user11.db";
    defaultBetaSettingsDbFilename = defaultFolder + "/VelociDroneBeta/settings.db";
    defaultBetaUserDbFilename = defaultFolder + "/VelociDroneBeta/user11.db";
  }

  // Read the settings from the config file to its according variables
  settings.beginGroup("general");
  ui->saveAsNewCheckbox->setChecked(settings.value("saveTrackAsNew", true).toString().toLower() == "true");
  ui->viewNodeTypeColumn->setChecked(settings.value("viewTypeColumn", false).toString().toLower() == "true");

  const QColor filterColor = QColor(settings.value("filterColorR", 254).toInt(),
                                    settings.value("filterColorG", 203).toInt(),
                                    settings.value("filterColorB", 137).toInt());
  if (filterColor.isValid()) {
    ui->filterColorPushButton->setStyleSheet("background-color: " + filterColor.name());
    nodeEditorManager->setFilterColor(filterColor);
  }

  const QColor filterFontColor = QColor(settings.value("filterFontColorR", 0).toInt(),
                                        settings.value("filterFontColorG", 0).toInt(),
                                        settings.value("filterFontColorB", 0).toInt());
  if (filterFontColor.isValid()) {
    ui->filterColorFontPushButton->setStyleSheet("background-color: " + filterFontColor.name());
    nodeEditorManager->setFilterFontColor(filterFontColor);
  }

  const QColor filterParentColor = QColor(settings.value("filterParentColorR", 192).toInt(),
                                          settings.value("filterParentColorG", 192).toInt(),
                                          settings.value("filterParentColorB", 192).toInt());
  if (filterParentColor.isValid()) {
    ui->filterColorParentPushButton->setStyleSheet("background-color: " + filterParentColor.name());
    nodeEditorManager->setFilterParentColor(filterParentColor);
  }

  const QColor filterParentFontColor = QColor(settings.value("filterParentFontColorR", 0).toInt(),
                                              settings.value("filterParentFontColorG", 0).toInt(),
                                              settings.value("filterParentFontColorB", 0).toInt());
  if (filterParentFontColor.isValid()) {
    ui->filterColorParentFontPushButton->setStyleSheet("background-color: " + filterParentFontColor.name());
    nodeEditorManager->setFilterParentFontColor(filterParentFontColor);
  }

  settings.endGroup();

  settings.beginGroup("database");
  productionDb->setSettingsDbFilename(settings.value("productionSettingsDbFilename", defaultProductionSettingsDbFilename).toString());
  productionDb->setUserDbFilename(settings.value("productionUserDbFilename", defaultProductionUserDbFilename).toString());
  betaDb->setSettingsDbFilename(settings.value("betaSettingsDbFilename", defaultBetaSettingsDbFilename).toString());
  betaDb->setUserDbFilename(settings.value("betaUserDbFilename", defaultBetaUserDbFilename).toString());
  customDb->setSettingsDbFilename(settings.value("customSettingsDbFilename", "").toString());
  customDb->setUserDbFilename(settings.value("customUserDbFilename", "").toString());
  settings.endGroup();

  settings.beginGroup("archive");
  ui->archiveMoveToArchiveCheckBox->setChecked(settings.value("moveToArchive", false).toBool());
  ui->archiveSettingsFilepathLineEdit->setText(settings.value("filename", "").toString());
  settings.endGroup();

  // Update the database filename settings controls
  setDatabaseOptionsDatabaseFilenames(DatabaseType::Production);
}

void MainWindow::updateStatusBar()
{ 
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();

  // Parse the count values into their templates and update the labels with it
  objectCountLabel.setText(prefabCountLabelText.arg(nodeEditor == nullptr ? 0 : nodeEditor->getTrack()->getObjectCount()));
  gateCountLabel.setText(gateCountLabelText.arg(nodeEditor == nullptr ? 0 : nodeEditor->getTrack()->getGateCount()));
  splineCountLabel.setText(splineCountLabelText.arg(nodeEditor == nullptr ? 0 : nodeEditor->getTrack()->getSplineCount()));
  const int searchResultCount = nodeEditor == nullptr ? 0 : nodeEditor->getSearchResult().count();
  filterCountLabel.setText(searchResultCount == 0 ? "" : filterCountLabelText.arg(searchResultCount));
}

void MainWindow::updateWindowTitle()
{
  NodeEditor* editor = nodeEditorManager->getEditor();
  if (editor == nullptr)
    setWindowTitle(defaultWindowTitle);

  // Determinate the database and set the window title
  QString databaseStr = (editor->getTrackData().assignedDatabase == DatabaseType::Production ? "Production" : ((editor->getTrackData().assignedDatabase == DatabaseType::Beta) ? "Beta" : "Custom"));
  setWindowTitle(editor->getTrackData().name + " @ " + databaseStr + " - " + defaultWindowTitle);
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


