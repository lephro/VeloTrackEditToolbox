#include "nodeeditor.h"

NodeEditor::NodeEditor()
{
  filteredModel.setRecursiveFilteringEnabled(true);
  filteredModel.setSourceModel(&model);
}

NodeEditor::~NodeEditor()
{

}

void NodeEditor::applyFilterToList(QVector<PrefabItem*>& matchingItems, const QVector<NodeFilter*>& filter, const FilterTypes filterType, const QVector<PrefabItem*>* initalItems)
{
  // Define our control variables outside of the loops to prevent excessive alloc and deallocs
  bool match = false;
  int rowCount = 1;
  QString filterValue = "";
  QString prefabValue = "";
  NodeFilter* nodeFilter = nullptr;
  PrefabItem* prefab = nullptr;

  // Go through every filter in the list  and eventually remove all items that dont apply to it
  for (int i = 0; i < filter.count(); ++i) {
    nodeFilter = filter.at(i);

    if (nodeFilter == nullptr || nodeFilter->getFilterType() != filterType)
      continue;

    filterValue = QString("%1").arg(nodeFilter->getFilterValue());

    // Every filter is applied to every remaining item in the list
    for (int item = 0; item < matchingItems.count(); ++item) {
      match = false;

      prefab = matchingItems.at(item);

      if (prefab == nullptr)
        continue;

      if (filterType == FilterTypes::GateNo && !prefab->isGate()) {
        matchingItems.removeOne(prefab);
        item--;
        continue;
      }

      // Some filter need to loop through multiple rows
      switch (filterType) {
      case FilterTypes::AnyPosition:
      case FilterTypes::AnyScaling:
        rowCount = 3;
        break;
      case FilterTypes::AnyRotation:
        rowCount = 4;
        break;
      default:
        rowCount = 1;
      }
      for (int row = 0; row < rowCount; ++row) {
        // Set the corresponding value of the filter and apply it
        switch (filterType) {
        case FilterTypes::Object:
          // We can do the check right away to save some nanoseconds lmao
          match = (int(prefab->getId()) == nodeFilter->getFilterValue());
          continue;
        case FilterTypes::AnyPosition:
          prefabValue = QString("%1").arg(prefab->getPosition(row));
          break;
        case FilterTypes::PositionR:
          prefabValue = QString("%1").arg(prefab->getPositionR());
          break;
        case FilterTypes::PositionG:
          prefabValue = QString("%1").arg(prefab->getPositionG());
          break;
        case FilterTypes::PositionB:
          prefabValue = QString("%1").arg(prefab->getPositionB());
          break;
        case FilterTypes::AnyRotation:
          prefabValue = QString("%1").arg(prefab->getRotationVector(row));
          break;
        case FilterTypes::RotationW:
          prefabValue = QString("%1").arg(prefab->getRotationW());
          break;
        case FilterTypes::RotationX:
          prefabValue = QString("%1").arg(prefab->getRotationX());
          break;
        case FilterTypes::RotationY:
          prefabValue = QString("%1").arg(prefab->getRotationY());
          break;
        case FilterTypes::RotationZ:
          prefabValue = QString("%1").arg(prefab->getRotationZ());
          break;
        case FilterTypes::AnyScaling:
          prefabValue = QString("%1").arg(prefab->getScaling(row));
          break;
        case FilterTypes::ScalingR:
          prefabValue = QString("%1").arg(prefab->getScalingR());
          break;
        case FilterTypes::ScalingG:
          prefabValue = QString("%1").arg(prefab->getScalingG());
          break;
        case FilterTypes::ScalingB:
          prefabValue = QString("%1").arg(prefab->getScalingB());
          break;
        case FilterTypes::GateNo:
          prefabValue = QString("%1").arg(prefab->getGateNo());
          break;
        case FilterTypes::IsOnSpline:
          if (prefab->isOnSpline())
            match = true;
          continue;
        case FilterTypes::IsDublicate: {
          // Check if the prefab matches any other prefab
          PrefabItem* item;
          for(int i = 0; i < initalItems->count(); ++i) {
            item = initalItems->at(i);
            if (item->isSpline() || item->isOnSpline() || item->isSplineControl())
              continue;

            if (item->getIndex() != prefab->getIndex() && *item == *prefab)
              match = true;
          }
          continue;
        }
        default:
          continue;
        }

        switch (nodeFilter->getFilterMethod()) {
        case FilterMethods::Contains:
          if (prefabValue.indexOf(filterValue) > -1)
            match = true;
          break;
        case FilterMethods::Is:
          if (prefabValue == filterValue)
            match = true;
          break;
        case FilterMethods::SmallerThan:
          if (prefabValue.toInt() < nodeFilter->getFilterValue())
            match = true;
          break;
        case FilterMethods::BiggerThan:
          if (prefabValue.toInt() > nodeFilter->getFilterValue())
            match = true;
          break;
        }

        // No need to process the other rows, if one matches
        if (match)
          break;
      }

      // If no filter matched this prefab, remove it from the list and decrement the iterator
      if (!match) {
        matchingItems.removeOne(prefab);
        item--;
      }
    }
  }
}

void NodeEditor::changeGateOrder(const uint oldGateNo, const uint newGateNo)
{
  bool shiftLeft = (int(oldGateNo) - int(newGateNo)) > 0;
  const QList<QStandardItem*> gateKeys = model.findItems("gate", Qt::MatchRecursive, 0);
  for (int i = 0; i < gateKeys.size(); ++i) {
    const QModelIndex gateValueIndex = gateKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (gateValueIndex.isValid()) {
      uint gateNo = gateValueIndex.data().toUInt();
      if (shiftLeft && (gateNo >= newGateNo) && (gateNo < oldGateNo))
        model.setData(gateValueIndex, gateNo + 1, Qt::EditRole);
      else if (!shiftLeft && (gateNo > oldGateNo) && (gateNo <= newGateNo))
        model.setData(gateValueIndex, gateNo - 1, Qt::EditRole);
    }
  }
}

bool NodeEditor::containsModifiedNode(const QStandardItem* item) const
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    QStandardItem* child = item->child(i, 0);
    if (child->data(Qt::UserRole).toBool())
      return true;

    if (child->hasChildren()) {
      if (containsModifiedNode(child))
        return true;
    }
  }

  return false;
}

void NodeEditor::deleteNode(const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  QStandardItem* parentItem = model.itemFromIndex(index.parent());
  if (parentItem == nullptr)
    return;

  parentItem->removeRow(index.row());
}

void NodeEditor::dublicateChildren(const QStandardItem *source, QStandardItem *target)
{
  // Check if we got a valid source and target
  if (source == nullptr || target == nullptr)
    return;

  // Recursivly fill a new row with clones of the source columns (in the corresponding row)
  QList<QStandardItem*> newRowList;
  for(int row = 0; row < source->rowCount(); row++) {
    newRowList.clear();
    for(int column = 0; column < source->columnCount(); column++) {
      const QStandardItem* child = source->child(row, column);
      if (child == nullptr)
        continue;
      newRowList << child->clone();
    }

    if (newRowList.count() == 0)
      continue;

    dublicateChildren(source->child(row, NodeTreeColumns::KeyColumn), newRowList.first());
    target->appendRow(newRowList);
  }
}

QModelIndex NodeEditor::dublicatePrefab(PrefabItem* sourcePrefab)
{
  // Get the source columns
  QStandardItem* sourceItemKey = model.itemFromIndex(sourcePrefab->getIndex().siblingAtColumn(NodeTreeColumns::KeyColumn));
  QStandardItem* sourceItemValue = model.itemFromIndex(sourcePrefab->getIndex().siblingAtColumn(NodeTreeColumns::ValueColumn));
  QStandardItem* sourceItemType = model.itemFromIndex(sourcePrefab->getIndex().siblingAtColumn(NodeTreeColumns::TypeColumn));
  if (sourceItemKey == nullptr || sourceItemValue == nullptr || sourceItemType == nullptr)
    return QModelIndex();

  // Clone the source columns and their children
  QStandardItem* newItemKey = sourceItemKey->clone();
  dublicateChildren(sourceItemKey, newItemKey);
  QStandardItem* newItemValue = sourceItemValue->clone();
  QStandardItem* newItemType = sourceItemType->clone();

  // Create a new row and append it to the parent of the source
  QList<QStandardItem*> row;
  row << newItemKey << newItemValue << newItemType;

  QStandardItem* parent = sourceItemKey->parent();
  if (parent == nullptr)
    return QModelIndex();

  parent->appendRow(row);

  // Prevent invalid gate data
  PrefabItem prefab(this);
  if (prefab.parseIndex(newItemKey->index())) {
    if (prefab.getGateNo() > -1) {
      prefab.setGateNo(int(gateCount));
      gateCount++;
    }
    prefab.setFinish(false);
    prefab.setStart(false);
  }

  return sourceItemKey->index();
}

QByteArray *NodeEditor::exportAsJsonData()
{
  VeloDataParser parser(nullptr, &prefabs, &model);
  QByteArray* veloByteData = parser.exportToJson();

  if (nodeCount > parser.getNodeCount())
    qDebug() << "Export error: Wrong node count! " << nodeCount << " vs " << parser.getNodeCount();

  if (prefabCount != parser.getPrefabCount())
    qDebug() << "Export error: Wrong prefab count!" << prefabCount << " vs " << parser.getPrefabCount();

  if (gateCount != parser.getGateCount())
    qDebug() << "Export error: Wrong gate count!" << gateCount << " vs " << parser.getGateCount();

  if (splineCount != parser.getSplineCount())
    qDebug() << "Export error: Wrong spline count!" << splineCount << " vs " << parser.getSplineCount();

  return veloByteData;
}

QList<QStandardItem*> NodeEditor::findPrefabs(const QModelIndexList& keyItemIndexList) const
{
  // Go through all children of the given index and check if their key matches "prefab"
  QList<QStandardItem*> foundPrefabs;
  foreach(QModelIndex index, keyItemIndexList) {
    foundPrefabs += findPrefabs(index);
  }

  return foundPrefabs;
}

QList<QStandardItem*> NodeEditor::findPrefabs(const QModelIndex& keyItemIndex) const
{
  // Go through all children of the given index and check if their key matches "prefab"
  QList<QStandardItem*> foundPrefabs;
  for (int i = 0; i < filteredModel.rowCount(keyItemIndex); ++i)
  {
    QModelIndex childIndex = filteredModel.index(i, NodeTreeColumns::KeyColumn, keyItemIndex);
    if (childIndex.data(Qt::DisplayRole) == "prefab")
      foundPrefabs.append(model.itemFromIndex(filteredModel.mapToSource(childIndex)));
    if (filteredModel.hasChildren(childIndex)) {
      foundPrefabs += findPrefabs(childIndex);
    }
  }

  return foundPrefabs;
}

void NodeEditor::importJsonData(const QByteArray *jsonData)
{
  VeloDataParser parser(nullptr, &prefabs, &model);
  parser.importJson(jsonData);

  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}

void NodeEditor::mergeJsonData(const QByteArray *jsonData, const bool addBarriers, const bool addGates)
{
  VeloDataParser parser(nullptr, &prefabs, &model);

  parser.mergeJson(jsonData, addBarriers, addGates);
  gateCount = parser.getGateCount();
  nodeCount = parser.getNodeCount();
  prefabCount = parser.getPrefabCount();
  splineCount = parser.getSplineCount();
}

bool NodeEditor::isEditableNode(const QModelIndex& keyIndex)
{
  const QModelIndex valueIndex = keyIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
  const PrefabData prefab = valueIndex.data(Qt::UserRole).value<PrefabData>();

  if (!valueIndex.isValid())
    return true;

  if (isStartGrid(prefab))
    return false;

  if (prefab.id > 0) {
    return ((prefab.name != "CtrlParent") &&
            (prefab.name != "ControlCurve") &&
            (prefab.name != "ControlPoint"));
  } else if (keyIndex.data() == "finish") {
    return valueIndex.data().toBool() == false;
  } else if (keyIndex.data() == "start") {
    return valueIndex.data().toBool() == false;
  }

  return true;
}

bool NodeEditor::isStartGrid(const PrefabData& prefab)
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

FilterProxyModel& NodeEditor::getFilteredModel()
{
  return filteredModel;
}

uint NodeEditor::getNodeCount() const
{
  return nodeCount;
}

uint NodeEditor::getPrefabCount() const
{
  return prefabCount;
}

uint NodeEditor::getSplineCount() const
{
  return splineCount;
}

QStandardItemModel& NodeEditor::getStandardModel()
{
  return model;
}

bool NodeEditor::isModified()
{
  return containsModifiedNode(model.invisibleRootItem());
}

uint NodeEditor::getGateCount() const
{
  return uint(model.findItems("gate", Qt::MatchRecursive, 0).size());
}

const PrefabData NodeEditor::getPrefab(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = prefabs.begin(); i != prefabs.end(); ++i)
    if (i->id == id)
      return *i;

  return PrefabData();
}

QString NodeEditor::getPrefabDesc(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = prefabs.begin(); i != prefabs.end(); ++i)
    if (i->id == id)
      return i->name + " (" + i->type + ")";

  return "";
}

const QVector<PrefabData> *NodeEditor::getPrefabData() const
{
  return &prefabs;
}

QVector<PrefabData> NodeEditor::getPrefabsInUse(bool includeNonEditable) const
{
  QMap<uint, PrefabData> prefabMap;
  const QList<QStandardItem*> prefabs = model.findItems("prefab", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  foreach (QStandardItem* child, prefabs) {
    PrefabData prefab = child->index().siblingAtColumn(NodeTreeColumns::ValueColumn).data(Qt::UserRole).value<PrefabData>();
    if ((!prefabMap.contains(prefab.id)) && (prefab.id > 0))
      if (isEditableNode(child->index()) || includeNonEditable)
        prefabMap.insert(prefab.id, prefab);
  }

  QVector<PrefabData> prefabsInUse;
  for (QMap<uint, PrefabData>::iterator i = prefabMap.begin(); i != prefabMap.end(); ++i)
    prefabsInUse.append(i.value());

  std::sort(prefabsInUse.begin(), prefabsInUse.end());

  return prefabsInUse;
}

QModelIndex NodeEditor::getRootIndex() const
{
  return model.invisibleRootItem()->index();
}

uint NodeEditor::getSceneId() const
{
  return sceneId;
}

uint NodeEditor::replacePrefabs(const QModelIndex &searchIndex, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  // Search for prefabs that match the fromPrefabId and replace them with the toPrefabId
  NodeFilter replaceIdFilter(FilterTypes::Object, FilterMethods::Is, int(fromPrefabId));
  QVector<NodeFilter*> replaceFilterList;
  replaceFilterList.append(&replaceIdFilter);

  // Return the amount of prefabs we replaced
  return replacePrefabs(search(findPrefabs(searchIndex), replaceFilterList), fromPrefabId, toPrefabId, scaling);
}

// Same Code but different variable-type
uint NodeEditor::replacePrefabs(const QModelIndexList &searchIndexList, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  // Search for prefabs that match the fromPrefabId and replace them with the toPrefabId
  NodeFilter replaceIdFilter(FilterTypes::Object, FilterMethods::Is, int(fromPrefabId));
  QVector<NodeFilter*> replaceFilterList;
  replaceFilterList.append(&replaceIdFilter);

  // Return the amount of prefabs we replaced
  return replacePrefabs(search(findPrefabs(searchIndexList), replaceFilterList), fromPrefabId, toPrefabId, scaling);
}

uint NodeEditor::replacePrefabs(const QVector<PrefabItem*>& prefabs, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  uint prefabCount = 0;

  // Get and check the target prefab
  const PrefabData toPrefab = getPrefab(toPrefabId);
  if (toPrefab.id == 0)
    return 0;

  // Replace the data if the id matches
  foreach(PrefabItem* prefab, prefabs) {
    if (prefab->getId() != fromPrefabId)
      continue;

    // Set item description
    prefab->setData(toPrefab);

    // Apply Scaling
    prefab->applyScaling(scaling);

    prefabCount++;
  }

  return prefabCount;
}

uint NodeEditor::transformPrefab(const QModelIndex& searchIndex, const ToolTypes toolType, const QVariant& value, const bool byPercent)
{
  qDebug() << "=== Transform called with Index";
  // Build a prefab vector from the searchIndex
  QVector<PrefabItem*> prefabs;
  foreach(QStandardItem* prefabItem, findPrefabs(searchIndex)) {
    PrefabItem* prefab = new PrefabItem(this);
    if (prefab->parseIndex(prefabItem->index()))
      prefabs.append(prefab);
  }

  // Return the amount of prefabs we've modified
  return transformPrefab(prefabs, toolType, value, byPercent);
}

uint NodeEditor::transformPrefab(const QModelIndexList& searchIndexList, const ToolTypes toolType, const QVariant& value, const bool byPercent)
{
  qDebug() << "=== Transform called with IndexList";
  // Build a prefab vector from the searchIndex-List
  QVector<PrefabItem*> prefabs;
  foreach(QStandardItem* prefabItem, findPrefabs(searchIndexList)) {
    PrefabItem* prefab = new PrefabItem(this);
    if (prefab->parseIndex(prefabItem->index()))
      prefabs.append(prefab);
  }

  // Return the amount of prefabs we've modified
  return transformPrefab(prefabs, toolType, value, byPercent);
}

uint NodeEditor::transformPrefab(const QVector<PrefabItem*>& prefabs, const ToolTypes toolType, const QVariant& value, const bool byPercent)
{
  qDebug() << "=== Transform called with Prefabs";
  qDebug() << "Method:" << toolType << "Value:" << value << "ByPercent:" << byPercent;
  qDebug() << ">> Prefabs:" << prefabs;

  // Check and cast the parameter
  if (prefabs.count() == 0)
    return 0;

  QVector3D transformValue;
  QVector4D rotationValue4D;
  QQuaternion rotationValue;

  switch (toolType) {
  case IncreasingPosition:
  case IncreasingScale:
    if (byPercent)
      return 0;
  [[clang::fallthrough]]; case Move:
  case Scale:
  case ReplacePosition:
  case ReplaceScaling:
  case Mirror:
    if (!value.canConvert<QVector3D>())
      return 0;

    transformValue = qvariant_cast<QVector3D>(value);
    qDebug() << "- got transform value: " << transformValue;
    break;

  case AddRotation:
  case IncreasingRotation:
    if (!value.canConvert<QQuaternion>())
      return 0;

    if (byPercent)
      return 0;

    rotationValue = qvariant_cast<QQuaternion>(value);
    qDebug() << "- got rotation value: " << rotationValue;
    break;

  case ReplaceRotation:
    if (!value.canConvert<QVector4D>())
      return 0;

    if (byPercent)
      return 0;

    rotationValue4D = qvariant_cast<QVector4D>(value);
    qDebug() << "- got rotation4D value: " << rotationValue4D;
    break;        

  default:
    return 0;
  }

  uint count = 0;
  QVector3D incValues;
  QQuaternion incRotation;

  switch (toolType) {
  case Move:
    qDebug() << "=> Move";
    foreach(PrefabItem* prefab, prefabs) {
      prefab->setPosition(byPercent ?
                            prefab->getPositionVector() * (transformValue / 100) :
                            prefab->getPositionVector() + transformValue);
      count++;
    }
    break;

  case Scale:
    qDebug() << "=> Scale";
    foreach(PrefabItem* prefab, prefabs) {
        prefab->applyScaling(byPercent ?
                               transformValue / 100 :
                               transformValue);
      count++;
    }
    break;

  case AddRotation:
    qDebug() << "=> Add Rotation";
    foreach(PrefabItem* prefab, prefabs) {
      const QQuaternion newRotation = QQuaternion(float(prefab->getRotationW()) / 1000,
                                                  float(prefab->getRotationX()) / 1000,
                                                  float(prefab->getRotationY()) / 1000,
                                                  float(prefab->getRotationZ()) / 1000) *
                                      rotationValue;
      prefab->setRotation(int(std::round(newRotation.scalar() * 1000)),
                          int(std::round(newRotation.x() * 1000)),
                          int(std::round(newRotation.y() * 1000)),
                          int(std::round(newRotation.z() * 1000)));
      count++;
    }
    break;

  case ReplacePosition:
    qDebug() << "=> Replace Position";
    foreach(PrefabItem* prefab, prefabs) {
      prefab->setPosition(byPercent ?
                            prefab->getPositionVector() * transformValue :
                            transformValue);
      count++;
    }
    break;

  case ReplaceScaling:
    qDebug() << "=> Replace Scaling";
    foreach(PrefabItem* prefab, prefabs) {
      prefab->setScaling(byPercent ?
                           prefab->getScalingVector() * transformValue :
                           transformValue);
      count++;
    }
    break;

  case ReplaceRotation:
    qDebug() << "=> Replace Rotation";
    foreach(PrefabItem* prefab, prefabs) {
      prefab->setRotation(rotationValue4D);
      count++;
    }
    break;

  case IncreasingPosition:
    qDebug() << "=> Increasing Position";
    foreach(PrefabItem* prefab, prefabs) {
      incValues += transformValue;

      prefab->setPosition(prefab->getPositionVector() + incValues);
      count++;
    }
    break;

  case IncreasingScale:
    qDebug() << "=> Increasing Scale";
    foreach(PrefabItem* prefab, prefabs) {
      incValues += transformValue;

      prefab->setScaling(prefab->getScalingVector() + incValues);
      count++;
    }
    break;

  case IncreasingRotation:
    qDebug() << "=> Increasing Rotation";
    foreach(PrefabItem* prefab, prefabs) {
      incRotation *= rotationValue;
      prefab->setRotation(prefab->getRotationQuaterion() * incRotation);
      count++;
    }
    break;

  case Mirror: {
    foreach(PrefabItem* prefab, prefabs) {
      PrefabItem dublicate(this, dublicatePrefab(prefab));
      dublicate.setPosition(dublicate.getPositionVector() * QVector3D(-1, 1, -1));
      //prefab->setRotation(prefab->getRotationQuaterion() * QQuaternion::fromEulerAngles(0, 180, 0));
      count++;
    }
    break;
  }
  default:
    return 0;
  }

  return count;
}

void NodeEditor::resetFinishGates()
{
  QList<QStandardItem*> finishKeys = model.findItems("finish", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < finishKeys.size(); ++i) {
    QModelIndex finishValueIndex = finishKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model.setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void NodeEditor::resetModifiedFlags()
{
  resetModifiedFlag(model.invisibleRootItem());
}

void NodeEditor::resetModifiedFlag(const QStandardItem* item)
{
  for (int i = 0; i < item->rowCount(); ++i)
  {
    model.setData(item->child(i, 0)->index(), true, Qt::UserRole);
    QStandardItem* child = item->child(i, 0);
    if (child->hasChildren())
      resetModifiedFlag(child);
  }
}

void NodeEditor::resetFilterMarks()
{
  QList<QStandardItem*> prefabNodes = model.findItems("prefab", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  foreach(QStandardItem* item, prefabNodes) {
    PrefabItem prefab(this);
    if (prefab.parseIndex(item->index()))
      prefab.setFilterMark(false);
  }
}

void NodeEditor::resetStartGates()
{
  QList<QStandardItem*> startKeys = model.findItems("start", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < startKeys.size(); ++i) {
    QModelIndex finishValueIndex = startKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model.setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

QVector<PrefabItem*> NodeEditor::search(const QVector<PrefabItem*>& searchItems, const QVector<NodeFilter*>& filterList) {
  QVector<PrefabItem*> initialMatchList = searchItems;
  QVector<PrefabItem*> matchList = searchItems;

  if (matchList.count() == 0 || filterList.count() == 0)
    return matchList;

  if (!(filterList.count() == 1 && filterList.first()->getFilterType() == FilterTypes::CustomIndex)) {

    // One does not simply apply the filter, but does it in a certain order :)
    applyFilterToList(matchList, filterList, FilterTypes::Object);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::PositionR);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::PositionB);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::PositionG);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::AnyPosition);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::RotationW);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::RotationX);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::RotationY);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::RotationZ);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::AnyRotation);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::AnyScaling);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::ScalingR);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::ScalingB);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::ScalingG);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::GateNo);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::IsOnSpline);
    if (matchList.count() == 0)
      return matchList;

    applyFilterToList(matchList, filterList, FilterTypes::IsDublicate, &initialMatchList);
  }

  // If the custom index filter is the only filter, we clear the matchList
  if (filterList.count() == 1 &&
      filterList.first() != nullptr &&
      filterList.first()->getFilterType() == FilterTypes::CustomIndex)
    matchList.clear();

  // Add the custom indexes if they havent been set yet
  bool matched = false;
  foreach(NodeFilter* filter, filterList) {
    // We only care about custom index filter
    if (filter->getFilterType() != FilterTypes::CustomIndex)
      continue;

    foreach(QModelIndex index, filter->getCustomIndexList()) {
      // Check if an prefab with the custom index is already present
      matched = false;
      foreach(PrefabItem* matchedItem, matchList) {
        if (matchedItem->getIndex() != index)
          continue;

        matched = true;
        break;
      }

      // If its present we skip this index
      if (matched)
        continue;

      // This index has not been matched yet.
      // Add a new prefab-item to the matches bases on the custom index
      PrefabItem* newMatch = new PrefabItem(this);
      if (!newMatch->parseIndex(index))
        continue;

      matchList.append(newMatch);
    }
  }

  return matchList;
}

QVector<PrefabItem*> NodeEditor::search(QModelIndex& index, QVector<NodeFilter*> filterList) {
  return search(findPrefabs(index), filterList);
}

QVector<PrefabItem*> NodeEditor::search(QList<QStandardItem*> prefabs, QVector<NodeFilter*> filterList) {
  QVector<PrefabItem*> matchList;
  foreach(QStandardItem* prefabItem, prefabs) {
    PrefabItem* prefab = new PrefabItem(this);
    if (prefab->parseIndex(prefabItem->index()))
      matchList.append(prefab);
  }

  return search(matchList, filterList);
}

void NodeEditor::setPrefabs(const QVector<PrefabData> value)
{
  prefabs = value;
}

void NodeEditor::setSceneId(const uint &value)
{
  sceneId = value;
}
