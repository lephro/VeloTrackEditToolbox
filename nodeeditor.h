#ifndef TRACKPARSER_H
#define TRACKPARSER_H

#include <cmath>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QSettings>
#include <QStandardItem>
#include <QString>
#include <QTreeWidgetItem>
#include <QQuaternion>
#include <QVector3D>

#include "exceptions.h"
#include "filterproxymodel.h"
#include "nodefilter.h"
#include "prefabitem.h"
#include "velodb.h"
#include "velodataparser.h"

enum NodeTreeColumns {
  KeyColumn = 0,
  ValueColumn = 1,
  TypeColumn = 2
};

enum ToolTypes {
  Replace             = 0,
  Move                = 1,
  ReplacePosition     = 2,
  IncreasingPosition  = 3,
  AddRotation         = 4,
  ReplaceRotation     = 5,
  IncreasingRotation  = 6,
  Scale               = 7,
  ReplaceScaling      = 8,
  IncreasingScale     = 9,
  Mirror              = 20
};

class PrefabItem;

class NodeEditor : QObject
{
  Q_OBJECT

public:  
  NodeEditor();
  ~NodeEditor();

  void                        changeGateOrder(const uint oldGateNo, const uint newGateNo);
  void                        deleteNode(const QModelIndex &index) const;
  QModelIndex                 dublicatePrefab(PrefabItem *sourcePrefab);
  QByteArray*                 exportAsJsonData();
  uint                        getGateCount() const;
  FilterProxyModel&           getFilteredModel();
  uint                        getNodeCount() const;
  const PrefabData            getPrefab(const uint id) const;
  uint                        getPrefabCount() const;
  QString                     getPrefabDesc(const uint id) const;
  const QVector<PrefabData>*  getPrefabData() const;
  QVector<PrefabData>         getPrefabsInUse(bool includeNonEditable = false) const;
  QModelIndex                 getRootIndex() const;
  uint                        getSceneId() const;
  uint                        getSplineCount() const;
  QStandardItemModel&         getStandardModel();
  void                        importJsonData(const QByteArray *jsonData);
  bool                        isModified();
  void                        mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates);
  uint                        replacePrefabs(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  uint                        replacePrefabs(const QModelIndexList &searchIndexList, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  uint                        replacePrefabs(const QVector<PrefabItem *> &prefabs, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  void                        resetModifiedFlag(const QStandardItem* item);
  uint                        transformPrefab(const QModelIndex& searchIndex, const ToolTypes toolType, const QVariant &value, const bool byPercent = false);
  uint                        transformPrefab(const QModelIndexList& searchIndexList, const ToolTypes toolType, const QVariant &value, const bool byPercent = false);
  uint                        transformPrefab(const QVector<PrefabItem*>& prefabs, const ToolTypes toolType, const QVariant &value, const bool byPercent = false);
  void                        resetFinishGates();
  void                        resetModifiedFlags();
  void                        resetFilterMarks();
  void                        resetStartGates();
  QVector<PrefabItem*>        search(const QVector<PrefabItem*>& index, const QVector<NodeFilter *> &filterList);
  QVector<PrefabItem*>        search(QModelIndex& index, QVector<NodeFilter*> filterList);
  QVector<PrefabItem*>        search(QList<QStandardItem*> items, QVector<NodeFilter*> filterList);
  void                        setSceneId(const uint& value);
  void                        setPrefabs(const QVector<PrefabData> value);

  static bool isEditableNode(const QModelIndex& keyIndex);
  static bool isStartGrid(const PrefabData& prefab);

private:
  const QBrush filterFontColor = QBrush(QColor(Qt::black));
  const QBrush filterBackgroundColor = QBrush(QColor(254, 203, 137));
  const QBrush filterContentBackgroundColor = QBrush(QColor(192, 192, 192));
  uint sceneId;
  QBrush defaultFontColor = QBrush(Qt::white);
  QBrush defaultBackgroundColor = QBrush(QColor(255, 255, 255, 0));
  QJsonDocument* doc = nullptr;
  QVector<PrefabData> prefabs;
  QStandardItemModel model;
  FilterProxyModel filteredModel;

  uint nodeCount = 0;
  uint prefabCount = 0;
  uint splineCount = 0;
  uint gateCount = 0;

  bool                  containsModifiedNode(const QStandardItem* item) const;
  void                  dublicateChildren(const QStandardItem* source, QStandardItem* target);
  QList<QStandardItem*> findPrefabs(const QModelIndex &keyItemIndex) const;
  QList<QStandardItem*> findPrefabs(const QModelIndexList &keyItemIndexList) const;
  void                  applyFilterToList(QVector<PrefabItem *> &items, const QVector<NodeFilter *> &filter, const FilterTypes filterType, const QVector<PrefabItem *> *initalItems = nullptr);
};

#endif // TRACKPARSER_H
