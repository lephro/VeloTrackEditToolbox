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
    throw SQLErrorException(resultCode, zErrMsg);

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

  resultCode = sqlite3_exec(db, "SELECT* from sceneries", queryScenesCallback, scenes, &zErrMsg);

  sqlite3_close(db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode, zErrMsg);

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

  resultCode = sqlite3_exec(db, "SELECT* from tracks WHERE protected_track=0", queryTracksCallback, tracks, &zErrMsg);

  sqlite3_close(db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode, zErrMsg);

  for(QVector<Track>::iterator i = tracks->begin(); i != tracks->end(); ++i)
    i->database = database;

  std::sort(tracks->begin(), tracks->end());
}

void VeloDb::saveTrack(Track& track, bool createNewEntry)
{
  if (createNewEntry)
    insertTrack(track);
  else
    updateTrack(track);
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

void VeloDb::insertTrack(Track &track)
{
  if (track.id == 0)
    throw InvalidTrackException();

  if (track.protectedTrack)
    throw ProtectedTrackException();

  if (track.database != database)
    throw TrackDoesNotBelongToDatabaseException();

  int resultCode = 0;
  char* zErrMsg = nullptr;

  resultCode = sqlite3_open(userDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  QString sql = "INSERT INTO tracks (scene_Id, name, value) " \
                "VALUES (%1, '%2', '%3');";

  sql = sql.arg(track.sceneId, 0, 10);
  sql = sql.arg(track.name + "-new");
  sql = sql.arg(track.value.toStdString().c_str());

  resultCode = sqlite3_exec(db, sql.toStdString().c_str(), nullptr, nullptr, &zErrMsg);
  sqlite3_close(db);

  if (resultCode)
    throw SQLErrorException(resultCode, zErrMsg);
}

void VeloDb::updateTrack(Track &track)
{
  if (track.id == 0)
    throw InvalidTrackException();

  if (track.protectedTrack)
    throw ProtectedTrackException();

  if (track.database != database)
    throw TrackDoesNotBelongToDatabaseException();

  int resultCode = 0;
  char* zErrMsg = nullptr;

  resultCode = sqlite3_open(userDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  QString sql = "UPDATE tracks " \
                "SET id=%1, scene_id=%2, name='%3' value='%4');";

  sql = sql.arg(track.id, 0, 10);
  sql = sql.arg(track.sceneId, 0, 10);
  sql = sql.arg(track.name);
  sql = sql.arg(track.value.toStdString().c_str());

  resultCode = sqlite3_exec(db, sql.toStdString().c_str(), nullptr, nullptr, &zErrMsg);
  sqlite3_close(db);

  if (resultCode)
    throw SQLErrorException(resultCode, zErrMsg);
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
    else if (azColName[i] == QString("protected_track")) {
      track.protectedTrack = short(*argv[i]);
    }
  }

  if ((track.protectedTrack != 1) && (track.value.at(0) == 123))
  {
    tracks->push_back(track);
  }

  return 0;
}
