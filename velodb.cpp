#include "velodb.h"

VeloDb::VeloDb(Database database, const QString& settingsDbFilename, const QString& userDbFilename)
{
  this->database = database;
  this->settingsDbFilename = settingsDbFilename;
  this->userDbFilename = userDbFilename;

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
  return hasValidUserDb() && hasValidSettingsDb();
}

void VeloDb::queryAll()
{
  queryPrefabs();
  queryScenes();
  queryTracks();
}

void VeloDb::queryPrefabs()
{
  int resultCode = 0;
  char* zErrMsg = nullptr;

  prefabs->clear();

  resultCode = sqlite3_open(settingsDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  resultCode = sqlite3_exec(db, "SELECT*  from trackprefabs", queryPrefabsCallback, prefabs, &zErrMsg);

  sqlite3_close(db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  std::sort(prefabs->begin(), prefabs->end());
}

void VeloDb::queryScenes()
{
  int resultCode = 0;
  char* zErrMsg = nullptr;

  scenes->clear();

  resultCode = sqlite3_open(settingsDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  resultCode = sqlite3_exec(db, "SELECT*  from sceneries", queryScenesCallback, scenes, &zErrMsg);

  sqlite3_close(db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  std::sort(scenes->begin(), scenes->end());
}

void VeloDb::queryTracks()
{
  int resultCode = 0;
  char* zErrMsg = nullptr;

  tracks->clear();

  resultCode = sqlite3_open(userDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  resultCode = sqlite3_exec(db, "SELECT*  from tracks", queryTracksCallback, tracks, &zErrMsg);

  sqlite3_close(db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  for(QVector<Track>::iterator i = tracks->begin(); i != tracks->end(); ++i)
    i->database = database;

  std::sort(tracks->begin(), tracks->end());
}

int VeloDb::saveChanges()
{
  return 0;
}

void VeloDb::setSettingsDbFilename(const QString &filename)
{
  if (settingsDbFilename != filename) {
    this->settingsDbFilename = filename;
    try {
      if (hasValidSettingsDb()) {
        queryPrefabs();
        queryScenes();
      }
    } catch (VeloToolkitException& e) {
      e.Message();
    }
  }
}

void VeloDb::setUserDbFilename(const QString &filename)
{
  if (userDbFilename != filename) {
    this->userDbFilename = filename;
    if (hasValidUserDb()) {
      queryTracks();
    }
  }
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

bool VeloDb::hasValidUserDb() const
{
  QFile userDbFile(userDbFilename);
  // ToDo: Check if tables exists
  return userDbFile.exists();
}

bool VeloDb::hasValidSettingsDb() const
{
  QFile settingsDbFile(settingsDbFilename);
  // Todo: Check if tables exists
  return settingsDbFile.exists();
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
