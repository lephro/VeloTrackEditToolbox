#ifndef TRACKPARSER_H
#define TRACKPARSER_H

#include <cmath>
#include <QDebug>
#include <QList>
#include <QMap>
#include <QMessageBox>
#include <QScrollBar>
#include <QSettings>
#include <QStandardItem>
#include <QString>
#include <QTreeView>
#include <QTreeWidgetItem>
#include <QQuaternion>
#include <QVector3D>

#include "track.h"

#include "editormodel.h"
#include "editorobject.h"
#include "exceptions.h"
#include "filterproxymodel.h"
#include "nodefilter.h"
#include "velodataparser.h"
#include "velodb.h"

enum ToolTypes {
  Replace             = 0,
  Move                = 1,
  IncreasingPosition  = 2,
  ReplacePosition     = 3,
  MultiplyPosition    = 4,
  AddRotation         = 5,
  IncreasingRotation  = 6,
  ReplaceRotation     = 7,
  Scale               = 8,
  IncreasingScale     = 9,
  ReplaceScaling      = 10,
  MultiplyScaling     = 11,
  Mirror              = 20
};

enum ToolTypeTargets {
  RGB = 0,
  R   = 1,
  G   = 2,
  B   = 3
};

class EditorModel;
class EditorModelItem;
class EditorObject;

class NodeEditor : public QObject
{
  Q_OBJECT

public:  
  explicit NodeEditor(Track &track);

//  NodeEditor& operator =(const NodeEditor& b);

  void                        beginNodeEdit();
  void                        changeGateOrder(const uint oldGateNo, const uint newGateNo);
  void                        clearFilterMarks();
  void                        clearModifiedFlag(EditorModelItem* modelItem = nullptr);
  void                        clearSearch(const int cacheId);
  void                        deleteNode(const QModelIndex &index) const;
  QModelIndex                 duplicateObject(EditorObject* sourceObject);
  void                        endNodeEdit();
  QByteArray*                 exportAsJsonData();
  QVector<PrefabData>         getAllPrefabData() const;
  FilterProxyModel&           getFilteredModel();
  EditorObject*               getObjectByIndex(const QModelIndex index);
  const PrefabData            getPrefabData(const uint id) const;
  QString                     getPrefabDesc(const uint id) const;
  QVector<PrefabData>         getPrefabsInUse(bool includeNonEditable = false) const;
  QModelIndex                 getRootIndex() const;
  uint                        getSceneId() const;
  int                         getSearchCacheId() const;
  QVector<EditorObject*>      getSearchResult() const;
  EditorModel&                getEditorModel();
  TrackData&                  getTrackData();
  QTreeView&                  getTreeView() const;
  bool                        isModified();
  void                        mergeJsonData(const QByteArray& jsonData, const bool addBarriers, const bool addGates);
  uint                        replacePrefabs(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  uint                        replacePrefabs(const QModelIndexList& searchIndexList, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  uint                        replacePrefabs(const QVector<EditorObject*>& prefabs, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling = QVector3D(1, 1, 1));
  uint                        transformPrefab(const QModelIndex& searchIndex, const ToolTypes toolType, const QVariant& value, const ToolTypeTargets target = ToolTypeTargets::RGB, const bool byPercent = false);
  uint                        transformPrefab(const QModelIndexList& searchIndexList, const ToolTypes toolType, const QVariant &value, const ToolTypeTargets target = ToolTypeTargets::RGB, const bool byPercent = false);
  uint                        transformPrefab(const QVector<EditorObject*>& prefabs, const ToolTypes toolType, const QVariant &value, const ToolTypeTargets target = ToolTypeTargets::RGB, const bool byPercent = false);
  void                        resetFinishGates();  
  void                        resetStartGates();
  QVector<EditorObject*>      search(const QVector<EditorObject*>&  index, const QVector<NodeFilter*>& filterList);
  //QVector<EditorObject*>      search(QModelIndex& index, QVector<NodeFilter*> filterList);
  QVector<EditorObject*>      search(QList<EditorModelItem*> items, QVector<NodeFilter*> filterList);
  void                        setFilterMarks();
  void                        setSearchFilter(const bool enable);
  void                        setSearchResult(const int cacheId, const QVector<EditorObject*>& value);
  void                        setSceneId(const uint& value);
  void                        setTrackData(const TrackData& value);

  static bool isStartGrid(const PrefabData& prefab);  

  Track *getTrack() const;
  void setTrack(Track *newTrack);

private:
  uint sceneId;
  int searchCacheId = INT_MIN;
  bool editStarted = false;

  Track* track;
  QTreeView* treeView;
  EditorModel* editorModel;
  FilterProxyModel filteredModel;  
  QVector<EditorObject*> searchResult;

  float lastScrollbarPos;
  QVector<bool> lastTreeExpansionStates;

  uint nodeCount = 0;
  uint prefabCount = 0;
  uint splineCount = 0;
  uint gateCount = 0;  

  void applyFilterToList(QVector<EditorObject*>& items, const QVector<NodeFilter*>& filter, const FilterTypes filterType, const QVector<EditorObject*>* initalItems = nullptr);
  bool containsModifiedNode() const;
};

#endif // TRACKPARSER_H
