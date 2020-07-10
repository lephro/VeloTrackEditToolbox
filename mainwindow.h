#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QMenu>
#include <QRegularExpressionValidator>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTreeView>
#include <QTreeWidgetItem>

#include "delegates.h"
#include "nodefilter.h"
#include "geodesicdome.h"
#include "opentrackdialog.h"
#include "prefabitem.h"
#include "searchfilterlayout.h"
#include "trackarchive.h"
#include "velodb.h"
#include "nodeeditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum NavRows {
  NodeEditorRow = 0,
  MergeTracksRow = 1,
  ArchiveRow = 2,
  OptionsRow = 3,
  AboutRow = 4
};

enum AboutStackedWidgetPages {
  AboutPage = 0,
  PatchLogPage = 1,
  LicensePage = 2,
};

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);

private slots:
  void on_aboutPatchLogPushButton_released();
  void on_aboutPushButton_released();
  void on_aboutLicensePushButton_released();
  void on_archiveAddTrackPushButton_released();
  void on_archiveDatabaseSelectionComboBox_currentIndexChanged(const QString &arg1);
  void on_archiveMoveToArchiveCheckBox_stateChanged(int arg1);
  void on_archiveRestoreTrackPushButton_released();
  void on_buildTypeComboBox_currentIndexChanged(int index);
  void on_browseUserDbToolButton_released();
  void on_browseSettingsDbToolButton_released();
  void on_deleteTrackPushButton_released();
  void on_openTrackPushButton_released();
  void on_mergeTrackPushButton_released();
  void on_mergeTrack1SelectPushButton_released();
  void on_mergeTrack2SelectPushButton_released();
  void on_navListWidget_currentRowChanged(int currentRow);
  void on_replacePrefabComboBox_currentIndexChanged(int index);
  void on_saveAsNewCheckbox_stateChanged(int arg1);
  void on_savePushButton_released();
  void on_searchAddFilterPushButton_released();
  void on_searchClearFilterPushButton_released();
  void on_searchFilterGroupBox_toggled(bool newState);
  void on_searchOptionsShowOnlyFilteredCheckBox_stateChanged(int checked);
  void on_searchSubtypeComboBox_currentIndexChanged(const QString &subtypeDesc);
  void on_searchTypeComboBox_currentIndexChanged(const QString &filterDesc);
  void on_searchTypeRotationWValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationXValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationYValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationZValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationRValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationGValueSpinBox_valueChanged(int value);
  void on_searchTypeRotationBValueSpinBox_valueChanged(int value);
  void on_settingsDbLineEdit_textChanged(const QString &arg1);  
  void on_trackArchiveSettingsBrowseToolButton_released();
  void on_trackArchiveSettingsFilepathLineEdit_textChanged(const QString &arg1);
  void on_userDbLineEdit_textChanged(const QString &arg1);
  void on_viewNodeTypeColumn_stateChanged(int arg1);
  void on_geoGenTestPushButton_released();

  void onNodeEditorContextMenu(const QPoint &point);
  void onNodeEditorContextMenuDeleteAction();
  void onNodeEditorContextMenuDublicateAction();
  void onNodeEditorContextMenuMassDublicateAction();

  void onSearchFilterChanged();
  void updateDynamicTabControlSize(int index);    



  void on_toolsApplyPushButton_released();

  void on_toolsTypeComboBox_currentIndexChanged(int index);

  void on_transformByComboBox_currentTextChanged(const QString &transformBy);

  void on_toolsSubtypeComboBox_currentIndexChanged(int index);

  void on_transformRDoubleResetPushButton_released();

  void on_transformGDoubleResetPushButton_released();

  void on_transformBDoubleResetPushButton_released();

  void on_transformRotationRValueSpinBox_valueChanged(int value);

  void on_transformRotationGValueSpinBox_valueChanged(int value);

  void on_transformRotationBValueSpinBox_valueChanged(int value);

  void on_transformRotationWValueSpinBox_valueChanged(int value);

  void on_transformRotationXValueSpinBox_valueChanged(int value);

  void on_transformRotationYValueSpinBox_valueChanged(int value);

  void on_transformRotationZValueSpinBox_valueChanged(int value);

  void onNodeEditorContextMenuAddToFilterAction();

protected:
  void closeEvent(QCloseEvent* e) override;

private:  
  const QString defaultProductionUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/user11.db";
  const QString defaultBetaUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/user11.db";
  const QString defaultProductionSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/settings.db";
  const QString defaultBetaSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/settings.db";
  const QString nodeCountLabelText = tr("Nodes: %1");
  const QString prefabCountLabelText = tr("Prefabs: %1");
  const QString gateCountLabelText = tr("Gates: %1");
  const QString splineCountLabelText = tr("Splines: %1");
  const QString filterCountLabelText = tr("Filtered: %1");

  QString defaultWindowTitle;

  QLabel nodeCountLabel;
  QLabel prefabCountLabel;  
  QLabel gateCountLabel;
  QLabel splineCountLabel;
  QLabel filterCountLabel;

  bool settingMoveToArchive = false;
  bool settingSaveAsNew = true;
  bool settingViewTypeColumn = false;

  DatabaseType databaseOptionsSelectedDbType = DatabaseType::Production;

  QString productionUserDbFilename = "";
  QString betaUserDbFilename = "";
  QString productionSettingsDbFilename = "";
  QString betaSettingsDbFilename = "";
  QString customUserDbFilename = "";
  QString customSettingsDbFilename = "";
  QString archiveDbFileName = "";

  TrackData loadedTrack;

  Ui::MainWindow* ui;
  SearchFilterLayout* searchFilterLayout;
  QVector<PrefabItem*> lastSearchResult;
  QMenu nodeEditorContextMenu;

  VeloDb* selectedDb;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;
  NodeEditor nodeEditor;

  TrackArchive* archive;

  TrackData mergeTrack1;
  TrackData mergeTrack2;

  bool nodeEditStarted = false;
  QVector<bool> nodeEditTreeExpansionStates;
  float nodeEditLastScrollbarPos;

  void readSettings();

  void updatePrefabComboBoxes();

  QString browseDatabaseFile() const;
  VeloDb* getDatabase();
  VeloDb* getDatabase(DatabaseType databaseType);
  void setDatabaseOptionsDatabaseFilenames(const DatabaseType index);
  void setDatabaseOptionsUserDb(const QString& value);
  void setDatabaseOptionsSettingsDb(const QString& value);
  void updateDatabaseOptionsDatabaseStatus();
  void updateStatusBar();
  void updateWindowTitle();

  void closeTrack();
  void loadTrack(const TrackData& track);
  bool maybeSave();
  void saveTrackToDb();
  void saveTrackToFile();  

  QString getDefaultPath();
  bool maybeCreateOrSelectArchive();
  void loadArchive();
  void loadDatabaseForArchive(VeloDb *database);

  bool maybeDontBecauseItsBeta();

  void updateSearchFilter();
  void updateSearchFilter(bool filterEnabled);
  void updateSearchFilterMarks();
  void resetFilterMarks(bool setNodeEdit = true);
  void updateSearchFromAngleValues();
  void updateSearchFromQuaternionValues();
  void addFilter(const FilterTypes filterType);
  void toolsReplaceObject();
  void updateTransformFromQuaternionValues();
  void updateTransformFromAngleValues();
  void beginNodeEdit();
  void endNodeEdit();
};
#endif // MAINWINDOW_H
