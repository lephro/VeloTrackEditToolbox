#ifndef VELODB_H
#define VELODB_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>

#include "exceptions.h"
#include "sqlite3.h"


enum Database {
  Production = 0,
  Beta = 1,
  Custom = 2
};
Q_ENUMS(Database)

struct Prefab
{
  uint id = 0;
  QString name = "";
  QString type = "";
  bool gate = false;

  bool operator < (const Prefab& prefab)
  {
    return name.compare(prefab.name, Qt::CaseInsensitive) < 0;
  }

  operator QString()
  {
    return name;
  }
};
Q_DECLARE_METATYPE(Prefab);

struct Scene
{
  uint id = 0;
  QString name = "";
  QString type = "";
  QString title = "";
  bool enabled = false;

  bool operator < (const Scene& scene)
  {
    return name.compare(scene.name, Qt::CaseInsensitive) < 0;
  }

  operator QString()
  {
    return title;
  }
};

struct Track
{
  uint id = 0;
  uint sceneId = 0;
  QByteArray value = QByteArray();
  QString name = "";
  short protectedTrack = short(true);
  Database database = Database::Production;

  bool operator < (const Track& track) const
  {
    return name.compare(track.name, Qt::CaseInsensitive) < 0;
  }

  operator QString()
  {
    return name;
  }
};
Q_DECLARE_METATYPE(Track);

class VeloDb
{
public:
  VeloDb(Database database, const QString& userDbFilename = "", const QString& settingsDbFilename = "");
  ~VeloDb();

  bool isValid() const;

  void queryAll();
  void queryPrefabs();
  void queryScenes();
  void queryTracks();

  void saveTrack(Track& track, bool createNewEntry = true);
  void setSettingsDbFilename(const QString& filename);
  void setUserDbFilename(const QString& filename);

  QVector<Prefab> *getPrefabs() const;
  QVector<Scene>* getScenes() const;
  QVector<Track>* getTracks() const;

private:
  Database database;
  QString settingsDbFilename;
  QString userDbFilename;

  sqlite3* db;
  QVector<Prefab>* prefabs;
  QVector<Scene>* scenes;
  QVector<Track>* tracks;

  bool hasValidSettingsDb() const;
  bool hasValidUserDb() const;

  void insertTrack(Track& track);
  void updateTrack(Track& track);

  static int queryPrefabsCallback(void* data, int argc, char** argv, char** azColName);
  static int queryScenesCallback(void* data, int argc, char** argv, char** azColName);
  static int queryTracksCallback(void* data, int argc, char** argv, char** azColName);
};

#endif // VELODB_H
