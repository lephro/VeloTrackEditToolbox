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

void MainWindow::on_archiveMoveToArchiveCheckBox_stateChanged(int arg1)
{
  // Set the setting
  settingMoveToArchive = bool(arg1);

  // Write into config
  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->setValue("archive/moveToArchive", settingSaveAsNew);
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

void MainWindow::on_saveAsNewCheckbox_stateChanged(int arg1)
{
  if (arg1 && !maybeDontBecauseItsBeta())
    return;

  settingSaveAsNew = bool(arg1);

  QSettings *settings = new QSettings("settings.ini", QSettings::IniFormat);
  settings->setValue("database/saveTrackAsNew", settingSaveAsNew);
}

void MainWindow::on_settingsDbLineEdit_textChanged(const QString &arg1)
{
  setDatabaseOptionsSettingsDb(arg1);
}

void MainWindow::on_userDbLineEdit_textChanged(const QString &arg1)
{
  setDatabaseOptionsUserDb(arg1);
}

void MainWindow::on_viewNodeTypeColumn_stateChanged(int arg1)
{
  // Show or hide the type column, depending on the selected item
  settingViewTypeColumn = bool(arg1);
  if (settingViewTypeColumn) {
    ui->treeView->showColumn(NodeTreeColumns::TypeColumn);
  } else {
    ui->treeView->hideColumn(NodeTreeColumns::TypeColumn);
  }
}
