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

class VeloDataParser : QObject
{
  Q_OBJECT

public:  
  VeloDataParser();
  ~VeloDataParser();

  QByteArray *exportTrackDataFromModel();
  void importTrackDataToModel(const QByteArray *jsonData);

  int                 getGateCount() const;
  QStandardItemModel* getStandardItemModel() const;
  Prefab              getPrefab(const uint id) const;
  QString             getPrefabDesc(const uint id) const;
  QVector<Prefab>*    getPrefabs() const;
  QVector<Prefab>*    getPrefabsInUse() const;
  QModelIndex         getRootIndex() const;
  uint                getSceneId() const;

  uint replacePrefab(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId);
  void resetFinishGates();
  void resetStartGates();
  void setSceneId(const uint &value);
  void setPrefabs(QVector<Prefab> *value);

  void changeGateOrder(const uint oldGateNo, const uint newGateNo);
private:
  uint sceneId;
  QJsonDocument* doc = nullptr;
  QVector<Prefab>* prefabs;
  QStandardItemModel* model;

  QList<QModelIndex> findPrefabs(const QModelIndex &keyItemIndex) const;
  QString getQJsonValueTypeString(const QJsonValue::Type type) const;
  void importJsonArray(QStandardItem *parentItem, const QJsonArray &dataArray);
  void importJsonObject(QStandardItem *parentItem, const QJsonObject &dataObject);
  QJsonArray *exportToDataArray(const QStandardItem *treeItem);
  QJsonObject *exportToObject(const QStandardItem *treeItem);
};

#endif // TRACKPARSER_H
