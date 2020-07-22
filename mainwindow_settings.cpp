#include "mainwindow.h"
#include "ui_mainwindow.h"

QString MainWindow::browseDatabaseFile() const
{
  return QFileDialog::getOpenFileName(nullptr,
                                      tr("Choose Database"),
                                      "C:/Users/%username%/AppData/LocalLow/VelociDrone/",
                                      tr("Database Files (*.db)"));
}

bool MainWindow::maybeDontBecauseItsBeta()
{
  // Open a message box with the beta warning
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

QColor MainWindow::pickColor(const QString settingsPath, QColor defaultColor) {
  QSettings settings("settings.ini", QSettings::IniFormat);
  const QColor currentColor = QColor(settings.value(settingsPath + "R", defaultColor.red()).toInt(),
                                     settings.value(settingsPath + "G", defaultColor.green()).toInt(),
                                     settings.value(settingsPath + "B", defaultColor.blue()).toInt());

  QColor newColor = QColorDialog::getColor(currentColor, this, tr("Choose a color"), QColorDialog::DontUseNativeDialog);
  if (!newColor.isValid() || newColor == currentColor)
    return QColor(QColor::Invalid);

  settings.setValue(settingsPath + "R", newColor.red());
  settings.setValue(settingsPath + "G", newColor.green());
  settings.setValue(settingsPath + "B", newColor.blue());

  return newColor;
}

void MainWindow::setDatabaseOptionsDatabaseFilenames(const DatabaseType databaseType)
{  
  databaseOptionsSelectedDbType = databaseType;

  QSettings settings("settings.ini", QSettings::IniFormat);

  QString userDbFilename = "";
  QString settingsDbFilename = "";
  switch (databaseType) {
  case DatabaseType::Production:
    userDbFilename = settings.value("productionSettingsDbFilename", defaultProductionSettingsDbFilename).toString();
    settingsDbFilename = settings.value("productionUserDbFilename", defaultProductionUserDbFilename).toString();
    break;

  case DatabaseType::Beta:
    userDbFilename = settings.value("betaSettingsDbFilename", defaultBetaSettingsDbFilename).toString();
    settingsDbFilename = settings.value("betaUserDbFilename", defaultBetaUserDbFilename).toString();
    break;

  case DatabaseType::Custom:
    userDbFilename = settings.value("customSettingsDbFilename", "").toString();
    settingsDbFilename = settings.value("customUserDbFilename", "").toString();
    break;
  default:
    return;
  }

  ui->userDbLineEdit->setText(userDbFilename);
  ui->settingsDbLineEdit->setText(settingsDbFilename);
}

void MainWindow::setDatabaseOptionsSettingsDb(const QString& settingsDbFilename)
{
  QSettings settings("settings.ini", QSettings::IniFormat);

  QString currentDbFilename;
  switch (databaseOptionsSelectedDbType) {
  case DatabaseType::Production:
    currentDbFilename = settings.value("productionUserDbFilename", defaultProductionUserDbFilename).toString();
    break;

  case DatabaseType::Beta:
    currentDbFilename = settings.value("betaUserDbFilename", defaultBetaUserDbFilename).toString();
    break;

  case DatabaseType::Custom:
    currentDbFilename = settings.value("customUserDbFilename", "").toString();
    break;
  default:
    return;
  }

  if (settingsDbFilename != currentDbFilename) {
    if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
      setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
    } else {
      settings.setValue("database/productionSettingsDbFilename", settingsDbFilename);

      updateDatabaseOptionsDatabaseStatus();

      closeTrack();
    }
  }
}

void MainWindow::setDatabaseOptionsUserDb(const QString& userDbFilename)
{
  QSettings settings("settings.ini", QSettings::IniFormat);

  QString currentDbFilename = "";
  switch (databaseOptionsSelectedDbType) {
  case DatabaseType::Production:
    currentDbFilename = settings.value("productionSettingsDbFilename", defaultProductionSettingsDbFilename).toString();
    break;

  case DatabaseType::Beta:
    currentDbFilename = settings.value("betaSettingsDbFilename", defaultBetaSettingsDbFilename).toString();
    break;

  case DatabaseType::Custom:
    currentDbFilename = settings.value("customSettingsDbFilename", "").toString();
    break;
  default:
    return;
  }

  if (userDbFilename != currentDbFilename) {
    if ((loadedTrack.assignedDatabase == databaseOptionsSelectedDbType) && (!maybeSave())) {
      setDatabaseOptionsDatabaseFilenames(databaseOptionsSelectedDbType);
    } else {
      currentDbFilename = userDbFilename;
      settings.setValue("database/productionUserDbFilename", userDbFilename);

      updateDatabaseOptionsDatabaseStatus();

      closeTrack();
    }
  }
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


void MainWindow::on_aboutPushButton_released()
{
  // View the about page and view/hide the related buttons
  ui->aboutGroupBox->setTitle(tr("About"));
  ui->aboutLicensePushButton->setVisible(true);
  ui->aboutPatchLogPushButton->setVisible(true);
  ui->aboutPushButton->setVisible(false);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::AboutPage);
}

void MainWindow::on_aboutPatchLogPushButton_released()
{
  // View the patch-log and view/hide the related buttons
  ui->aboutGroupBox->setTitle(tr("Patch Log"));
  ui->aboutLicensePushButton->setVisible(true);
  ui->aboutPatchLogPushButton->setVisible(false);
  ui->aboutPushButton->setVisible(true);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::PatchLogPage);
}

void MainWindow::on_aboutLicensePushButton_released()
{
  // View the license and view/hide the related buttons
  ui->aboutGroupBox->setTitle(tr("License"));
  ui->aboutLicensePushButton->setVisible(false);
  ui->aboutPatchLogPushButton->setVisible(true);
  ui->aboutPushButton->setVisible(true);
  ui->aboutStackedWidget->setCurrentIndex(AboutStackedWidgetPages::LicensePage);
}

void MainWindow::on_archiveMoveToArchiveCheckBox_stateChanged(int moveToArchiveState)
{
  // Write into config
  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.setValue("archive/moveToArchive", bool(moveToArchiveState));
}
void MainWindow::on_archiveSettingsBrowseToolButton_released()
{
  // Open a save file dialog
  QString result = QFileDialog::getSaveFileName(this,
                                                tr("Choose or create an archive"),
                                                QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first(),
                                                tr("Database Files (*.db)"),
                                                nullptr,
                                                QFileDialog::Option::DontConfirmOverwrite);
  if (result == "")
    return;

  // Write the new filepath into the filepath control
  ui->archiveSettingsFilepathLineEdit->setText(result);
}

void MainWindow::on_archiveSettingsFilepathLineEdit_textChanged(const QString& archiveSettingsFilepath)
{
  // Write config and reload
  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.setValue("archive/filename", archiveSettingsFilepath);

  try {
    archive->setFileName(archiveSettingsFilepath);
    loadArchive();
  } catch (VeloToolkitException& e) {
    // Catch weird shit
    e.Message();
  }
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

void MainWindow::on_buildTypeComboBox_currentIndexChanged(int index)
{
  setDatabaseOptionsDatabaseFilenames(DatabaseType(index));
  updateDatabaseOptionsDatabaseStatus();
}

void MainWindow::on_filterColorPushButton_released()
{
  const QColor pick = pickColor("general/filterColor", QColor(254, 203, 137));
  if (!pick.isValid())
    return;

  ui->filterColorPushButton->setStyleSheet("background: " + pick.name());
  nodeEditor.setFilterBackgroundColor(pick);
  updateSearchFilter();
}

void MainWindow::on_filterColorFontPushButton_released()
{
  const QColor pick = pickColor("general/filterFontColor", Qt::black);
  if (!pick.isValid())
    return;

  ui->filterColorFontPushButton->setStyleSheet("background: " + pick.name());
  nodeEditor.setFilterFontColor(pick);
  updateSearchFilter();
}

void MainWindow::on_filterColorParentPushButton_released()
{
  const QColor pick = pickColor("general/filterParentColor", QColor(192, 192, 192));
  if (!pick.isValid())
    return;

  ui->filterColorParentPushButton->setStyleSheet("background: " + pick.name());
  nodeEditor.setFilterContentBackgroundColor(pick);
  updateSearchFilter();
}

void MainWindow::on_filterColorParentFontPushButton_released()
{
  const QColor pick = pickColor("general/filterParentFontColor", Qt::black);
  if (!pick.isValid())
    return;

  ui->filterColorParentFontPushButton->setStyleSheet("background: " + pick.name());
  nodeEditor.setFilterContentFontColor(pick);
  updateSearchFilter();
}

void MainWindow::on_saveAsNewCheckbox_stateChanged(int saveAsNewState)
{
  qDebug() << "SaveAsNew: " << saveAsNewState;
  if (saveAsNewState == 0 && !maybeDontBecauseItsBeta())
    return;

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.setValue("general/saveTrackAsNew", bool(saveAsNewState));
}

void MainWindow::on_settingsDbLineEdit_textChanged(const QString &settingsDbFilename)
{
  setDatabaseOptionsSettingsDb(settingsDbFilename);
}

void MainWindow::on_userDbLineEdit_textChanged(const QString &userDbFilename)
{
  setDatabaseOptionsUserDb(userDbFilename);
}

void MainWindow::on_viewNodeTypeColumn_stateChanged(int viewNodeTypeColumnState)
{
  const bool viewNodeTypeColumn = bool(viewNodeTypeColumnState);
  // Show or hide the type column, depending on the selected item
  if (viewNodeTypeColumn) {
    ui->nodeTreeView->showColumn(NodeTreeColumns::TypeColumn);
  } else {
    ui->nodeTreeView->hideColumn(NodeTreeColumns::TypeColumn);
  }

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.setValue("general/viewTypeColumn", viewNodeTypeColumn);
}
