#include "track.h"

Track::Track(QObject *parent) :
  QObject(parent)
{

}

//Track::Track(const Track &b) :
//  QObject(b.parent())
//{
//  availablePrefabs = b.availablePrefabs;
//  gates = b.gates;
//  objects = b.objects;
//  trackData = b.trackData;
//  weather = b.weather;
//}

//Track& Track::operator =(const Track &b)
//{
//  if (&b == this)
//    return *this;

//  availablePrefabs = b.availablePrefabs;
//  gates = b.gates;
//  objects = b.objects;
//  trackData = b.trackData;
//  weather = b.weather;

//  return *this;
//}

void Track::addObject(EditorObject* object)
{
  if (!object)
    return;

  object->setParent(this);

  objects.append(object);

  if (object->isGate())
    gates.append(object);      
}

int Track::getAvailablePrefabCount() const
{
  return availablePrefabs.count();
}

int Track::getGateCount() const
{
  return gates.count();
}

int Track::getSplineCount() const
{
  int counter = 0;
  foreach(EditorObject* object, objects) {
    if (object->isSpline())
      counter++;
  }
  return 0;
}

TrackData& Track::getTrackData()
{
  return trackData;
}

WeatherData& Track::Weather()
{
  return weather;
}

QVector<EditorObject*> Track::getObjects() const
{
  return objects;
}

int Track::getObjectCount() const
{
  return objects.count();
}

QVector<PrefabData> Track::getAvailablePrefabs() const
{
    return availablePrefabs;
}

void Track::setAvailablePrefabs(const QVector<PrefabData> &value)
{
    availablePrefabs = value;
}

QVector<EditorObject *> Track::getGates() const
{
  return gates;
}

void Track::setTrackData(const TrackData& value)
{
  trackData = value;
}
