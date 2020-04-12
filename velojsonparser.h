#ifndef VELOJSONPARSER_H
#define VELOJSONPARSER_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QObject>
#include <QStandardItem>
#include <QString>
#include <QTreeWidgetItem>

#include "exceptions.h"
#include "velodb.h"
#include "velotrack.h"

class VeloDataParser : public QObject
{
  Q_OBJECT
public:
  explicit VeloDataParser(QObject* parent = nullptr, QVector<PrefabData>* prefabs = new QVector<PrefabData>(), QStandardItemModel* model = new QStandardItemModel());

  QByteArray* exportToJson();

  uint getGateCount() const;
  uint getNodeCount() const;
  uint getPrefabCount() const;
  uint getSplineCount() const;

  void importJson(const QByteArray* jsonData);

  void mergeJson(const QByteArray* jsonData, const bool addBarriers, const bool addGates);

private:
  QJsonDocument* doc = nullptr;
  QVector<PrefabData>* prefabs;

  uint gateCount = 0;
  uint nodeCount = 0;
  uint prefabCount = 0;
  uint splineCount = 0;

  QStandardItemModel* model;

  QJsonArray* exportToDataArray(const QStandardItem *treeItem);
  QJsonObject* exportToObject(const QStandardItem *treeItem);

  uint getGatesInModelCount() const;

  PrefabData getPrefab(const uint id) const;

  void importJsonArray(QStandardItem *parentItem,
                       const QJsonArray &dataArray,
                       const uint gateOffset = 0,
                       const bool skipStartgrid = false);
  void importJsonObject(QStandardItem *parentItem,
                        const QJsonObject &dataObject,
                        const uint gateOffset = 0,
                        const bool skipStartgrid = false);

  static QString getJsonValueTypeAsString(const QJsonValue::Type type);
};
#endif // VELOJSONPARSER_H
