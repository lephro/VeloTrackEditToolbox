#include "velodataparser.h"
#include "errorcodes.h"

VeloDataParser::VeloDataParser()
{
  model = new QStandardItemModel();
}

void VeloDataParser::changeGateOrder(const uint oldGateNo, const uint newGateNo)
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

QByteArray* VeloDataParser::exportTrackDataFromModel()
{
  QJsonDocument jsonDoc;
  QJsonObject jsonObj(jsonDoc.object());

  QStandardItem* rootKeyItem = model->invisibleRootItem();

  qDebug() << "Root row count is: " << rootKeyItem->rowCount();

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

int VeloDataParser::importTrackDataToModel(const QByteArray* jsonData)
{
  model->clear();

  QStringList labels;
  labels << tr("Node") << tr("Value") << tr("Type");
  model->setHorizontalHeaderLabels(labels);

  QStandardItem *rootItem = model->invisibleRootItem();

  doc = new QJsonDocument(QJsonDocument::fromJson(*jsonData));
  if (doc->isNull())
    return ERROR_COULD_NOT_PARSE_TRACK;

  QJsonObject* jsonRootObject = new QJsonObject(doc->object());
  importJsonObject(rootItem, *jsonRootObject);

  return 0;
}

QJsonDocument* VeloDataParser::getDoc() const
{
  return doc;
}

int VeloDataParser::getGateCount() const
{
  return model->findItems("gate", Qt::MatchRecursive, 0).size();
}

Prefab VeloDataParser::getPrefab(const uint id) const
{
  for (QVector<Prefab>::iterator i = prefabs->begin(); i != prefabs->end(); ++i)
    if (i->id == id)
      return *i;

  return *new Prefab;
}

QString VeloDataParser::getPrefabDesc(const uint id) const
{
  for (QVector<Prefab>::iterator i = prefabs->begin(); i != prefabs->end(); ++i)
    if (i->id == id)
      return i->name + " (" + i->type + ")";

  return "";
}

QVector<Prefab>* VeloDataParser::getPrefabs() const
{
  return prefabs;
}

QVector<Prefab>* VeloDataParser::getPrefabsInUse() const
{
  QMap<uint, Prefab> prefabMap;
  QList<QModelIndex> prefabs = findPrefabs(model->invisibleRootItem()->index());
  foreach (QModelIndex childIndex, prefabs) {
    Prefab prefab = childIndex.siblingAtColumn(NodeTreeColumns::ValueColumn).data(Qt::UserRole).value<Prefab>();
    if ((!prefabMap.contains(prefab.id)) && (prefab.id > 0))
      prefabMap.insert(prefab.id, prefab);
  }

  QVector<Prefab> prefabsInUse;
  for (QMap<uint, Prefab>::iterator i = prefabMap.begin(); i != prefabMap.end(); ++i)
    prefabsInUse.append(i.value());

  std::sort(prefabsInUse.begin(), prefabsInUse.end());

  return new QVector<Prefab>(prefabsInUse);
}

QModelIndex VeloDataParser::getRootIndex() const
{
  return model->invisibleRootItem()->index();
}

uint VeloDataParser::getSceneId() const
{
  return sceneId;
}

uint VeloDataParser::replacePrefab(const QModelIndex& searchIndex, const uint fromPrefabId, const uint toPrefabId)
{
  uint prefabCount = 0;
  Prefab toPrefab = getPrefab(toPrefabId);

  QList<QModelIndex> prefabs = findPrefabs(searchIndex);
  foreach (QModelIndex childIndex, prefabs) {
    QModelIndex valueIndex = childIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
    Prefab prefab = valueIndex.data(Qt::UserRole).value<Prefab>();
    if (prefab.id == fromPrefabId) {
      model->setData(valueIndex, toPrefab.name, Qt::EditRole);
      QVariant var;
      var.setValue(toPrefab);
      model->setData(valueIndex, var, Qt::UserRole);
      QModelIndex parentKeyIndex = childIndex.parent();
      if (parentKeyIndex.data(Qt::DisplayRole).toString() == prefab.name) {
        model->setData(parentKeyIndex, toPrefab.name, Qt::EditRole);
      }
      prefabCount++;
    }
  }

  return prefabCount;
}

void VeloDataParser::resetFinishGates()
{
  QList<QStandardItem*> finishKeys = model->findItems("finish", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < finishKeys.size(); ++i) {
    QModelIndex finishValueIndex = finishKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model->setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void VeloDataParser::resetStartGates()
{
  QList<QStandardItem*> startKeys = model->findItems("start", Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
  for (int i = 0; i < startKeys.size(); ++i) {
    QModelIndex finishValueIndex = startKeys.value(i)->index().siblingAtColumn(NodeTreeColumns::ValueColumn);
    if (finishValueIndex.isValid()) {
      model->setData(finishValueIndex, QVariant(false), Qt::EditRole);
    }
  }
}

void VeloDataParser::setSceneId(const uint &value)
{
  sceneId = value;
}

void VeloDataParser::setPrefabs(QVector<Prefab> *value)
{
  prefabs = value;
}

QStandardItemModel *VeloDataParser::getModel() const
{
  return model;
}

QJsonArray* VeloDataParser::exportToDataArray(const QStandardItem* treeItem)
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

QJsonObject* VeloDataParser::exportToObject(const QStandardItem* treeItem)
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
        Prefab prefab = valueItem->data(Qt::UserRole).value<Prefab>();
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

QList<QModelIndex> VeloDataParser::findPrefabs(const QModelIndex& keyItemIndex) const
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

QString VeloDataParser::getQJsonValueTypeString(const QJsonValue::Type type) const
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

void VeloDataParser::importJsonArray(QStandardItem* parentItem, const QJsonArray& dataArray)
{
  for (int i = 0; i < dataArray.size(); ++i) {
    QJsonValue jsonValue = dataArray.at(i);
    QStandardItem* itemKey = new QStandardItem();
    QStandardItem* itemValue = new QStandardItem();
    QStandardItem* itemType = new QStandardItem();
    itemKey->setText(QString("[%1]").arg(i, 0, 10));
    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);
    itemType->setText(getQJsonValueTypeString(jsonValue.type()));

    itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
    if (itemValue->data(Qt::EditRole).toString() == "") {
      itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
    }
    itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

    QList<QStandardItem*> childColumns;
    childColumns << itemKey << itemValue << itemType;
    parentItem->appendRow(childColumns);

    if (jsonValue.type() == QJsonValue::Object)
      importJsonObject(itemKey, jsonValue.toObject());
  }
}

void VeloDataParser::importJsonObject(QStandardItem* parentItem, const QJsonObject& dataObject)
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
        Prefab prefab = getPrefab(jsonValue.toVariant().toUInt());
        itemValue->setData(prefab.name, Qt::DisplayRole);
        QVariant var;
        var.setValue(prefab);
        itemValue->setData(var, Qt::UserRole);
      }
    }
    itemType->setText(getQJsonValueTypeString(jsonValue.type()));

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
      importJsonObject(itemKey, jsonValue.toObject());
    else if (jsonValue.type() == QJsonValue::Array) {
      importJsonArray(itemKey, jsonValue.toArray());
    }
  }
}

