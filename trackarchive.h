#ifndef TRACKARCHIVE_H
#define TRACKARCHIVE_H

#include <QObject>
#include <QSettings>

#include "exceptions.h"
#include "sqlite3.h"
#include "velodb.h"

class TrackArchive : public QObject
{
  Q_OBJECT
public:
  explicit TrackArchive(QObject *parent = nullptr);

  void archiveTrack(TrackData& trackData);
  TrackData restoreTrack(TrackData& trackData);

  QString getFileName() const;
  void setFileName(const QString &value);

  QVector<TrackData> getTracks() const;
  void setTracks(const QVector<TrackData> &value);

private:
  QString fileName;
  VeloDb* archiveDb;
  QVector<TrackData> tracks;

  void queryTracks();
};

#endif // TRACKARCHIVE_H
