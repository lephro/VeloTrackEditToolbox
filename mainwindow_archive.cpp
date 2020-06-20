#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::loadArchive()
{
  // Get an available database to pull scenes from
  VeloDb* database = getDatabase();

  // Clear the archive tree view
  ui->archiveTreeWidget->clear();

  // Insert the tracks into the archive tree view
  int row = 0;
  foreach(TrackData track, archive->getTracks()) {
    // Create a new tree view item
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();

    // Set the track name as item text
    trackItem->setText(0, track.name);

    // Get the scene of the track if we got a database
    if (database != nullptr) {
      for (QVector<SceneData>::iterator i = database->getScenes().begin(); i != database->getScenes().end(); ++i) {
        if (i->id == track.sceneId) {
          trackItem->setText(TrackTreeColumns::SceneColumn, i->title);
          break;
        }
      }
    } else {
      // if we dont have a database use the scene id as fallback
      trackItem->setText(TrackTreeColumns::SceneColumn, QString("%1").arg(track.sceneId, 0, 10));
    }

    // Insert the track data into the tree view item as user data
    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    // Add the tree view item to the tree view
    ui->archiveTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  // Create and set the labels for the tree view
  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTreeWidget->setHeaderLabels(labels);

  // Resize columns to their content
  ui->archiveTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}

void MainWindow::loadDatabaseForArchive(VeloDb* database)
{
  // Clear the archive track selection tree view
  ui->archiveTrackSelectionTreeWidget->clear();

  if (database == nullptr)
    return;

  // Query all data from the database
  database->queryAll();

  // Insert tracks into the archive track selection tree view
  int row = 0;
  foreach(TrackData track, database->getTracks()) {
    // Create a new tree view item
    QTreeWidgetItem* trackItem = new QTreeWidgetItem();

    // Set the track name as item text
    trackItem->setText(0, track.name);

    // Get the scene of the track
    foreach(SceneData scene, database->getScenes()) {
      if (scene.id == track.sceneId) {
        // Set the scene title in the scene column
        trackItem->setText(TrackTreeColumns::SceneColumn, scene.title);
        break;
      }
    }

    // Insert the track data into the tree view item as user data
    QVariant var;
    var.setValue(track);
    trackItem->setData(TrackTreeColumns::NameColumn, Qt::UserRole, var);

    // Add the tree view item to the tree view
    ui->archiveTrackSelectionTreeWidget->addTopLevelItem(trackItem);
    row++;
  }

  // Create and set the labels for the tree view
  QStringList labels;
  labels << tr("Name") << tr("Scene");
  ui->archiveTrackSelectionTreeWidget->setHeaderLabels(labels);

  // Resize columns to their content
  ui->archiveTrackSelectionTreeWidget->header()->setSectionResizeMode(TrackTreeColumns::NameColumn, QHeaderView::ResizeToContents);
}

bool MainWindow::maybeCreateOrSelectArchive()
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

void MainWindow::on_archiveAddTrackPushButton_released()
{
  // Get selected tracks
  QList<QTreeWidgetItem*> selection = ui->archiveTrackSelectionTreeWidget->selectedItems();

  // Check if a selection was made
  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to transfer into your archive."));
    return;
  }

  // If we move the track we have to delete it from the origin database
  // Gets the origin database
  QString selectedDbStr = ui->archiveDatabaseSelectionComboBox->currentText();
  VeloDb* selectedDbInArchive = nullptr;
  if (settingMoveToArchive) {
    if (selectedDbStr == tr("Production")) {
      selectedDbInArchive = productionDb;
    } else if (selectedDbStr == tr("Beta")) {
      selectedDbInArchive = betaDb;
    } else if (selectedDbStr == tr("Custom")) {
      selectedDbInArchive = customDb;
    } else {
      // ToDo: Errorhandling
      return;
    }
  }

  // Archive all selected tracks
  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();
    // If we move the track we have to delete it from the origin database
    // Deletes the track from the origin database
    if (settingMoveToArchive) {
      try {
        selectedDbInArchive->deleteTrack(track);
      } catch (VeloToolkitException& e) {
        e.Message();
        return;
      }
    }

    try {
      archive->archiveTrack(track);
    } catch (VeloToolkitException& e) {
      e.Message();
      return;
    }
  }

  // Reload the archive and selected database
  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
  on_archiveDatabaseSelectionComboBox_currentIndexChanged(ui->archiveDatabaseSelectionComboBox->currentText());

  // Show a status bar message
  statusBar()->showMessage(tr("Track archived."), 2000);
}

void MainWindow::on_archiveDatabaseSelectionComboBox_currentIndexChanged(const QString &arg1)
{
  // If a user selects another build / database, we clear the database...
  ui->archiveTrackSelectionTreeWidget->clear();

  // .. get the userselection...
  VeloDb* selectedDb = nullptr;
  if (arg1 == tr("Production")) {
    selectedDb = productionDb;
  } else if (arg1 == tr("Beta")) {
    selectedDb = betaDb;
  } else if (arg1 == tr("Custom")) {
    selectedDb = customDb;
  }

  // ... and load the new database
  loadDatabaseForArchive(selectedDb);
}

void MainWindow::on_archiveRestoreTrackPushButton_released()
{
  // Get selected tracks
  QList<QTreeWidgetItem*> selection = ui->archiveTreeWidget->selectedItems();

  // Check if a selection was made
  if (selection.count() == 0) {
    QMessageBox::information(this, tr("No selection!"), tr("Please select one or more tracks, that you want to restore from your archive."));
    return;
  }

  // Restore each selected track
  foreach(QTreeWidgetItem* selectedTrackItem, selection) {
    // Get the trackdata
    TrackData track = selectedTrackItem->data(0, Qt::UserRole).value<TrackData>();

    // Get the selected database in which the track has to be restored
    QString selectedDbStr = ui->archiveDatabaseSelectionComboBox->currentText();
    VeloDb* selectedDbInArchive = nullptr;
    if (selectedDbStr == tr("Production")) {
      selectedDbInArchive = productionDb;
    } else if (selectedDbStr == tr("Beta")) {
      selectedDbInArchive = betaDb;
    } else if (selectedDbStr == tr("Custom")) {
      selectedDbInArchive = customDb;
    } else {
      // ToDo: Errorhandling
      return;
    }

    // Restore the track
    try {
      TrackData restoredTrack = archive->restoreTrack(track);
      restoredTrack.assignedDatabase = selectedDbInArchive->getDatabaseType();
      selectedDbInArchive->saveTrack(restoredTrack);
      //selectedDbInArchive->queryAll();
      loadDatabaseForArchive(selectedDbInArchive);
    } catch (VeloToolkitException& e) {
      e.Message();
      return;
    }
  }

  // Reload the archive and selected database
  loadArchive();
  ui->archiveDatabaseSelectionComboBox->setCurrentIndex(ui->archiveDatabaseSelectionComboBox->currentIndex());
  on_archiveDatabaseSelectionComboBox_currentIndexChanged(ui->archiveDatabaseSelectionComboBox->currentText());

  // Show a status bar message
  statusBar()->showMessage(tr("Track restored."), 2000);
}

void MainWindow::on_trackArchiveSettingsBrowseToolButton_released()
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
  ui->trackArchiveSettingsFilepathLineEdit->setText(result);
}

void MainWindow::on_trackArchiveSettingsFilepathLineEdit_textChanged(const QString &arg1)
{
  try {
    // Write config and reload
    archiveDbFileName = arg1;
    archive->setFileName(arg1);
    loadArchive();
    writeSettings();
  } catch (VeloToolkitException& e) {
    // Catch weird shit
    e.Message();
  }
}
