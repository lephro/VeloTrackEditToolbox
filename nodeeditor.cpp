#include "nodeeditor.h"

NodeEditor::NodeEditor(Track& track)
  : track(&track),
    treeView(new QTreeView())
{
  track.setParent(this);

  editorModel = new EditorModel(&track, this);

  filteredModel.setRecursiveFilteringEnabled(true);
  filteredModel.setSourceModel(editorModel);
}

void NodeEditor::beginNodeEdit()
{
  qDebug() << "NodeEditor::beginNodeEdit() Started: " << editStarted;
//  // Everytime we start to do a lot of changes to the tree, we memorize if the root node was expanded
//  // and collapse it for performance reasons (so any redraw / update call is processed way faster)
//  if (editStarted)
//    return;

//  editStarted = true;

//  if (treeView->verticalScrollBar() != nullptr && treeView->verticalScrollBar()->maximum() > 0)
//    lastScrollbarPos = float(treeView->verticalScrollBar()->value()) / float(treeView->verticalScrollBar()->maximum());

//  QModelIndex index;
//  for (int i = 0; i < editorModel->getRootItem()->childCount(); ++i) {
//    index = filteredModel.mapFromSource(editorModel->getRootItem()->child(i)->getIndex());
//    lastTreeExpansionStates.append(index.isValid() && treeView->isExpanded(index));
//    treeView->collapse(index);
//  }
}

void NodeEditor::applyFilterToList(QVector<EditorObject*>& matchingItems, const QVector<NodeFilter*>& filter, const FilterTypes filterType, const QVector<EditorObject*>* initalItems)
{
  // Define our control variables outside of the loops to prevent excessive alloc and deallocs
  bool match = false;
  int rowCount = 1;
  QString filterValue = "";
  QString prefabValue = "";
  NodeFilter* nodeFilter = nullptr;

  // Go through every filter in the list  and eventually remove all items that dont apply to it
  for (int i = 0; i < filter.count(); ++i) {
    nodeFilter = filter.at(i);

    if (nodeFilter == nullptr || nodeFilter->getFilterType() != filterType)
      continue;

    filterValue = QString("%1").arg(nodeFilter->getFilterValue());

    // Every filter is applied to every remaining item in the list
    for (int item = 0; item < matchingItems.count(); ++item) {
      match = false;

      EditorObject* prefab = matchingItems.at(item);

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
          for(int i = 0; i < initalItems->count(); ++i) {
            const EditorObject* item = initalItems->at(i);
            if (item->isSpline() || item->isOnSpline() || item->isSplineControl())
              continue;

            if (item->getIndex() != prefab->getIndex() && item == prefab)
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

  QVector<EditorObject*> gates = getTrack()->getGates();
  foreach(EditorObject* gate, gates) {
    const int gateNo = gate->getGateNo();
    if (shiftLeft && (gateNo >= int(newGateNo)) && (gateNo < int(oldGateNo)))
      gate->setGateNo(gateNo + 1, false);
    else if (!shiftLeft && (gateNo > int(oldGateNo)) && (gateNo <= int(newGateNo)))
      gate->setGateNo(gateNo - 1, false);
  }
}

void NodeEditor::clearFilterMarks()
{
//  QList<QStandardItem*> prefabNodes = model.findItems("prefab", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
//  foreach(QStandardItem* item, prefabNodes) {
//    PrefabItem prefab(this);
//    if (prefab.parseIndex(item->index()))
//      prefab.setFilterMark(false);
//  }
  for(int resultIdx = 0; resultIdx < searchResult.count(); ++resultIdx)
  {
    searchResult[resultIdx]->setFilterMarked(false);
  }
}

void NodeEditor::clearModifiedFlag(EditorModelItem* modelItem)
{
  EditorModelItem* item = modelItem;
  if (!item)
    item = editorModel->getRootItem();

  item->setModified(false);
}

void NodeEditor::clearSearch(const int cacheId)
{  
  searchCacheId = cacheId;
  clearFilterMarks();
  searchResult.clear();
}

void NodeEditor::deleteNode(const QModelIndex &index) const
{
  if (!index.isValid())
    return;

  EditorModelItem* parentItem = editorModel->itemFromIndex(index.parent());
  if (parentItem == nullptr)
    return;

  parentItem->removeChild(index.row());
}

QModelIndex NodeEditor::duplicateObject(EditorObject* sourceObject)
{
  EditorObject* newObject = new EditorObject(*sourceObject);
  EditorModelItem* newItem = new EditorModelItem(newObject);
  EditorModelItem* parent = sourceObject->getParentModelItem();
  if (!parent)
    return QModelIndex();

  parent->addChild(*newItem);

  // Prevent invalid gate data
  if (newObject->isValid()) {
    if (newObject->isGate()) {
      newObject->setGateNo(int(gateCount));
      gateCount++;
    }
    newObject->setFinish(false);
    newObject->setStart(false);
    if (newObject->isSpline())
      splineCount++;
  }

  prefabCount++;

  return editorModel->index(newItem->childNumber(), 0, parent->getIndex());


//  // Clone the source columns and their children
//  QStandardItem* newItemKey = sourceItemKey->clone();
//  duplicateChildren(sourceItemKey, newItemKey);
//  QStandardItem* newItemValue = sourceItemValue->clone();
//  QStandardItem* newItemType = sourceItemType->clone();

//  // Create a new row and append it to the parent of the source
//  QList<QStandardItem*> row;
//  row << newItemKey << newItemValue << newItemType;

//  QStandardItem* parent = sourceItemKey->parent();
//  if (parent == nullptr)
//    return QModelIndex();

//  parent->appendRow(row);

//  // Prevent invalid gate data
//  EditorObject* object = getObjectByIndex(newItemKey->index());
//  if (object->isValid()) {
//    if (object->isGate()) {
//      object->setGateNo(int(gateCount));
//      gateCount++;
//    }
//    object->setFinish(false);
//    object->setStart(false);
//    if (object->isSpline())
//      splineCount++;
//  }

//  prefabCount++;

//  return sourceItemKey->index();
}

void NodeEditor::endNodeEdit()
{
//  qDebug() << "NodeEditor::endNodeEdit() Started: " << editStarted;

//  // Once we have finished our changes we restore the original view and move the scrollbar to its previous location
//  if (!editStarted)
//    return;

//  editStarted = false;

//  QModelIndex index;
//  for (int i = 0; i < editorModel->getRootItem()->childCount(); ++i) {
//    if (i >= lastTreeExpansionStates.count())
//      break;

//    index = filteredModel.mapFromSource(editorModel->getRootItem()->child(i)->getIndex());
//    if (lastTreeExpansionStates[i])
//      treeView->expand(index);
//  }

//  if (treeView->verticalScrollBar() != nullptr && treeView->verticalScrollBar()->maximum() > 0)
//    treeView->verticalScrollBar()->setValue(int(std::round(lastScrollbarPos * treeView->verticalScrollBar()->maximum())));

//  lastTreeExpansionStates.clear();
}

QByteArray *NodeEditor::exportAsJsonData()
{
  VeloDataParser parser(editorModel);
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


void NodeEditor::mergeJsonData(const QByteArray& jsonData, const bool addBarriers, const bool addGates)
{
//  VeloDataParser parser;

//  //parser.mergeJson(jsonData, addBarriers, addGates);
//  gateCount = parser.getGateCount();
//  nodeCount = parser.getNodeCount();
//  prefabCount = parser.getPrefabCount();
//  splineCount = parser.getSplineCount();
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

Track* NodeEditor::getTrack() const
{
  return track;
}

void NodeEditor::setTrack(Track* newTrack)
{
  track = newTrack;
}

QTreeView& NodeEditor::getTreeView() const
{
  return *treeView;
}

TrackData& NodeEditor::getTrackData()
{
  return track->getTrackData();
}

void NodeEditor::setTrackData(const TrackData &value)
{
  track->setTrackData(value);
}

QVector<EditorObject*> NodeEditor::getSearchResult() const
{
  return searchResult;
}

void NodeEditor::setSearchResult(const int cacheId, const QVector<EditorObject*> &value)
{
  searchCacheId = cacheId;
  searchResult = value;
}

int NodeEditor::getSearchCacheId() const
{
    return searchCacheId;
}

void NodeEditor::setFilterMarks()
{
  if (searchResult.isEmpty())
    return;

  foreach(EditorObject* item, searchResult) {
    item->setFilterMarked();
  }
}

FilterProxyModel& NodeEditor::getFilteredModel()
{
  return filteredModel;
}

EditorObject* NodeEditor::getObjectByIndex(const QModelIndex index)
{
  const QModelIndex sourceIndex = filteredModel.mapToSource(index);
  const EditorModelItem* item = editorModel->itemFromIndex(sourceIndex);
  if (!item->hasObject())
    return nullptr;

  return item->getObject();
}

EditorModel& NodeEditor::getEditorModel()
{
  return *editorModel;
}

bool NodeEditor::isModified()
{
  foreach(EditorObject* object, track->getObjects()) {
    if (object->isModified())
      return true;
  }

  return false;
}

const PrefabData NodeEditor::getPrefabData(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = track->getAvailablePrefabs().begin(); i != track->getAvailablePrefabs().end(); ++i)
    if (i->id == id)
      return *i;

  return PrefabData();
}

QString NodeEditor::getPrefabDesc(const uint id) const
{
  for (QVector<PrefabData>::const_iterator i = track->getAvailablePrefabs().begin(); i != track->getAvailablePrefabs().end(); ++i)
    if (i->id == id)
      return i->name + " (" + i->type + ")";

  return "";
}

QVector<PrefabData> NodeEditor::getAllPrefabData() const
{
  return track->getAvailablePrefabs();
}

QVector<PrefabData> NodeEditor::getPrefabsInUse(bool includeNonEditable) const
{
  QMap<uint, PrefabData> prefabMap;
  foreach (EditorObject* object, track->getObjects().toList()) {
    if (includeNonEditable) {
      if (object->getSplineParents().count() > 0) {
        const PrefabData& prefab = object->getSplineParents().first()->getData();
        prefabMap.insert(prefab.id, prefab);
      }
      if (object->getSplineControls().count() > 0) {
        const PrefabData& prefab = object->getSplineControls().first()->getData();
        prefabMap.insert(prefab.id, prefab);
      }
    }
    foreach(EditorObject* splineObject, object->getSplineObjects())
    {
      if (includeNonEditable || splineObject->isEditable())
        prefabMap.insert(splineObject->getData().id, splineObject->getData());
    }

    if (includeNonEditable || object->isEditable())
      prefabMap.insert(object->getData().id, object->getData());
  }

  QVector<PrefabData> prefabsInUse;
  for (QMap<uint, PrefabData>::iterator i = prefabMap.begin(); i != prefabMap.end(); ++i)
    prefabsInUse.append(i.value());

  std::sort(prefabsInUse.begin(), prefabsInUse.end());

  return prefabsInUse;
}

QModelIndex NodeEditor::getRootIndex() const
{
  return editorModel->getRootItem()->getIndex();
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
  return replacePrefabs(search(editorModel->itemsFromIndex(searchIndex), replaceFilterList), fromPrefabId, toPrefabId, scaling);
}

// Same Code but different variable-type
uint NodeEditor::replacePrefabs(const QModelIndexList &searchIndexList, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  // Search for prefabs that match the fromPrefabId and replace them with the toPrefabId
  NodeFilter replaceIdFilter(FilterTypes::Object, FilterMethods::Is, int(fromPrefabId));
  QVector<NodeFilter*> replaceFilterList;
  replaceFilterList.append(&replaceIdFilter);

  // Return the amount of prefabs we replaced
  return replacePrefabs(search(editorModel->itemsFromIndexList(searchIndexList), replaceFilterList), fromPrefabId, toPrefabId, scaling);
}

uint NodeEditor::replacePrefabs(const QVector<EditorObject*>& prefabs, const uint fromPrefabId, const uint toPrefabId, const QVector3D scaling)
{
  uint prefabCount = 0;

  const bool doScaling = scaling != QVector3D(1, 1, 1);

  // Get and check the target prefab
  const PrefabData toPrefab = getPrefabData(toPrefabId);
  if (toPrefab.id == 0)
    return 0;

  // Replace the data if the id matches
  foreach(EditorObject* prefab, prefabs) {
    if (prefab->getId() != fromPrefabId)
      continue;

    // Increase the counter if the changing was successfull.
    if (prefab->setData(toPrefab) &&
        (!doScaling || prefab->applyScaling(scaling)))
      prefabCount++;
  }

  return prefabCount;
}

uint NodeEditor::transformPrefab(const QModelIndex& searchIndex, const ToolTypes toolType, const QVariant& value, const ToolTypeTargets target, const bool byPercent)
{
  qDebug() << "=== Transform called with Index";

  // Build a prefab vector from the searchIndex
  QVector<EditorObject*> objects;
  EditorObject* object = getObjectByIndex(searchIndex);
  if (object->isValid())
    objects.append(object);

  // Return the amount of prefabs we've modified
  return transformPrefab(objects, toolType, value, target, byPercent);
}

uint NodeEditor::transformPrefab(const QModelIndexList& searchIndexList, const ToolTypes toolType, const QVariant& value, const ToolTypeTargets target, const bool byPercent)
{
  //qDebug() << "=== Transform called with IndexList";
  // Build a prefab vector from the searchIndex-List
  QVector<EditorObject*> objects;
  EditorObject* object;
  foreach(QModelIndex index, searchIndexList) {
    object = getObjectByIndex(index);
    if (object->isValid())
      objects.append(object);
  }

  // Return the amount of prefabs we've modified
  return transformPrefab(objects, toolType, value, target, byPercent);
}

uint NodeEditor::transformPrefab(const QVector<EditorObject*>& objects, const ToolTypes toolType, const QVariant& value, const ToolTypeTargets target, const bool byPercent)
{
  //qDebug() << "=== Transform called with objects";
  //qDebug() << "Method:" << toolType << "Value:" << value << "Target:" << target << "ByPercent:" << byPercent;
  //qDebug() << ">> objects:" << objects;

  // Check and cast the parameter
  if (objects.count() == 0)
    return 0;

  QVector3D transformValue;
  QVector4D rotationValue4D;
  QQuaternion rotationValue;

  switch (toolType) {
  case IncreasingPosition:
  case IncreasingScale:
    if (byPercent)
      return 0;
  [[clang::fallthrough]];
  case Move:
  case Scale:
  case ReplacePosition:
  case ReplaceScaling:
  case MultiplyPosition:
  case MultiplyScaling:
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

    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setPositionR(byPercent ?
                               object->getPositionR() + int(std::round(object->getPositionR() * (transformValue.x() / 100))) :
                               object->getPositionR() + int(std::round(transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setPositionG(byPercent ?
                               object->getPositionG() + int(std::round(object->getPositionG() * (transformValue.y() / 100))) :
                               object->getPositionG() + int(std::round(transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setPositionB(byPercent ?
                               object->getPositionB() + int(std::round(object->getPositionB() * (transformValue.z() / 100))) :
                               object->getPositionB() + int(std::round(transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
        object->setPosition(byPercent ?
                              object->getPositionVector() + (object->getPositionVector() * (transformValue / 100)) :
                              object->getPositionVector() + transformValue);
        count++;
      }
      break;
    }
    break;

  case Scale:
    qDebug() << "=> Scale";

    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setScalingR(byPercent ?
                               object->getScalingR() + int(std::round(object->getScalingR() * (transformValue.x() / 100))) :
                               object->getScalingR() + int(std::round(transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setScalingG(byPercent ?
                               object->getScalingG() + int(std::round(object->getScalingG() * (transformValue.y() / 100))) :
                               object->getScalingG() + int(std::round(transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setScalingB(byPercent ?
                               object->getScalingB() + int(std::round(object->getScalingB() * (transformValue.z() / 100))) :
                               object->getScalingB() + int(std::round(transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
        if (byPercent) {
          object->applyScaling(transformValue / 100);
        }
        else
          object->setScaling(transformValue);

        count++;
      }
      break;
    }

    break;

  case AddRotation:
    qDebug() << "=> Add Rotation";
    foreach(EditorObject* object, objects) {
      const QQuaternion newRotation = QQuaternion(float(object->getRotationW()) / 1000,
                                                  float(object->getRotationX()) / 1000,
                                                  float(object->getRotationY()) / 1000,
                                                  float(object->getRotationZ()) / 1000) *
                                      rotationValue;
      object->setRotation(int(std::round(newRotation.scalar() * 1000)),
                          int(std::round(newRotation.x() * 1000)),
                          int(std::round(newRotation.y() * 1000)),
                          int(std::round(newRotation.z() * 1000)));
      count++;
    }
    break;

  case ReplacePosition:
    qDebug() << "=> Replace Position";

    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setPositionR(byPercent ?
                               int(std::round(object->getPositionR() * transformValue.x() / 100)) :
                               int(std::round(transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setPositionG(byPercent ?
                               int(std::round(object->getPositionG() * transformValue.y() / 100)) :
                               int(std::round(transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setPositionB(byPercent ?
                               int(std::round(object->getPositionB() * transformValue.z() / 100)) :
                               int(std::round(transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
        object->setPosition(byPercent ?
                              object->getPositionVector() * (transformValue / 100) :
                              transformValue);
        count++;
      }
      break;
    }
    break;

  case ReplaceScaling:
    qDebug() << "=> Replace Scaling";
    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setScalingR(byPercent ?
                               int(std::round(object->getScalingR() * transformValue.x() / 100)) :
                               int(std::round(transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setScalingG(byPercent ?
                               int(std::round(object->getScalingG() * transformValue.y() / 100)) :
                               int(std::round(transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setScalingB(byPercent ?
                               int(std::round(object->getScalingB() * transformValue.z() / 100)) :
                               int(std::round(transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
          object->setScaling(byPercent ?
                               object->getScalingVector() * transformValue / 100 :
                               transformValue);
        count++;
      }
      break;
    }
    break;

  case ReplaceRotation:
    qDebug() << "=> Replace Rotation";
    foreach(EditorObject* object, objects) {
      object->setRotation(rotationValue4D);
      count++;
    }
    break;

  case IncreasingPosition:
    qDebug() << "=> Increasing Position";
    foreach(EditorObject* object, objects) {
      incValues += transformValue;

      object->setPosition(object->getPositionVector() + incValues);
      count++;
    }
    break;

  case IncreasingScale:
    qDebug() << "=> Increasing Scale";
    foreach(EditorObject* object, objects) {
      incValues += transformValue;

      object->setScaling(object->getScalingVector() + incValues);
      count++;
    }
    break;

  case IncreasingRotation:
    qDebug() << "=> Increasing Rotation";
    foreach(EditorObject* object, objects) {
      incRotation *= rotationValue;
      object->setRotation(object->getRotationQuaterion() * incRotation);
      count++;
    }
    break;

  case MultiplyPosition:
    qDebug() << "=> Multiply Position";

    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setPositionR(byPercent ?
                               int(std::round(object->getPositionR() * transformValue.x() / 100)) :
                               int(std::round(object->getPositionR() * transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setPositionG(byPercent ?
                               int(std::round(object->getPositionG() * transformValue.y() / 100)) :
                               int(std::round(object->getPositionG() * transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setPositionB(byPercent ?
                               int(std::round(object->getPositionB() * transformValue.z() / 100)) :
                               int(std::round(object->getPositionB() * transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
        object->setPosition(byPercent ?
                              object->getPositionVector() * (transformValue / 100) :
                              object->getPositionVector() * transformValue);
        count++;
      }
      break;
    }
    break;

  case MultiplyScaling:
    qDebug() << "=> Replace Scaling";
    switch(target) {
    case ToolTypeTargets::R:
      foreach(EditorObject* object, objects) {
        object->setScalingR(byPercent ?
                               int(std::round(object->getScalingR() * transformValue.x() / 100)) :
                               int(std::round(object->getScalingR() * transformValue.x())));
        count++;
      }
      break;

    case ToolTypeTargets::G:
      foreach(EditorObject* object, objects) {
        object->setScalingG(byPercent ?
                               int(std::round(object->getScalingG() * transformValue.y() / 100)) :
                               int(std::round(object->getScalingG() * transformValue.y())));
        count++;
      }
      break;

    case ToolTypeTargets::B:
      foreach(EditorObject* object, objects) {
        object->setScalingB(byPercent ?
                               int(std::round(object->getScalingB() * transformValue.z() / 100)) :
                               int(std::round(object->getScalingB() * transformValue.z())));
        count++;
      }
      break;

    default:
      foreach(EditorObject* object, objects) {
          object->setScaling(byPercent ?
                               object->getScalingVector() * transformValue / 100 :
                               object->getScalingVector() * transformValue);
        count++;
      }
      break;
    }
    break;

  case Mirror: {
    foreach(EditorObject* object, objects) {
      EditorObject* duplicate = getObjectByIndex(duplicateObject(object));
      duplicate->setPosition(duplicate->getPositionVector() * QVector3D(-1, 1, -1));
      //object->setRotation(object->getRotationQuaterion() * QQuaternion::fromEulerAngles(0, 180, 0));
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
  foreach(EditorObject* gate, track->getGates()) {
    gate->setFinish(false);
  }
}

void NodeEditor::resetStartGates()
{
  foreach(EditorObject* gate, track->getGates()) {
    gate->setStart(false);
  }
}

QVector<EditorObject*> NodeEditor::search(const QVector<EditorObject*>& searchItems, const QVector<NodeFilter*>& filterList) {
  qDebug() << "NodeEditor::search called.";
  QVector<EditorObject*> initialMatchList = searchItems;
  QVector<EditorObject*> matchList = searchItems;

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
      // Check if an object with the custom index is already present
      matched = false;
      foreach(EditorObject* matchedItem, matchList) {
        if (matchedItem->getIndex() != index)
          continue;

        if (index.model() != editorModel)
          continue;

        matched = true;
        break;
      }

      // If its present we skip this index
      if (matched)
        continue;

      // This index has not been matched yet.
      // Add a new prefab-item to the matches based on the custom index
      EditorObject* object = getObjectByIndex(index);
      if (!object->isValid())
        continue;

      matchList.append(object);
    }
  }

  return matchList;
}

//QVector<EditorObject*> NodeEditor::search(QModelIndex& index, QVector<NodeFilter*> filterList) {
//  return search(findPrefabs(index), filterList);
//}

QVector<EditorObject*> NodeEditor::search(QList<EditorModelItem*> objects, QVector<NodeFilter*> filterList) {
  QVector<EditorObject*> matchList;
  foreach(EditorModelItem* objectItem, objects) {
    EditorObject* object = objectItem->getObject();
    if (object && object->isValid())
      matchList.append(object);
  }

  return search(matchList, filterList);
}

void NodeEditor::setSceneId(const uint &value)
{
  sceneId = value;
}
