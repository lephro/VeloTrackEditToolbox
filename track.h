#ifndef TRACK_H
#define TRACK_H

#include <QObject>

#include "editorobject.h"
#include "velodb.h"

struct WeatherData
{
  double cloud;
  double dambient;
  double dlight;
  double dshadow;
  bool fog;
  double hour;
  double latitude;
  double longitude;
  char month;
  double nambient;
  double nlight;
  double nshadow;
  bool time;
  double utc;
};

class EditorObject;

class Track : public QObject
{
  Q_OBJECT

public:
  explicit Track(QObject* parent = nullptr);
//  explicit Track(const Track& b);

//  Track& operator = (const Track& b);

  void                    addObject(EditorObject* object);

  QVector<PrefabData>     getAvailablePrefabs() const;
  int                     getGateCount() const;
  QVector<EditorObject*>  getObjects() const;
  int                     getObjectCount() const;
  int                     getAvailablePrefabCount() const;
  int                     getSplineCount() const;
  TrackData&              getTrackData();
  void                    setTrackData(const TrackData& value);
  WeatherData&            Weather();

  void                    setAvailablePrefabs(const QVector<PrefabData>& value);

  QVector<EditorObject*>  getGates() const;

private:
  QVector<PrefabData>     availablePrefabs;
  QVector<EditorObject*>  gates;
  QVector<EditorObject*>  objects;
  TrackData               trackData;
  WeatherData             weather;
};

#endif // TRACK_H
