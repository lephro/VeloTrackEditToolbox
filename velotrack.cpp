#include "velotrack.h"

VeloTrack::VeloTrack()
{
  model = new QStandardItemModel();
}

VeloTrack::~VeloTrack()
{
  delete model;
}

QByteArray *VeloTrack::exportAsJsonData()
{
  VeloDataParser parser(nullptr, prefabs, model);
  QByteArray* veloByteData = parser.exportToJson();

  if (nodeCount != parser.getNodeCount())
    qDebug() << "Export error: Wrong node count! " << nodeCount << " vs " << parser.getNodeCount();

  if (prefabCount != parser.getPrefabCount())
    qDebug() << "Export error: Wrong prefab count!" << prefabCount << " vs " << parser.getPrefabCount();

  if (gateCount != parser.getGateCount())
    qDebug() << "Export error: Wrong gate count!" << gateCount << " vs " << parser.getGateCount();

  if (splineCount != parser.getSplineCount())
    qDebug() << "Export error: Wrong spline count!" << splineCount << " vs " << parser.getSplineCount();

  return veloByteData;
}

void VeloTrack::importJsonData(const QByteArray *jsonData)
{
  VeloDataParser parser(nullptr, prefabs, model);
  parser.importJson(jsonData);
  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}


void VeloTrack::mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates)
{
  VeloDataParser parser(nullptr, prefabs, model);
  parser.mergeJson(jsonData, addBarriers, addGates);
  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}

void VeloTrack::changeGateOrder(const uint oldGateNo, const uint newGateNo)
{
  bool shiftLeft = (int(oldGateNo) - int(newGateNo)) > 0;
  QList<QStandardItem*> gateKeys = model->findItems("gate", Qt::MatchRecursive, 0);
  for (int i = 0; i < gateKeys.size(); ++i) {
    QModelIndex gateValueIndex = gateKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (gateValueIndex.isValid()) {
      uint gateNo = gateValueIndex.data().toUInt();
      if (shiftLeft && (gateNo >= newGateNo) && (gateNo < oldGateNo))
        model->setData(gateValueIndex, gateNo + 1, Qt::EditRole);
      else if (!shiftLeft && (gateNo > oldGateNo) && (gateNo <= newGateNo))
        model->setData(gateValueIndex, gateNo - 1, Qt::EditRole);
    }
  }
}

bool VeloTrack::isNotEditableNode(const QModelIndex& keyIndex)
{
  QModelIndex valueIndex = keyIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
  PrefabData prefab = valueIndex.data(Qt::UserRole).value<PrefabData>();

  if (!valueIndex.isValid())
    return false;

  if (isStartGrid(prefab))
    return true;

  if (prefab.id > 0) {
    return ((prefab.name == "CtrlParent") ||
            (prefab.name == "ControlCurve") ||
            (prefab.name == "ControlPoint"));
  } else if (keyIndex.data() == "finish") {
    return valueIndex.data().toBool() == true;
  } else if (keyIndex.data() == "start") {
    return valueIndex.data().toBool() == true;
  }

  return false;
}

bool VeloTrack::isStartGrid(const PrefabData& prefab)
{
  if (prefab.id > 0) {
    return ((prefab.name == "DefaultStartGrid") ||
            (prefab.name == "DefaultKDRAStartGrid") ||
            (prefab.name == "DR1StartGrid") ||
            (prefab.name == "PolyStartGrid") ||
            (prefab.name == "MicroStartGrid"));
  }
  return false;
}

uint VeloTrack::getNodeCount() const
{
  return nodeCount;
}

uint VeloTrack::getPrefabCount() const
{
  return prefabCount;
}

uint VeloTrack::getSplineCount() const
{
  return splineCount;
}

bool VeloTrack::isModified()
{
  return isModifiedNode(model->invisibleRootItem());
}

uint VeloTrack::getGateCount() const
{
  return uint(model->findItems("gate", Qt::MatchRecursive, 0).size());
}

PrefabData VeloTrack::getPrefab(const uint id) const
{
  for (QVector<PrefabData>::iterator i = prefabs->begin(); i != prefabs->end(); ++i)
    if (i->id == id)
      return *i;

  return PrefabData();
}

QString VeloTrack::getPrefabDesc(const uint id) const
{
  for (QVector<PrefabData>::iterator i = prefabs->begin(); i != prefabs->end(); ++i)
    if (i->id == id)
      return i->name + " (" + i->type + ")";

  return "";
}

QVector<PrefabData>* VeloTrack::getPrefabs() const
{
  return prefabs;
}

QVector<PrefabData>* VeloTrack::getPrefabsInUse() const
{
  QMap<uint, PrefabData> prefabMap;
  QList<QModelIndex> prefabs = findPrefabs(model->invisibleRootItem()->index());
  foreach (QModelIndex childIndex, prefabs) {
    PrefabData prefab = childIndex.siblingAtColumn(NodeTreeColumns::ValueColumn).data(Qt::UserRole).value<PrefabData>();
    if ((!prefabMap.contains(prefab.id)) && (prefab.id > 0))
      if (!isNotEditableNode(childIndex))
        prefabMap.insert(prefab.id, prefab);
  }

  QVector<PrefabData> prefabsInUse;
  for (QMap<uint, PrefabData>::iterator i = prefabMap.begin(); i != prefabMap.end(); ++i)
    prefabsInUse.append(i.value());

  std::sort(prefabsInUse.begin(), prefabsInUse.end());

  return new QVector<PrefabData>(prefabsInUse);
}

QModelIndex VeloTrack::getRootIndex() const
{
  return model->invisibleRootItem()->index();
}

uint VeloTrack::getSceneId() const
{
  return sceneId;
}

uint VeloTrack::replacePrefab(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId)
{
  uint prefabCount = 0;

  PrefabData toPrefab = getPrefab(toPrefabId);

  QList<QModelIndex> prefabs = findPrefabs(searchIndex);
  foreach (QModelIndex prefabIndex, prefabs) {
    QModelIndex prefabValueIndex = prefabIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
    PrefabData prefab = prefabValueIndex.data(Qt::UserRole).value<PrefabData>();
    if (prefab.id == fromPrefabId) {
      model->setData(prefabIndex, true, Qt::UserRole);
      model->setData(prefabValueIndex, toPrefab.name, Qt::EditRole);

      QVariant var;
      var.setValue(toPrefab);
      model->setData(prefabValueIndex, var, Qt::UserRole);

      QModelIndex parentKeyIndex = prefabIndex.parent();
      if (parentKeyIndex.data(Qt::DisplayRole).toString() == prefab.name) {
        model->setData(parentKeyIndex, toPrefab.name, Qt::EditRole);
      }
      prefabCount++;
    }
  }

  return prefabCount;
}

void VeloTrack::resetFinishGates()
{
  QList<QStandardItem*> finishKeys = model->findItems("finish", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < finishKeys.size(); ++i) {
    QModelIndex finishValueIndex = finishKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model->setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void VeloTrack::resetModified()
{
  resetModifiedNodes(model->invisibleRootItem());
}

void VeloTrack::resetStartGates()
{
  QList<QStandardItem*> startKeys = model->findItems("start", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < startKeys.size(); ++i) {
    QModelIndex finishValueIndex = startKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model->setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void VeloTrack::setSceneId(const uint &value)
{
  sceneId = value;
}

void VeloTrack::setPrefabs(QVector<PrefabData> *value)
{
  prefabs = value;
}

QStandardItemModel *VeloTrack::getStandardItemModel() const
{
  return model;
}

void VeloTrack::resetModifiedNodes(const QStandardItem* item) const
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    model->setData(item->child(i, 0)->index(), true, Qt::UserRole);
    QStandardItem* child = item->child(i, 0);
    if (child->hasChildren())
      resetModifiedNodes(child);
  }
}

QList<QModelIndex> VeloTrack::findPrefabs(const QModelIndex& keyItemIndex) const
{
  QList<QModelIndex> foundPrefabs;
  for (int i = 0; i < model->rowCount(keyItemIndex); ++i)
  {
    QModelIndex childIndex = model->index(i, NodeTreeColumns::KeyColumn, keyItemIndex);
    if (childIndex.data(Qt::DisplayRole) == "prefab")
      foundPrefabs.append(childIndex);
    if (model->hasChildren(childIndex)) {
      foundPrefabs += findPrefabs(childIndex);
    }
  }

  return foundPrefabs;
}


bool VeloTrack::isModifiedNode(const QStandardItem* item) const
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    QStandardItem* child = item->child(i, 0);
    //bool test = child->data(Qt::UserRole).toBool();
    //qDebug() << child->index() << " " << child->row() << ":" << child->column() << "(" << child->text() << ") = " << test;
    if (child->data(Qt::UserRole).toBool())
      return true;

    if (child->hasChildren()) {
      if (isModifiedNode(child))
        return true;
    }
  }

  return false;
}

