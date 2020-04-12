#ifndef TRACKPARSER_H
#define TRACKPARSER_H

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItem>
#include <QString>
#include <QTreeWidgetItem>

#include "exceptions.h"
#include "velodb.h"
#include "velojsonparser.h"

enum NodeTreeColumns {
  KeyColumn = 0,
  ValueColumn = 1,
  TypeColumn = 2
};

class VeloTrack : QObject
{
  Q_OBJECT

public:  
  VeloTrack();
  ~VeloTrack();

  QByteArray* exportAsJsonData();
  void importJsonData(const QByteArray *jsonData);
  bool isModified();
  void mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates);

  uint                  getGateCount() const;
  uint                  getNodeCount() const;
  PrefabData            getPrefab(const uint id) const;
  uint                  getPrefabCount() const;
  QString               getPrefabDesc(const uint id) const;
  QVector<PrefabData>*  getPrefabs() const;
  QVector<PrefabData>*  getPrefabsInUse() const;
  QModelIndex           getRootIndex() const;
  uint                  getSceneId() const;
  uint                  getSplineCount() const;
  QStandardItemModel*   getStandardItemModel() const;

  uint replacePrefab(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId);
  void resetFinishGates();
  void resetModified();
  void resetStartGates();
  void setSceneId(const uint &value);
  void setPrefabs(QVector<PrefabData> *value);

  void changeGateOrder(const uint oldGateNo, const uint newGateNo);

  static bool isNotEditableNode(const QModelIndex &keyIndex);
  static bool isStartGrid(const PrefabData &prefab);

private:
  uint sceneId;
  QJsonDocument* doc = nullptr;
  QVector<PrefabData>* prefabs;
  QStandardItemModel* model;

  uint nodeCount = 0;
  uint prefabCount = 0;
  uint splineCount = 0;
  uint gateCount = 0;

  QList<QModelIndex> findPrefabs(const QModelIndex &keyItemIndex) const;
  bool isModifiedNode(const QStandardItem* item) const;
  void resetModifiedNodes(const QStandardItem* item) const;
};

#endif // TRACKPARSER_H
