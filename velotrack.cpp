#include "velotrack.h"
#include "velotrack.h"

VeloTrack::VeloTrack()
{
  model = new QStandardItemModel();
}

VeloTrack::~VeloTrack()
{
  delete model;
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

bool VeloTrack::isStartGrid(PrefabData& prefab)
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

QByteArray* VeloTrack::exportAsJsonData()
{
  QJsonDocument jsonDoc;
  QJsonObject jsonObj(jsonDoc.object());

  QStandardItem* rootKeyItem = model->invisibleRootItem();

  for(int i = 0; i < rootKeyItem->rowCount(); ++i) {
    QModelIndex index = model->index(i, NodeTreeColumns::KeyColumn, rootKeyItem->index());
    QStandardItem* keyItem = rootKeyItem->child(index.row(), NodeTreeColumns::KeyColumn);
    QStandardItem* valueItem = rootKeyItem->child(index.row(), NodeTreeColumns::ValueColumn);
    QStandardItem* typeItem = rootKeyItem->child(index.row(), NodeTreeColumns::TypeColumn);

    if (typeItem->text() == "Array") {
      jsonObj.insert(keyItem->text(), *exportToDataArray(keyItem));
    } else if (typeItem->text() == "Object") {
      jsonObj.insert(keyItem->text(), *exportToObject(keyItem));
    } else {
      jsonObj.insert(keyItem->text(), valueItem->text());
    }
  }

  jsonDoc.setObject(jsonObj);
  return new QByteArray(jsonDoc.toJson(QJsonDocument::Compact));
}

void VeloTrack::importJsonData(const QByteArray* jsonData)
{
  model->clear();

  QStringList labels;
  labels << tr("Node") << tr("Value") << tr("Type");
  model->setHorizontalHeaderLabels(labels);

  QStandardItem* rootItem = model->invisibleRootItem();

  QJsonDocument doc = QJsonDocument(QJsonDocument::fromJson(*jsonData));
  if (doc.isNull())
    throw TrackWithoutNodesException();

  QJsonObject* jsonRootObject = new QJsonObject(doc.object());

  importJsonObject(rootItem, *jsonRootObject);

  if (rootItem->rowCount() == 0)
    throw TrackWithoutNodesException();
}

bool VeloTrack::isModified()
{
  return isModifiedNode(model->invisibleRootItem());
}

void VeloTrack::mergeJsonData(const QByteArray* jsonData,
                              const bool addBarriers,
                              const bool addGates)
{
  QStandardItem* rootItem = model->invisibleRootItem();

  QJsonDocument doc = QJsonDocument(QJsonDocument::fromJson(*jsonData));
  if (doc.isNull())
    throw TrackWithoutNodesException();

  QJsonObject* jsonRootObject = new QJsonObject(doc.object());

  QStringList keyList = jsonRootObject->keys();
  for (int i = 0; i < keyList.size(); ++i) {
    if (!addGates && (keyList.at(i) == "gates"))
      continue;

    if (!addBarriers && (keyList.at(i) == "barriers"))
      continue;

    QList<QStandardItem*> foundItems = model->findItems(keyList.at(i));
    if (foundItems.count() > 0) {
      if (foundItems.first()->text() == "weather")
        continue;

      QJsonValue jsonValue = jsonRootObject->value(keyList.at(i));
      if(jsonValue.type() == QJsonValue::Object)
        importJsonObject(foundItems.first(), jsonValue.toObject(), getGateCount(), true);
      else if (jsonValue.type() == QJsonValue::Array) {
        importJsonArray(foundItems.first(), jsonValue.toArray(), getGateCount(), true);
      }
    } else {
      QJsonValue jsonValue = jsonRootObject->value(keyList.at(i));
      QStandardItem* itemKey = new QStandardItem();
      QStandardItem* itemValue = new QStandardItem();
      QStandardItem* itemType = new QStandardItem();

      itemKey->setData(keyList.at(i), Qt::DisplayRole);
      itemValue->setData(jsonValue.toVariant(), Qt::EditRole);

      if (keyList.at(i) == "prefab") {
        uint prefabId = jsonValue.toVariant().toUInt();
        if (prefabId > 0 ) {
          PrefabData prefab = getPrefab(jsonValue.toVariant().toUInt());
          itemValue->setData(prefab.name, Qt::DisplayRole);
          QVariant var;
          var.setValue(prefab);
          itemValue->setData(var, Qt::UserRole);
        }
      }

      itemType->setText(getJsonValueTypeString(jsonValue.type()));

      itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
      if (itemValue->data(Qt::DisplayRole).toString() == "")
        itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
      itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

      QList<QStandardItem*> childColumns;
      childColumns << itemKey << itemValue << itemType;
      rootItem->appendRow(childColumns);

      if (itemKey->text() == "prefab") {
        if (itemValue->text() != "")
          rootItem->setText(itemValue->text());
        else
        rootItem->setText(itemKey->text());
      }

      if(jsonValue.type() == QJsonValue::Object)
        importJsonObject(itemKey, jsonValue.toObject(), getGateCount());
      else if (jsonValue.type() == QJsonValue::Array) {
        importJsonArray(itemKey, jsonValue.toArray(), getGateCount());
      }
    }
  }

  if (rootItem->rowCount() == 0)
    throw TrackWithoutNodesException();
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

QJsonArray* VeloTrack::exportToDataArray(const QStandardItem* treeItem)
{
  QJsonArray* array = new QJsonArray();
  for(int i = 0; i < treeItem->rowCount(); ++i) {
    QModelIndex index = model->index(i, NodeTreeColumns::KeyColumn, treeItem->index());
    QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
    QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
    QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);
    if (typeItem->text() == "Array") {
      array->append(*exportToDataArray(keyItem));
    } else if (typeItem->text() == "Bool") {
      array->append(valueItem->data(Qt::EditRole).toBool());
    } else if (typeItem->text() == "Double") {
      array->append(valueItem->data(Qt::EditRole).toDouble());
    } else if (typeItem->text() == "Object") {
      array->append(*exportToObject(keyItem));
    } else {
      array->append(keyItem->text());
    }
  }

  return array;
}

QJsonObject* VeloTrack::exportToObject(const QStandardItem* treeItem)
{
  QJsonObject* object = new QJsonObject();
  for(int i = 0; i < treeItem->rowCount(); ++i) {
    QModelIndex index = model->index(i, NodeTreeColumns::KeyColumn, treeItem->index());
    QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
    QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
    QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);
    if (typeItem->text() == "Array") {
      object->insert(keyItem->text(),* exportToDataArray(treeItem->child(i)));
    } else if (typeItem->text() == "Bool") {
      object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toBool());
    } else if (typeItem->text() == "Double") {
      if (keyItem->text() == "prefab") {
        PrefabData prefab = valueItem->data(Qt::UserRole).value<PrefabData>();
        if (prefab.id > 0)
          object->insert(keyItem->text(), int(prefab.id));
      } else
        object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toDouble());
    } else if (typeItem->text() == "Object") {
      object->insert(keyItem->text(),* exportToObject(treeItem->child(i)));
    } else {
      object->insert(keyItem->text(), valueItem->text());
    }
  }

  return object;
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

QString VeloTrack::getJsonValueTypeString(const QJsonValue::Type type) const
{
  switch (type) {
  case QJsonValue::Type::Null:
    return tr("Null");
  case QJsonValue::Type::Bool:
    return tr("Bool");
  case QJsonValue::Type::Double:
    return tr("Double");
  case QJsonValue::Type::String:
    return tr("String");
  case QJsonValue::Type::Array:
    return tr("Array");
  case QJsonValue::Type::Object:
    return tr("Object");
  case QJsonValue::Type::Undefined:
    return tr("Undefined");
  }
  return "";
}

void VeloTrack::importJsonArray(QStandardItem* parentItem, const QJsonArray& dataArray, const uint gateOffset, const bool skipStartgrid)
{
  for (int i = 0; i < dataArray.size(); ++i) {
    QJsonValue jsonValue = dataArray.at(i);
    QStandardItem* itemKey = new QStandardItem();
    QStandardItem* itemValue = new QStandardItem();
    QStandardItem* itemType = new QStandardItem();
    itemKey->setText(QString("[%1]").arg(i, 0, 10));
    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);
    itemType->setText(getJsonValueTypeString(jsonValue.type()));

    itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
    if (itemValue->data(Qt::EditRole).toString() == "") {
      itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
    }
    itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

    QList<QStandardItem*> childColumns;
    childColumns << itemKey << itemValue << itemType;
    parentItem->appendRow(childColumns);

    if (jsonValue.type() == QJsonValue::Object)
      importJsonObject(itemKey, jsonValue.toObject(), gateOffset, skipStartgrid);
  }
}

void VeloTrack::importJsonObject(QStandardItem* parentItem, const QJsonObject& dataObject, const uint gateOffset, const bool skipStartgrid)
{
  QStringList keyList = dataObject.keys();
  for (int i = 0; i < keyList.size(); ++i) {
    QJsonValue jsonValue = dataObject.value(keyList.at(i));
    QStandardItem* itemKey = new QStandardItem();
    QStandardItem* itemValue = new QStandardItem();
    QStandardItem* itemType = new QStandardItem();

    itemKey->setData(keyList.at(i), Qt::DisplayRole);
    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);

    if (keyList.at(i) == "prefab") {
      uint prefabId = jsonValue.toVariant().toUInt();
      if (prefabId > 0 ) {
        PrefabData prefab = getPrefab(jsonValue.toVariant().toUInt());

        if (skipStartgrid && isStartGrid(prefab)) {
          model->removeRows(parentItem->row(), 1, parentItem->parent()->index());
          return;
        }

        itemValue->setData(prefab.name, Qt::DisplayRole);
        QVariant var;
        var.setValue(prefab);
        itemValue->setData(var, Qt::UserRole);
      }
    }

    if ((gateOffset > 0) && (keyList.at(i) == "gate")) {
      uint gateId = jsonValue.toVariant().toUInt() + gateOffset;
      itemValue->setData(gateId, Qt::EditRole);
    }

    itemType->setText(getJsonValueTypeString(jsonValue.type()));

    itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
    if (itemValue->data(Qt::DisplayRole).toString() == "")
      itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
    itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

    QList<QStandardItem*> childColumns;
    childColumns << itemKey << itemValue << itemType;
    parentItem->appendRow(childColumns);

    if (itemKey->text() == "prefab") {
      if (itemValue->text() != "")
        parentItem->setText(itemValue->text());
      else
        parentItem->setText(itemKey->text());
    }

    if(jsonValue.type() == QJsonValue::Object)
      importJsonObject(itemKey, jsonValue.toObject(), gateOffset);
    else if (jsonValue.type() == QJsonValue::Array) {
      importJsonArray(itemKey, jsonValue.toArray(), gateOffset);
    }
  }
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

