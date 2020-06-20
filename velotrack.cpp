#include "velotrack.h"

VeloTrack::VeloTrack()
{

}

VeloTrack::~VeloTrack()
{

}

QByteArray *VeloTrack::exportAsJsonData()
{
  VeloDataParser parser(nullptr, &prefabs, &model);
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
  VeloDataParser parser(nullptr, &prefabs, &model);
  parser.importJson(jsonData);
  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}


void VeloTrack::mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates)
{
  VeloDataParser parser(nullptr, &prefabs, &model);
  parser.mergeJson(jsonData, addBarriers, addGates);
  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}

void VeloTrack::changeGateOrder(const uint oldGateNo, const uint newGateNo)
{
  bool shiftLeft = (int(oldGateNo) - int(newGateNo)) > 0;
  QList<QStandardItem*> gateKeys = model.findItems("gate", Qt::MatchRecursive, 0);
  for (int i = 0; i < gateKeys.size(); ++i) {
    QModelIndex gateValueIndex = gateKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (gateValueIndex.isValid()) {
      uint gateNo = gateValueIndex.data().toUInt();
      if (shiftLeft && (gateNo >= newGateNo) && (gateNo < oldGateNo))
        model.setData(gateValueIndex, gateNo + 1, Qt::EditRole);
      else if (!shiftLeft && (gateNo > oldGateNo) && (gateNo <= newGateNo))
        model.setData(gateValueIndex, gateNo - 1, Qt::EditRole);
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
  return containsModifiedNode(model.invisibleRootItem());
}

uint VeloTrack::getGateCount() const
{
  return uint(model.findItems("gate", Qt::MatchRecursive, 0).size());
}

const PrefabData VeloTrack::getPrefab(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = prefabs.begin(); i != prefabs.end(); ++i)
    if (i->id == id)
      return *i;

  return PrefabData();
}

QString VeloTrack::getPrefabDesc(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = prefabs.begin(); i != prefabs.end(); ++i)
    if (i->id == id)
      return i->name + " (" + i->type + ")";

  return "";
}

const QVector<PrefabData>* VeloTrack::getPrefabs() const
{
  return &prefabs;
}

QVector<PrefabData> VeloTrack::getPrefabsInUse() const
{
  QMap<uint, PrefabData> prefabMap;
  QList<QModelIndex> prefabs = findPrefabs(model.invisibleRootItem()->index());
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

  return prefabsInUse;
}

QModelIndex VeloTrack::getRootIndex() const
{
  return model.invisibleRootItem()->index();
}

uint VeloTrack::getSceneId() const
{
  return sceneId;
}

uint VeloTrack::replacePrefab(const QModelIndex &searchIndex, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  uint prefabCount = 0;

  const PrefabData toPrefab = getPrefab(toPrefabId);

  const QList<QModelIndex> prefabs = findPrefabs(searchIndex);
  foreach (QModelIndex prefabIndex, prefabs) {
    const QModelIndex prefabValueIndex = prefabIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
    const PrefabData prefab = prefabValueIndex.data(Qt::UserRole).value<PrefabData>();
    if (prefab.id == fromPrefabId) {
      // Set modified flag
      model.setData(prefabIndex, true, Qt::UserRole);

      // Set item description
      model.setData(prefabValueIndex, toPrefab.name, Qt::EditRole);

      // Update item data
      QVariant var;
      var.setValue(toPrefab);
      model.setData(prefabValueIndex, var, Qt::UserRole);

      // Update parent description
      model.setData(prefabIndex.parent(), toPrefab.name + " " + QString("(%1)").arg(toPrefabId, 0, 10), Qt::EditRole);

      // Apply Scaling
      const QStandardItem* parentItem = model.itemFromIndex(prefabIndex.parent());
      for(int transRow = 0; transRow < parentItem->rowCount(); ++transRow) {
        const QStandardItem* transItem = parentItem->child(transRow, NodeTreeColumns::KeyColumn);
        if (transItem->text() == "trans") {
          for(int scaleRow = 0; scaleRow < transItem->rowCount(); ++scaleRow) {
            const QStandardItem* scaleItem = transItem->child(scaleRow, NodeTreeColumns::KeyColumn);
            if (scaleItem->text() == "scale") {
              QStandardItem* xValueItem = scaleItem->child(0, NodeTreeColumns::ValueColumn);
              QStandardItem* yValueItem = scaleItem->child(1, NodeTreeColumns::ValueColumn);
              QStandardItem* zValueItem = scaleItem->child(2, NodeTreeColumns::ValueColumn);
              if ((xValueItem != nullptr) && (scaling.x() != 1.0f)) {
                xValueItem->setData(xValueItem->text().toFloat() * scaling.x(), Qt::EditRole);
              }
              if ((yValueItem != nullptr) && (scaling.y() != 1.0f)) {
                yValueItem->setData(yValueItem->text().toFloat() * scaling.y(), Qt::EditRole);
              }
              if ((zValueItem != nullptr) && (scaling.z() != 1.0f)) {
                zValueItem->setData(zValueItem->text().toFloat() * scaling.z(), Qt::EditRole);
              }
              break;
            }
          }
          break;
        }
      }

      prefabCount++;
    }
  }

  return prefabCount;
}

void VeloTrack::repositionAndRescale(const QModelIndex &searchIndex, const QVector3D positionOffset, const QVector3D scale)
{
  Q_UNUSED(searchIndex)
  Q_UNUSED(positionOffset)
  Q_UNUSED(scale)
}

void VeloTrack::resetFinishGates()
{
  QList<QStandardItem*> finishKeys = model.findItems("finish", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < finishKeys.size(); ++i) {
    QModelIndex finishValueIndex = finishKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model.setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void VeloTrack::resetModified()
{
  resetModifiedNodes(model.invisibleRootItem());
}

void VeloTrack::resetStartGates()
{
  QList<QStandardItem*> startKeys = model.findItems("start", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < startKeys.size(); ++i) {
    QModelIndex finishValueIndex = startKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model.setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

QList<PrefabItem*> VeloTrack::search(const QList<PrefabItem*>& initialMatchList, QList<NodeFilter*> filterList) {
  QList<PrefabItem*> matchList = initialMatchList;
  foreach(PrefabItem* prefab, matchList) {
    qDebug() << prefab << prefab->getId() << prefab->getData();
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::Object)
      continue;

    foreach(PrefabItem* prefab, matchList) {
      if (int(prefab->getId()) != filter->getFilterValue()) {
        matchList.removeOne(prefab);
      }
    }
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::Position)
      continue;

    QString filterValue = QString("%1").arg(filter->getFilterValue());
    foreach(PrefabItem* prefab, matchList) {
      bool match = false;
      for (int i = 0; i < 3; ++i) {
        QString prefabValue = QString("%1").arg(prefab->getPosition(i));
        if (prefabValue.indexOf(filterValue) > -1)
          match = true;
      }
      if (!match)
        matchList.removeOne(prefab);
    }
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::Rotation)
      continue;

    QString filterValue = QString("%1").arg(filter->getFilterValue());
    foreach(PrefabItem* prefab, matchList) {
      bool match = false;
      for (int i = 0; i < 3; ++i) {
        QString prefabValue = QString("%1").arg(prefab->getRotation(i));
        if (prefabValue.indexOf(filterValue) > -1)
          match = true;
      }
      if (!match)
        matchList.removeOne(prefab);
    }
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::Scaling)
      continue;

    QString filterValue = QString("%1").arg(filter->getFilterValue());
    foreach(PrefabItem* prefab, matchList) {
      bool match = false;
      for (int i = 0; i < 3; ++i) {
        QString prefabValue = QString("%1").arg(prefab->getScaling(i));
        if (prefabValue.indexOf(filterValue) > -1)
          match = true;
      }
      if (!match)
        matchList.removeOne(prefab);
    }
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::GateNo)
      continue;

    foreach(PrefabItem* prefab, matchList) {
      if (int(prefab->getGateNo()) != filter->getFilterValue()) {
        matchList.removeOne(prefab);
      }
    }
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::IsOnSpline)
      continue;
    // ToDo
    // Check if spline index is valid?
  }

  if (matchList.count() == 0)
    return matchList;

  foreach(NodeFilter* filter, filterList) {
    if (filter->getFilterType() != FilterTypes::IsDublicate)
      continue;
    foreach(PrefabItem* prefab, matchList) {
      // Search for a prefab that matches our own
      // We just create a new filter set and search at our index again :)
      QList<NodeFilter*> dublicateFilterList;
      NodeFilter dublicateIdFilter(nullptr, FilterTypes::Object, int(prefab->getId()), "");
      NodeFilter dublicatePosRFilter(nullptr, FilterTypes::Position, prefab->getPositionR(), "");
      NodeFilter dublicatePosGFilter(nullptr, FilterTypes::Position, prefab->getPositionB(), "");
      NodeFilter dublicatePosBFilter(nullptr, FilterTypes::Position, prefab->getPositionG(), "");
      NodeFilter dublicateRotLFilter(nullptr, FilterTypes::Rotation, prefab->getRotationL(), "");
      NodeFilter dublicateRotIFilter(nullptr, FilterTypes::Rotation, prefab->getRotationI(), "");
      NodeFilter dublicateRotJFilter(nullptr, FilterTypes::Rotation, prefab->getRotationJ(), "");
      NodeFilter dublicateRotKFilter(nullptr, FilterTypes::Rotation, prefab->getRotationK(), "");
      NodeFilter dublicateScaleRFilter(nullptr, FilterTypes::Scaling, prefab->getScalingR(), "");
      NodeFilter dublicateScaleGFilter(nullptr, FilterTypes::Scaling, prefab->getScalingB(), "");
      NodeFilter dublicateScaleBFilter(nullptr, FilterTypes::Scaling, prefab->getScalingG(), "");
      dublicateFilterList.append(&dublicateIdFilter);
      dublicateFilterList.append(&dublicatePosRFilter);
      dublicateFilterList.append(&dublicatePosGFilter);
      dublicateFilterList.append(&dublicatePosBFilter);
      dublicateFilterList.append(&dublicateRotLFilter);
      dublicateFilterList.append(&dublicateRotIFilter);
      dublicateFilterList.append(&dublicateRotJFilter);
      dublicateFilterList.append(&dublicateRotKFilter);
      dublicateFilterList.append(&dublicateScaleRFilter);
      dublicateFilterList.append(&dublicateScaleGFilter);
      dublicateFilterList.append(&dublicateScaleBFilter);

      search(initialMatchList, dublicateFilterList);
    }
  }

  foreach(PrefabItem* prefab, matchList) {
    qDebug() << prefab << prefab->getId() << prefab->getData();
  }
}

void VeloTrack::search(QModelIndex& index, QList<NodeFilter*> filterList) {
  QList<PrefabItem*> matchList;
  QList<QModelIndex> prefabs = findPrefabs(index);
  foreach(QModelIndex prefabIndex, prefabs) {
    PrefabItem* prefab = new PrefabItem();
    prefab->parseIndex(&model, prefabIndex);
    matchList.append(prefab);
  }

  search(matchList, filterList);
}

void VeloTrack::setSceneId(const uint &value)
{
  sceneId = value;
}

void VeloTrack::setPrefabs(const QVector<PrefabData> value)
{
  prefabs = value;
}

QStandardItemModel& VeloTrack::getStandardItemModel()
{
  return model;
}

void VeloTrack::resetModifiedNodes(const QStandardItem* item)
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    model.setData(item->child(i, 0)->index(), true, Qt::UserRole);
    QStandardItem* child = item->child(i, 0);
    if (child->hasChildren())
      resetModifiedNodes(child);
  }
}

QList<QModelIndex> VeloTrack::findPrefabs(const QModelIndex& keyItemIndex) const
{
  QList<QModelIndex> foundPrefabs;
  for (int i = 0; i < model.rowCount(keyItemIndex); ++i)
  {
    QModelIndex childIndex = model.index(i, NodeTreeColumns::KeyColumn, keyItemIndex);
    if (childIndex.data(Qt::DisplayRole) == "prefab")
      foundPrefabs.append(childIndex);
    if (model.hasChildren(childIndex)) {
      foundPrefabs += findPrefabs(childIndex);
    }
  }

  return foundPrefabs;
}


bool VeloTrack::containsModifiedNode(const QStandardItem* item) const
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    QStandardItem* child = item->child(i, 0);
    //bool test = child->data(Qt::UserRole).toBool();
    //qDebug() << child->index() << " " << child->row() << ":" << child->column() << "(" << child->text() << ") = " << test;
    if (child->data(Qt::UserRole).toBool())
      return true;

    if (child->hasChildren()) {
      if (containsModifiedNode(child))
        return true;
    }
  }

  return false;
}

