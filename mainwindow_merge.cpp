#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_mergeTrack1SelectPushButton_released()
{
  try {
    // Execute the open track dialog
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      // Get the selection
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      // Check if a track was selected
      if (selectedTrack.id > 0) {
        // Put the selected track into the merge selection
        mergeTrack1 = selectedTrack;
        ui->mergeTrack1Name->setText(selectedTrack.name);
        // Set the track name of the new track
        if (ui->mergeTrackNewTrackNameLineEdit->text() == "")
          ui->mergeTrackNewTrackNameLineEdit->setText(selectedTrack.name + "-merged");
      }
    }
  } catch (VeloToolkitException& e) {
    e.Message();
  }
}

void MainWindow::on_mergeTrack2SelectPushButton_released()
{
  try {
    // Execute the open track dialog
    OpenTrackDialog openTrackDialog(this, productionDb, betaDb, customDb);
    if (openTrackDialog.exec()) {
      // Get the selection
      TrackData selectedTrack = openTrackDialog.getSelectedTrack();
      // Check if a track was selected
      if (selectedTrack.id > 0) {
        // Put the selected track into the merge selection
        mergeTrack2 = selectedTrack;
        ui->mergeTrack2Name->setText(selectedTrack.name);
      }
    }
  } catch (VeloToolkitException& e) {
    e.Message();
  }
}

void MainWindow::on_mergeTrackPushButton_released()
{
  // Check if two tracks are selected
  if ((mergeTrack1.id == 0) || (mergeTrack2.id == 0)) {
    QMessageBox::critical(this, tr("Merge error"), tr("Not enough tracks selected\nPlease select two tracks, that you want to merged."));
    return;
  }

  // Check if both are different :)
  if ((mergeTrack1.assignedDatabase == mergeTrack2.assignedDatabase) && (mergeTrack1.id == mergeTrack2.id)) {
    QMessageBox::critical(this, tr("Merge error"), tr("You are trying to merge a track into itself. Thats a nope!"));
    return;
  }

  // Create a new velo track manager
  VeloTrack newVeloTrack;
  try {
    // Set the prefabs
    newVeloTrack.setPrefabs(getDatabase(mergeTrack1.assignedDatabase)->getPrefabs());

    // Merge the json data
    newVeloTrack.mergeJsonData(&mergeTrack1.value, ui->mergeTrack1BarriersCheckBox->isChecked(), ui->mergeTrack1GatesCheckBox->isChecked());
    newVeloTrack.mergeJsonData(&mergeTrack2.value, ui->mergeTrack2BarriersCheckBox->isChecked(), ui->mergeTrack2GatesCheckBox->isChecked());
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  // Create a new track
  TrackData newTrack;
  newTrack.id = 0;
  newTrack.name = ui->mergeTrackNewTrackNameLineEdit->text();
  newTrack.sceneId = mergeTrack1.sceneId;
  newTrack.assignedDatabase = mergeTrack1.assignedDatabase;
  newTrack.protectedTrack = 0;
  newTrack.value = *newVeloTrack.exportAsJsonData();

  // Save the track into the database
  try {
    getDatabase(newTrack.assignedDatabase)->saveTrack(newTrack);
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  // Reset the selection
  mergeTrack1 = TrackData();
  mergeTrack2 = TrackData();
  ui->mergeTrack1Name->setText(tr("None"));
  ui->mergeTrack2Name->setText(tr("None"));
  ui->mergeTrackNewTrackNameLineEdit->setText("");

  // Message the user
  QMessageBox::information(this, "Merge succeeded!", "The tracks got successfully merged.\nThe new track \"" + newTrack.name + "\" has been written to the database.");

  // Show a status bar message
  statusBar()->showMessage(tr("Track deleted."), 2000);
}
