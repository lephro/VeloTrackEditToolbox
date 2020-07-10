#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  // Set the default window title
  defaultWindowTitle = QString(windowTitle());

  // Create and connect the context menu of the node edito
  nodeEditorContextMenu.addAction(QIcon(":/icons/add-filter"), tr("Add to filter"), this, SLOT(onNodeEditorContextMenuAddToFilterAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/copy-add"), tr("D&ublicate"), this, SLOT(onNodeEditorContextMenuDublicateAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/copy-add"), tr("&Mass dublicate"), this, SLOT(onNodeEditorContextMenuMassDublicateAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/delete"), tr("&Delete"), this, SLOT(onNodeEditorContextMenuDeleteAction()));
  connect(ui->nodeTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNodeEditorContextMenu(const QPoint&)));

  // Create the labels for the status bar and add them to it
  updateStatusBar();
  statusBar()->addPermanentWidget(&filterCountLabel);
  statusBar()->addPermanentWidget(&splineCountLabel);
  statusBar()->addPermanentWidget(&gateCountLabel);
  statusBar()->addPermanentWidget(&prefabCountLabel);
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

  // Hook up our delegation for the node value control
  ui->nodeTreeView->setItemDelegateForColumn(NodeTreeColumns::ValueColumn, new JsonTreeViewItemDelegate(nullptr, &nodeEditor));

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
#ifdef Q_OS_WIN
  return QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation).first().replace("AppData/Local", "AppData/LocalLow/VelociDrone");
#else
  return "";
#endif
}

bool MainWindow::maybeSave()
{
  // if the track wasn't modified and the scene didn't change there is nothing to do here
  if (!nodeEditor.isModified() && (loadedTrack.sceneId == ui->sceneComboBox->currentData().toUInt()))
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
  ui->viewNodeTypeColumn->setChecked(settings.value("viewTypeColumn").toString().toLower() == "true");
  if (!ui->viewNodeTypeColumn->isChecked())
    ui->nodeTreeView->hideColumn(NodeTreeColumns::TypeColumn);
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
  // Parse the count values into their templates and update the labels with it
  nodeCountLabel.setText(nodeCountLabelText.arg(nodeEditor.getNodeCount()));
  prefabCountLabel.setText(prefabCountLabelText.arg(nodeEditor.getPrefabCount()));
  gateCountLabel.setText(gateCountLabelText.arg(nodeEditor.getGateCount()));
  splineCountLabel.setText(splineCountLabelText.arg(nodeEditor.getSplineCount()));
  filterCountLabel.setText(lastSearchResult.count() == 0 ? "" : filterCountLabelText.arg(lastSearchResult.count()));
}

void MainWindow::updateWindowTitle()
{
  // Determinate the database and set the window title
  QString databaseStr = (loadedTrack.assignedDatabase == DatabaseType::Production ? "Production" : ((loadedTrack.assignedDatabase == DatabaseType::Beta) ? "Beta" : "Custom"));
  setWindowTitle(loadedTrack.name + " @ " + databaseStr + " - " + defaultWindowTitle);
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
