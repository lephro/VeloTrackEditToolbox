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

protected:
  void closeEvent(QCloseEvent* e) override;

private:
  const QString defaultProductionUserDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDrone/user11.db";
  const QString defaultBetaUserDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDroneBeta/user11.db";
  const QString defaultProductionSettingsDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDrone/settings.db";
  const QString defaultBetaSettingsDbFilename = "C:/Users/lephro/AppData/LocalLow/VelociDrone/VelociDroneBeta/settings.db";

  bool trackModified = false;

  Database currentDbSelection = Database::Production;
  QString productionUserDbFilename = "";
  QString betaUserDbFilename = "";
  QString productionSettingsDbFilename = "";
  QString betaSettingsDbFilename = "";
  QString customUserDbFilename = "";
  QString customSettingsDbFilename = "";

  Track currentTrack;

  Ui::MainWindow* ui;
  VeloDataParser* dataParser;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;

  QTreeWidgetItem lastItem;

  void writeDefaultSettings();
  void readSettings();
  void writeSettings();

  void refreshDbInfo();
  void refreshPrefabsInUse();

  void switchCurrentDbSelection(const Database index);
  void setUserDb(const QString value);
  void setSettingsDb(const QString value);
  void updateDatabaseStatus();

  QString browseDatabaseFile() const;
  bool maybeSave();
  bool saveTrackToDb();
  bool saveTrackToFile();

  void replacePrefab();
  void openTrack();
};
#endif // MAINWINDOW_H
