#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTreeView>

#include "delegates.h"
#include "opentrackdialog.h"
#include "velodb.h"
#include "velotrack.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);

private slots:
  void on_buildTypeComboBox_currentIndexChanged(int index);
  void on_browseUserDbToolButton_released();
  void on_browseSettingsDbToolButton_released();
  void on_openTrackPushButton_released();
  void on_replacePushButton_released();
  void on_replacePrefabComboBox_currentIndexChanged(int index);
  void on_savePushButton_released();
  void on_settingsDbLineEdit_textChanged(const QString &arg1);
  void on_userDbLineEdit_textChanged(const QString &arg1);

  void on_saveAsNewCheckbox_stateChanged(int arg1);
  void on_deleteTrackPushButton_released();
  void on_mergeTrack1SelectPushButton_released();
  void on_mergeTrack2SelectPushButton_released();
  void on_mergeTrackPushButton_released();

  void on_aboutLicensePushButton_released();

protected:
  void closeEvent(QCloseEvent* e) override;

private:  
  const QString defaultProductionUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/user11.db";
  const QString defaultBetaUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/user11.db";
  const QString defaultProductionSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/settings.db";
  const QString defaultBetaSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/settings.db";

  QString defaultWindowTitle;

  bool saveAsNew = true;

  DatabaseType databaseOptionsSelectedDbType = DatabaseType::Production;

  QString productionUserDbFilename = "";
  QString betaUserDbFilename = "";
  QString productionSettingsDbFilename = "";
  QString betaSettingsDbFilename = "";
  QString customUserDbFilename = "";
  QString customSettingsDbFilename = "";

  TrackData loadedTrack;

  Ui::MainWindow* ui;
  VeloTrack* veloTrack;
  VeloDb* selectedDb;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;

  TrackData mergeTrack1;
  TrackData mergeTrack2;

  QTreeWidgetItem lastItem;

  void writeDefaultSettings();
  void readSettings();
  void writeSettings();

  void updateReplacePrefabComboBox();

  QString browseDatabaseFile() const;
  VeloDb *getDatabase();
  VeloDb* getDatabase(DatabaseType databaseType);
  void setDatabaseOptionsDatabaseFilenames(const DatabaseType index);
  void setDatabaseOptionsUserDb(const QString& value);
  void setDatabaseOptionsSettingsDb(const QString& value);
  void updateDatabaseOptionsDatabaseStatus();
  void updateWindowTitle();

  void closeTrack();
  void loadTrack(const TrackData& track);
  bool maybeSave();
  void openTrack();
  void saveTrackToDb();
  void saveTrackToFile();

  void replacePrefab();

  QString getDefaultPath();
};
#endif // MAINWINDOW_H
