#ifndef VELOJSONPARSER_H
#define VELOJSONPARSER_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QTreeWidgetItem>

#include "exceptions.h"
#include "nodeeditor.h"
#include "track.h"
#include "velodb.h"

class EditorObject;
class Track;

class InvalidDataException : public VeloToolkitException
{
public:
  InvalidDataException(const QString errormessage = "") :
    VeloToolkitException(QObject::tr("Invalid Track data. ") + errormessage) {}
};

class TrackWithoutNodesException : public VeloToolkitException
{
public:
  TrackWithoutNodesException() :
    VeloToolkitException("The track does not contain any nodes!") {}
};

class VeloDataParser : public QObject
{
  Q_OBJECT

public:
  explicit VeloDataParser(QObject* parent = nullptr);

  QByteArray* exportToJson();

  uint getGateCount() const;
  uint getNodeCount() const;
  uint getPrefabCount() const;
  uint getSplineCount() const;

  void mergeJson(const QByteArray& jsonData, const bool addBarriers, const bool addGates);

  Track& parseTrack(const QVector<PrefabData>& prefabs, const TrackData& trackData);

private:
  uint nodeCount = 0;
  uint readGateCount = 0;
  uint readPrefabCount = 0;
  uint readSplineCount = 0;

  QJsonArray* exportToDataArray(const QStandardItem *treeItem);
  QJsonObject* exportToObject(const QStandardItem *treeItem);

  void importJsonArray(QStandardItem *parentItem,
                       const QJsonArray &dataArray,
                       const uint gateOffset = 0,
                       const bool skipStartgrid = false);
  void importJsonObject(QStandardItem *parentItem,
                        const QJsonObject &dataObject,
                        const uint gateOffset = 0,
                        const bool skipStartgrid = false);

  uint getGatesInModelCount() const;

  static QString getJsonValueTypeAsString(const QJsonValue::Type type);
  EditorObject* parsePrefab(const QVector<PrefabData>& prefabs, const QJsonObject &dataObject);
};
#endif // VELOJSONPARSER_H
