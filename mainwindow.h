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
#include <QSettings>
#include <QStringList>
#include <QTreeView>
#include <QTreeWidgetItem>

#include "delegates.h"
#include "opentrackdialog.h"
#include "velodataparser.h"
#include "velodb.h"

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

protected:
  void closeEvent(QCloseEvent* e) override;

private:  
  const QString defaultProductionUserDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDrone/user11.db";
  const QString defaultBetaUserDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDroneBeta/user11.db";
  const QString defaultProductionSettingsDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDrone/settings.db";
  const QString defaultBetaSettingsDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDroneBeta/settings.db";

  QString defaultWindowTitle;

  bool saveAsNew = true;
  bool trackModified = false;

  Database databaseOptionsSelectedDb = Database::Production;

  QString productionUserDbFilename = "";
  QString betaUserDbFilename = "";
  QString productionSettingsDbFilename = "";
  QString betaSettingsDbFilename = "";
  QString customUserDbFilename = "";
  QString customSettingsDbFilename = "";

  Track loadedTrack;

  Ui::MainWindow* ui;
  VeloDataParser* dataParser;
  VeloDb* selectedDb;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;

  QTreeWidgetItem lastItem;

  void writeDefaultSettings();
  void readSettings();
  void writeSettings();

  void updateReplacePrefabComboBox();

  QString browseDatabaseFile() const;
  VeloDb *getDatabase();
  VeloDb* getDatabase(Database database);
  void setDatabaseOptionsDatabaseFilenames(const Database index);
  void setDatabaseOptionsUserDb(const QString& value);
  void setDatabaseOptionsSettingsDb(const QString& value);
  void updateDatabaseOptionsDatabaseStatus();

  void closeTrack();
  void loadTrack(const Track& track);
  bool maybeSave();
  void openTrack();
  void saveTrackToDb();
  void saveTrackToFile();

  void replacePrefab();

};
#endif // MAINWINDOW_H
