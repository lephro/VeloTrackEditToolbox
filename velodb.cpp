#include "velodb.h"

VeloDb::VeloDb(Database database, const QString* userDbFilename, const QString* settingsDbFilename)
{
  this->database = database;
  this->userDbFilename = *userDbFilename;
  this->settingsDbFilename = *settingsDbFilename;

  prefabs = new QVector<Prefab>();
  scenes = new QVector<Scene>();
  tracks = new QVector<Track>();
}

VeloDb::~VeloDb()
{
  delete prefabs;
  delete scenes;
  delete tracks;
}

bool VeloDb::isValid() const
{
  QFile userDbFile(userDbFilename);
  QFile settingsDbFile(settingsDbFilename);
  return userDbFile.exists() && settingsDbFile.exists();
}

int VeloDb::queryAll()
{
  int resultCode = 0;
  resultCode = queryPrefabs();
  resultCode = queryScenes();
  resultCode = queryTracks();

  return resultCode;
}

int VeloDb::queryPrefabs()
{
  int resultCode = 0;

  prefabs->clear();

  std::string test = settingsDbFilename.toStdString();
  const char* test2 = test.c_str();

  resultCode = sqlite3_open(test2, &db);

  if(resultCode)
    return resultCode;

  if (db == nullptr)
    return ERROR_NO_CONNECTION;

  char* zErrMsg = nullptr;

  resultCode = sqlite3_exec(db, "SELECT*  from trackprefabs", queryPrefabsCallback, prefabs, &zErrMsg);

  sqlite3_close(db);

  if(resultCode != SQLITE_OK) {
    db = nullptr;
    return resultCode;
  }
  std::sort(prefabs->begin(), prefabs->end());

  return 0;
}

int VeloDb::queryScenes()
{
  int resultCode = 0;

  scenes->clear();

  std::string test = settingsDbFilename.toStdString();
  const char* test2 = test.c_str();

  resultCode = sqlite3_open(test2, &db);

  if(resultCode)
    return resultCode;

  if (db == nullptr)
    return ERROR_NO_CONNECTION;

  char* zErrMsg = nullptr;

  resultCode = sqlite3_exec(db, "SELECT*  from sceneries", queryScenesCallback, scenes, &zErrMsg);

  sqlite3_close(db);

  if(resultCode != SQLITE_OK) {
    db = nullptr;
    return resultCode;
  }
  std::sort(scenes->begin(), scenes->end());

  return 0;
}

int VeloDb::queryTracks()
{
  int resultCode = 0;

  tracks->clear();

  resultCode = sqlite3_open(userDbFilename.toStdString().c_str(), &db);

  if(resultCode)
    return resultCode;

  if (db == nullptr)
    return ERROR_NO_CONNECTION;

  char* zErrMsg = nullptr;

  resultCode = sqlite3_exec(db, "SELECT*  from tracks", queryTracksCallback, tracks, &zErrMsg);

  sqlite3_close(db);

  if(resultCode != SQLITE_OK) {
    db = nullptr;
    //fprintf(stderr, "SQL error: %s\n", zErrMsg);
    //sqlite3_free(zErrMsg);

    return resultCode;
  }

  for(QVector<Track>::iterator i = tracks->begin(); i != tracks->end(); ++i)
    i->database = database;

  std::sort(tracks->begin(), tracks->end());

  return 0;
}
int VeloDb::saveChanges()
{
  return 0;
}

QVector<Prefab>* VeloDb::getPrefabs() const
{
  return prefabs;
}

QVector<Scene>* VeloDb::getScenes() const
{
  return scenes;
}

QVector<Track>* VeloDb::getTracks() const
{
  return tracks;
}

int VeloDb::queryPrefabsCallback(void *data, int argc, char **argv, char **azColName)
{
  QVector<Prefab>* prefabs = reinterpret_cast<QVector<Prefab>*>(data);
  Prefab prefab;

  for(int i = 0; i < argc; i++) {
    if (azColName[i] == QString("id")) {
      prefab.id = QString(argv[i]).toUInt();
    }
    else if (azColName[i] == QString("name")) {
      prefab.name = argv[i];
    }
    else if (azColName[i] == QString("type")) {
      prefab.type = argv[i];
    }
    else if (azColName[i] == QString("gate")) {
      prefab.gate = (QString(argv[i]) == "true") || (QString(argv[i]) == "1");
    }
  }

  prefabs->push_back(prefab);

  return 0;
}

int VeloDb::queryScenesCallback(void* data, int argc, char* *argv, char* *azColName)
{
  QVector<Scene>* scenes = reinterpret_cast<QVector<Scene>*>(data);
  Scene scene;

  for(int i = 0; i < argc; i++) {
    if (azColName[i] == QString("id")) {
      scene.id = QString(argv[i]).toUInt();
    }
    else if (azColName[i] == QString("name")) {
      scene.name = argv[i];
    }
    else if (azColName[i] == QString("type")) {
      scene.type = argv[i];
    }
    else if (azColName[i] == QString("title")) {
      scene.title = argv[i];
    }
    else if (azColName[i] == QString("enabled")) {
      scene.enabled = (QString(argv[i]) == "true") ? true : false;
    }
    //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }

  if (scene.enabled && (scene.type != "system"))
  {
    scenes->push_back(scene);
  }

  return 0;
}

int VeloDb::queryTracksCallback(void* data, int argc, char* *argv, char* *azColName)
{
  QVector<Track>* tracks = reinterpret_cast<QVector<Track>* >(data);
  Track track;

  for(int i = 0; i < argc; i++) {
    if (azColName[i] == QString("id")) {
      track.id = QString(argv[i]).toUInt();
    }
    else if (azColName[i] == QString("scene_id")) {
      track.sceneId = QString(argv[i]).toUInt();
    }
    else if (azColName[i] == QString("value")) {
      track.value = argv[i];
    }
    else if (azColName[i] == QString("name")) {
      track.name = QString(QUrl::fromPercentEncoding(argv[i]).replace("+", " "));
    }
    else if (azColName[i] == QString("protectedTrack")) {
      track.protectedTrack = QString(argv[i]).toShort();
    }
    //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }

  if ((track.protectedTrack != 1) && (track.value.at(0) == 123))
  {
    tracks->push_back(track);
  }

  return 0;
}
