#include "trackarchive.h"

TrackArchive::TrackArchive(QObject *parent)
{
  Q_UNUSED(parent)

  archiveDb = new VeloDb(DatabaseType::Archive);
}

void TrackArchive::archiveTrack(TrackData &trackData)
{
  trackData.assignedDatabase = archiveDb->getDatabaseType();
  archiveDb->saveTrack(trackData, true);

  tracks.append(trackData);
}

TrackData TrackArchive::restoreTrack(TrackData &trackData)
{
  for(int i = 0; i < tracks.count(); ++i) {
    TrackData restoredTrack = tracks.at(i);
    if (restoredTrack == trackData) {
      archiveDb->deleteTrack(restoredTrack);
      tracks.removeAt(i);

      return restoredTrack;
    }
  }

  return TrackData();
}

QString TrackArchive::getFileName() const
{
  return fileName;
}

void TrackArchive::setFileName(const QString &value)
{
  fileName = value;

  archiveDb->setUserDbFilename(value, false);

  try {
    archiveDb->createTrackTable();
  } catch (NoDatabasesFileNameException& e) {
    Q_UNUSED(e)
  }

  queryTracks();
}

QVector<TrackData> TrackArchive::getTracks() const
{
  return tracks;
}

void TrackArchive::setTracks(const QVector<TrackData> &value)
{
  tracks = value;
}

void TrackArchive::queryTracks()
{
  tracks.clear();

  try {
    archiveDb->queryTracks();
  } catch (NoDatabasesFileNameException& e) {
    Q_UNUSED(e)
  }

  tracks = archiveDb->getTracks();
}



