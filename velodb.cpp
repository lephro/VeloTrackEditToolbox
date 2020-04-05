#include "velodb.h"

VeloDb::VeloDb(DatabaseType databaseType, const QString& settingsDbFilename, const QString& userDbFilename)
{
  this->databaseType = databaseType;
  this->settingsDbFilename = settingsDbFilename;
  this->userDbFilename = userDbFilename;

  prefabs = new QVector<PrefabData>();
  scenes = new QVector<SceneData>();
  tracks = new QVector<TrackData>();
}

VeloDb::~VeloDb()
{
  delete prefabs;
  delete scenes;
  delete tracks;
}

void VeloDb::createTrackTable()
{
  QString sql = "CREATE TABLE IF NOT EXISTS [tracks] (" \
                "[id] INTEGER NOT NULL PRIMARY KEY," \
                "[scene_id] INTEGER NOT NULL," \
                "[name] VARCHAR," \
                "[value] VARCHAR," \
                "[protected_track] TINYINT(1) NOT NULL DEFAULT 0," \
                "[online_id] INT DEFAULT 0," \
                "[rating] INT DEFAULT 0," \
                "[favourite] INT DEFAULT 0," \
                "[date] VARCHAR DEFAULT '2020-04-30 00:00:00'," \
                "[type] INT DEFAULT 0)";

  executeStatement(sql);
}

void VeloDb::deleteTrack(TrackData &track)
{
  if (track.id == 0)
    throw InvalidTrackException();

  if (track.protectedTrack == 1)
    throw ProtectedTrackException();

  if (track.assignedDatabase != databaseType)
    throw TrackDoesNotBelongToDatabaseException();

  QString sql = "DELETE FROM tracks WHERE id=%1;";
  sql = sql.arg(track.id, 0, 10);

  executeStatement(sql);
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
  if (settingsDbFilename == "")
    throw NoDatabasesFileNameException();

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
  if (settingsDbFilename == "")
    throw NoDatabasesFileNameException();

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

  for(QVector<TrackData>::iterator i = tracks->begin(); i != tracks->end(); ++i)
    i->assignedDatabase = databaseType;

  std::sort(tracks->begin(), tracks->end());
}

uint VeloDb::saveTrack(TrackData &track, const bool createNewEntry)
{
  track.onlineId = 0;
  track.protectedTrack = 0;

  if (createNewEntry)
    return insertTrack(track);

  updateTrack(track);

  return track.id;
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

void VeloDb::setUserDbFilename(const QString &filename, bool refreshData)
{
  if (userDbFilename != filename) {
    this->userDbFilename = filename;
    if (refreshData && hasValidUserDb()) {
      queryTracks();
    }
  }
}

DatabaseType VeloDb::getDatabaseType() const
{
  return databaseType;
}

QVector<PrefabData>* VeloDb::getPrefabs() const
{
  return prefabs;
}

QVector<SceneData>* VeloDb::getScenes() const
{
  return scenes;
}

QVector<TrackData>* VeloDb::getTracks() const
{
  return tracks;
}

int VeloDb::queryPrefabsCallback(void *data, int argc, char **argv, char **azColName)
{
  QVector<PrefabData>* prefabs = reinterpret_cast<QVector<PrefabData>*>(data);
  PrefabData prefab;

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
  QVector<SceneData>* scenes = reinterpret_cast<QVector<SceneData>*>(data);
  SceneData scene;

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
  QVector<TrackData>* tracks = reinterpret_cast<QVector<TrackData>* >(data);
  TrackData track;

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
      track.protectedTrack = QString(argv[i]).toShort();
    }
    else if (azColName[i] == QString("online_id")) {
      track.onlineId = QString(argv[i]).toUInt();
    }
  }

  if ((track.protectedTrack != 1) && (track.value.at(0) == 123))
  {
    tracks->push_back(track);
  }

  return 0;
}

bool VeloDb::hasValidUserDb() const
{
  QFile userDbFile(userDbFilename);
  // ToDo: Check if tables exists
  return userDbFile.exists();
}

uint VeloDb::insertTrack(const TrackData &track)
{
  if (track.protectedTrack == 1)
    throw ProtectedTrackException();

  if (track.assignedDatabase != databaseType)
    throw TrackDoesNotBelongToDatabaseException();

  QString sql = "INSERT INTO tracks (scene_Id, name, value) VALUES" \
                "(%1, '%2', '%3')";
  sql = sql.arg(track.sceneId, 0, 10);
  sql = sql.arg(track.name);
  sql = sql.arg(track.value.toStdString().c_str());

  return executeStatement(sql);
}

void VeloDb::updateTrack(const TrackData &track)
{
  if (track.id == 0)
    throw InvalidTrackException();

  if (track.protectedTrack == 1)
    throw ProtectedTrackException();

  if (track.assignedDatabase != databaseType)
    throw TrackDoesNotBelongToDatabaseException();

  QString sql = "UPDATE tracks " \
                "SET scene_id=%2, name='%3', value='%4' online_id='%5' protected_track='%6' " \
                "WHERE id=%1;";

  sql = sql.arg(track.id, 0, 10);
  sql = sql.arg(track.sceneId, 0, 10);
  sql = sql.arg(track.name);
  sql = sql.arg(track.value.toStdString().c_str());
  sql = sql.arg(track.onlineId, 0, 10);
  sql = sql.arg(track.protectedTrack, 0, 10);

  executeStatement(sql);
}

uint VeloDb::executeStatement(QString sql)
{
  if (userDbFilename == "")
    throw NoDatabasesFileNameException();

  int resultCode = 0;
  char* zErrMsg = nullptr;

  resultCode = sqlite3_open(userDbFilename.toStdString().c_str(), &db);

  if (resultCode != SQLITE_OK)
    throw SQLErrorException(resultCode);

  resultCode = sqlite3_exec(db, sql.toStdString().c_str(), nullptr, nullptr, &zErrMsg);

  uint rowId = uint(sqlite3_last_insert_rowid(db));

  sqlite3_close(db);

  if (resultCode)
    throw SQLErrorException(resultCode, zErrMsg);

  return rowId;
}

bool VeloDb::hasValidSettingsDb() const
{
  QFile settingsDbFile(settingsDbFilename);
  // Todo: Check if tables exists
  return settingsDbFile.exists();
}
