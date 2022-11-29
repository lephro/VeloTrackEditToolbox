#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QComboBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QListView>
#include <QMainWindow>
#include <QMessageBox>
#include <QRegularExpressionValidator>
#include <QScrollBar>
#include <QSettings>
#include <QStandardPaths>
#include <QStringList>
#include <QTreeView>
#include <QTreeWidgetItem>

//#include "geodesicdome.h"
#include "nodeeditor.h"
#include "editormanager.h"
#include "nodefilter.h"
#include "opentrackdialog.h"
#include "editorobject.h"
#include "searchfilterlayout.h"
#include "trackarchive.h"
#include "velodb.h"

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

class EditorManager;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  void addFilter(const QModelIndex& index);
  void addFilter(const FilterTypes filterType, const FilterMethods filterMethod, int value, const QString displayValue = "", const QModelIndex &customIndex = QModelIndex());
  void updateStatusBar();
  void updateSearch();
  void updateSearchFilterControls();

private slots:
  void on_aboutPatchLogPushButton_released();
  void on_aboutPushButton_released();
  void on_aboutLicensePushButton_released();
  void on_archiveAddTrackPushButton_released();
  void on_archiveDatabaseSelectionComboBox_currentIndexChanged(const QString &arg1);
  void on_archiveMoveToArchiveCheckBox_stateChanged(int moveToArchiveState);
  void on_archiveRestoreTrackPushButton_released();
  void on_archiveSettingsBrowseToolButton_released();
  void on_archiveSettingsFilepathLineEdit_textChanged(const QString &archiveSettingsFilepath);
  void on_buildTypeComboBox_currentIndexChanged(int index);
  void on_browseUserDbToolButton_released();
  void on_browseSettingsDbToolButton_released();
  void on_deleteTrackPushButton_released();
  void on_filterColorFontPushButton_released();
  void on_filterColorParentPushButton_released();
  void on_filterColorParentFontPushButton_released();
  void on_filterColorPushButton_released();
  void on_mergeTrackPushButton_released();
  void on_mergeTrack1SelectPushButton_released();
  void on_mergeTrack2SelectPushButton_released();
  void on_navListWidget_currentRowChanged(int currentRow);
  void on_nodeEditorTabWidget_currentChanged(int index);
  void on_nodeEditorTabWidget_tabCloseRequested(int index);
  void on_openTrackPushButton_released();
  void on_refreshTrackPushButton_released();
  void on_replacePrefabComboBox_currentIndexChanged(int index);
  void on_saveAsNewCheckbox_clicked(bool checked);
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
  void on_settingsDbLineEdit_textChanged(const QString &settingsDbFilename);
  void on_toolsApplyPushButton_released();
  void on_toolsSubtypeComboBox_currentIndexChanged(int index);
  void on_toolsSubtypeTargetComboBox_currentIndexChanged(int index);
  void on_toolsTypeComboBox_currentIndexChanged(int index);
  void on_transformBDoubleResetPushButton_released();
  void on_transformByComboBox_currentTextChanged(const QString &transformBy);
  void on_transformGDoubleResetPushButton_released();
  void on_transformRDoubleResetPushButton_released();
  void on_transformRotationRValueSpinBox_valueChanged(int value);
  void on_transformRotationGValueSpinBox_valueChanged(int value);
  void on_transformRotationBValueSpinBox_valueChanged(int value);
  void on_transformRotationWValueSpinBox_valueChanged(int value);
  void on_transformRotationXValueSpinBox_valueChanged(int value);
  void on_transformRotationYValueSpinBox_valueChanged(int value);
  void on_transformRotationZValueSpinBox_valueChanged(int value);
  void on_userDbLineEdit_textChanged(const QString &userDbFilename);

  void on_geoGenTestPushButton_released();

  void onSearchFilterChanged();
  void updateDynamicTabControlSize(int index);    


protected:
  void closeEvent(QCloseEvent* e) override;

private:    
  const QString nodeCountLabelText = tr("Nodes: %1");
  const QString prefabCountLabelText = tr("Prefabs: %1");
  const QString gateCountLabelText = tr("Gates: %1");
  const QString splineCountLabelText = tr("Splines: %1");
  const QString filterCountLabelText = tr("Filtered: %1");

  QString defaultWindowTitle;  
  QString defaultProductionUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/user11.db";
  QString defaultBetaUserDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/user11.db";
  QString defaultProductionSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDrone/settings.db";
  QString defaultBetaSettingsDbFilename = "C:/Users/<USER>/AppData/LocalLow/VelociDrone/VelociDroneBeta/settings.db";

  QLabel nodeCountLabel;
  QLabel objectCountLabel;  
  QLabel gateCountLabel;
  QLabel splineCountLabel;
  QLabel filterCountLabel;

  DatabaseType databaseOptionsSelectedDbType = DatabaseType::Production;

  Ui::MainWindow* ui;
  OpenTrackDialog* openTrackDialog;
  SearchFilterLayout* searchFilterLayout;

  VeloDb* selectedDb;
  VeloDb* productionDb;
  VeloDb* betaDb;
  VeloDb* customDb;

  EditorManager* nodeEditorManager;

  TrackArchive* archive;

  TrackData mergeTrack1;
  TrackData mergeTrack2;

  int currentCacheId = INT_MIN;

  void readSettings();

  void updatePrefabComboBoxes();

  QString browseDatabaseFile() const;
  VeloDb* getDatabase();
  VeloDb* getDatabase(const DatabaseType databaseType);
  QColor pickColor(const QString settingsPath, QColor defaultColor);
  void setDatabaseOptionsDatabaseFilenames(const DatabaseType index);
  void setDatabaseOptionsUserDb(const QString& value);
  void setDatabaseOptionsSettingsDb(const QString& value);
  void updateDatabaseOptionsDatabaseStatus();  
  void updateWindowTitle();

  void closeTrack();
  void closeTrack(const int index);
  void loadTrack(const TrackData& track);
  bool maybeSave();
  void saveTrackToDb();
  void saveTrackToFile();  

  QString getDefaultPath();
  bool maybeCreateOrSelectArchive();
  void loadArchive();
  void loadDatabaseForArchive(VeloDb *database);

  bool maybeDontBecauseItsBeta();

  void addFilter(const FilterTypes filterType);
  void toolsReplaceObject();

  void updateSearch(bool filterEnabled);
  void updateSearchFromAngleValues();
  void updateSearchFromQuaternionValues();
  void updateTransformFromQuaternionValues();
  void updateTransformFromAngleValues();
};
#endif // MAINWINDOW_H
