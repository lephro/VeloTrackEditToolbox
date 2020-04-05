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

  QByteArray *exportAsJsonData();
  void importJsonData(const QByteArray *jsonData);
  bool isModified();
  void mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates);

  uint getGateCount() const;
  PrefabData            getPrefab(const uint id) const;
  QString               getPrefabDesc(const uint id) const;
  QVector<PrefabData>*  getPrefabs() const;
  QVector<PrefabData>*  getPrefabsInUse() const;
  QModelIndex           getRootIndex() const;
  uint                  getSceneId() const;
  QStandardItemModel*   getStandardItemModel() const;

  uint replacePrefab(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId);
  void resetFinishGates();
  void resetModified();
  void resetStartGates();
  void setSceneId(const uint &value);
  void setPrefabs(QVector<PrefabData> *value);

  void changeGateOrder(const uint oldGateNo, const uint newGateNo);

  static bool isNotEditableNode(const QModelIndex &keyIndex);
  static bool isStartGrid(PrefabData &prefab);
private:
  uint sceneId;
  QJsonDocument* doc = nullptr;
  QVector<PrefabData>* prefabs;
  QStandardItemModel* model;

  QList<QModelIndex> findPrefabs(const QModelIndex &keyItemIndex) const;
  QString getJsonValueTypeString(const QJsonValue::Type type) const;
  void importJsonArray(QStandardItem *parentItem,
                       const QJsonArray &dataArray,
                       const uint gateOffset = 0,
                       const bool skipStartgrid = false);
  void importJsonObject(QStandardItem *parentItem,
                        const QJsonObject &dataObject,
                        const uint gateOffset = 0,
                        const bool skipStartgrid = false);
  bool isModifiedNode(const QStandardItem* item) const;
  QJsonArray *exportToDataArray(const QStandardItem *treeItem);
  QJsonObject *exportToObject(const QStandardItem *treeItem);
  void resetModifiedNodes(const QStandardItem* item) const;
};

#endif // TRACKPARSER_H
