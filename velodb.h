#ifndef VELODB_H
#define VELODB_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QVariant>

#include "exceptions.h"
#include "sqlite3.h"

enum DatabaseType {
  Production = 0,
  Beta = 1,
  Custom = 2,
  Archive = 3
};
Q_ENUMS(Database)

struct PrefabData
{
  uint id = 0;
  QString name = "";
  QString type = "";
  bool gate = false;

  bool operator < (const PrefabData& prefab)
  {
    return name.compare(prefab.name, Qt::CaseInsensitive) < 0;
  }

  operator QString()
  {
    return name;
  }
};
Q_DECLARE_METATYPE(PrefabData);

struct SceneData
{
  uint id = 0;
  QString name = "";
  QString type = "";
  QString title = "";
  bool enabled = false;

  bool operator < (const SceneData& scene)
  {
    return name.compare(scene.name, Qt::CaseInsensitive) < 0;
  }

  operator QString()
  {
    return title;
  }
};

struct TrackData
{
  uint id = 0;
  uint sceneId = 0;
  QByteArray value = QByteArray();
  QString name = "";
  short protectedTrack = short(true);
  DatabaseType assignedDatabase = DatabaseType::Production;
  uint onlineId = 0;
  uint type = 0;

  bool operator < (const TrackData& track) const { return name.compare(track.name, Qt::CaseInsensitive) < 0; }
  bool operator == (const TrackData& track) const { return ((assignedDatabase == track.assignedDatabase) && (id == track.id)); }

  operator QString() { return name; }
};
Q_DECLARE_METATYPE(TrackData);

class VeloDb
{
public:
  VeloDb(DatabaseType databaseType, const QString& userDbFilename = "", const QString& settingsDbFilename = "");

  void createTrackTable();

  bool isValid() const;

  void queryAll();
  void queryPrefabs();
  void queryScenes();
  void queryTracks();

  void deleteTrack(const TrackData &track);
  uint saveTrack(TrackData &track, const bool createNewEntry = true);
  void setSettingsDbFilename(const QString& filename);
  void setUserDbFilename(const QString& filename, bool refreshData = true);

  DatabaseType getDatabaseType() const;
  QVector<PrefabData> getPrefabs() const;
  QVector<SceneData> getScenes() const;
  QVector<TrackData> getTracks() const;

  static int queryPrefabsCallback(void* data, int argc, char** argv, char** azColName);
  static int queryScenesCallback(void* data, int argc, char** argv, char** azColName);
  static int queryTracksCallback(void* data, int argc, char** argv, char** azColName);

private:
  DatabaseType databaseType;
  QString settingsDbFilename;
  QString userDbFilename;

  sqlite3* db;
  QVector<PrefabData> prefabs;
  QVector<SceneData> scenes;
  QVector<TrackData> tracks;

  bool hasValidSettingsDb() const;
  bool hasValidUserDb() const;

  uint insertTrack(const TrackData &track);
  void updateTrack(const TrackData &track);

  uint executeStatement(const QString sql);
};

#endif // VELODB_H
