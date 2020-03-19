#ifndef VELODB_H
#define VELODB_H

#include <QObject>
#include <QFile>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QVector>

#include "errorcodes.h"
#include "sqlite3.h"

enum Database {
  Production = 0,
  Beta = 1,
  Custom = 2
};

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
};

Q_DECLARE_METATYPE(Prefab);

struct Scene
{
  uint id;
  QString name;
  QString type;
  QString title;
  bool enabled;

  bool operator < (const Scene& scene)
  {
    return name.compare(scene.name, Qt::CaseInsensitive) < 0;
  }
};

struct Track
{
  uint id;
  uint sceneId;
  QByteArray value;
  QString name;
  short protectedTrack;
  Database database;

  bool operator < (const Track& track) const
  {
    return name.compare(track.name, Qt::CaseInsensitive) < 0;
  }
};

Q_DECLARE_METATYPE(Track);

class VeloDb
{
public:
  VeloDb(Database database, const QString* userDbFilename, const QString* settingsDbFilename);
  ~VeloDb();

  bool isValid() const;

  int queryAll();
  int queryPrefabs();
  int queryScenes();
  int queryTracks();

  int saveChanges();

  QVector<Prefab> *getPrefabs() const;
  QVector<Scene>* getScenes() const;
  QVector<Track>* getTracks() const;

private:
  Database database;
  QString userDbFilename;
  QString settingsDbFilename;

  sqlite3* db;
  QVector<Prefab>* prefabs;
  QVector<Scene>* scenes;
  QVector<Track>* tracks;

  static int queryPrefabsCallback(void* data, int argc, char** argv, char** azColName);
  static int queryScenesCallback(void* data, int argc, char** argv, char** azColName);
  static int queryTracksCallback(void* data, int argc, char** argv, char** azColName);
};

#endif // VELODB_H
