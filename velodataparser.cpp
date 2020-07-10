#include "velodataparser.h"

VeloDataParser::VeloDataParser(QObject* parent, QVector<PrefabData> *prefabs, QStandardItemModel* model) : QObject(parent)
{
  this->prefabs = prefabs;
  this->model = model;
}

QByteArray* VeloDataParser::exportToJson()
{
  nodeCount = 0;
  prefabCount = 0;
  splineCount = 0;
  gateCount = 0;

  QJsonDocument jsonDoc;
  QJsonObject jsonObj(jsonDoc.object());

  QStandardItem* rootKeyItem = model->invisibleRootItem();

  for(int i = 0; i < rootKeyItem->rowCount(); ++i) {
    nodeCount++;

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

uint VeloDataParser::getGateCount() const
{
  return gateCount;
}

uint VeloDataParser::getGatesInModelCount() const
{
  return uint(model->findItems("gate", Qt::MatchRecursive, 0).size());
}

uint VeloDataParser::getNodeCount() const
{
  return nodeCount;
}

uint VeloDataParser::getPrefabCount() const
{
  return prefabCount;
}


uint VeloDataParser::getSplineCount() const
{
  return splineCount;
}

PrefabData VeloDataParser::getPrefabData(const uint id) const
{
  foreach(PrefabData prefab, *prefabs)
    if (prefab.id == id)
      return prefab;

  return PrefabData();
}

void VeloDataParser::importJson(const QByteArray* jsonData)
{
  model->clear();

  nodeCount = 0;
  prefabCount = 0;
  splineCount = 0;
  gateCount = 0;

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

void VeloDataParser::mergeJson(const QByteArray* jsonData,
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

    QList<QStandardItem*> foundItems = model->findItems(keyList.at(i), Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
    if (foundItems.count() > 0) {
      if (foundItems.first()->text() == "weather")
        continue;

      QJsonValue jsonValue = jsonRootObject->value(keyList.at(i));
      if(jsonValue.type() == QJsonValue::Object)
        importJsonObject(foundItems.first(), jsonValue.toObject(), getGatesInModelCount(), true);
      else if (jsonValue.type() == QJsonValue::Array) {
        importJsonArray(foundItems.first(), jsonValue.toArray(), getGatesInModelCount(), true);
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
          PrefabData prefab = getPrefabData(jsonValue.toVariant().toUInt());
          itemValue->setData(prefab.name, Qt::DisplayRole);
          QVariant var;
          var.setValue(prefab);
          itemValue->setData(var, Qt::UserRole);
        }
      }

      itemType->setText(getJsonValueTypeAsString(jsonValue.type()));

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
        importJsonObject(itemKey, jsonValue.toObject(), getGatesInModelCount());
      else if (jsonValue.type() == QJsonValue::Array) {
        importJsonArray(itemKey, jsonValue.toArray(), getGatesInModelCount());
      }
    }
  }

  if (rootItem->rowCount() == 0)
    throw TrackWithoutNodesException();
}

QJsonArray* VeloDataParser::exportToDataArray(const QStandardItem* treeItem)
{
  QJsonArray* array = new QJsonArray();

  for(int i = 0; i < treeItem->rowCount(); ++i) {
    nodeCount++;

    const QModelIndex index = model->index(i, NodeTreeColumns::KeyColumn, treeItem->index());
    const QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
    const QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
    const QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);

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
    nodeCount++;

    const QModelIndex index = model->index(i, NodeTreeColumns::KeyColumn, treeItem->index());
    const QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
    const QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
    const QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);

    if (typeItem->text() == "Array") {
      object->insert(keyItem->text(),* exportToDataArray(treeItem->child(i)));
    } else if (typeItem->text() == "Bool") {
      object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toBool());
    } else if (typeItem->text() == "Double") {
      if (keyItem->text() == "prefab") {
        prefabCount++;

        PrefabData prefab = valueItem->data(Qt::UserRole).value<PrefabData>();
        if (prefab.id > 0) {
          object->insert(keyItem->text(), int(prefab.id));

          if (prefab.name == "ControlCurve")
            splineCount++;
        } else {
          //ToDo: Exception
          qDebug() << "Unknown prefab id";
        }
      } else {
        if (keyItem->text() == "gate")
          gateCount++;

        object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toDouble());
      }
    } else if (typeItem->text() == "Object") {
      object->insert(keyItem->text(), *exportToObject(treeItem->child(i)));
    } else {
      object->insert(keyItem->text(), valueItem->text());
    }
  }

  return object;
}

void VeloDataParser::importJsonArray(QStandardItem* parentItem, const QJsonArray& dataArray, const uint gateOffset, const bool skipStartgrid)
{
  for (int i = 0; i < dataArray.size(); ++i) {
    nodeCount++;

    const QJsonValue jsonValue = dataArray.at(i);
    QStandardItem* itemKey = new QStandardItem();
    QStandardItem* itemValue = new QStandardItem();
    QStandardItem* itemType = new QStandardItem();

    if (parentItem->text() == "pos" || parentItem->text() == "scale")
      itemKey->setText(i == 0 ? "R" : (i == 1 ? "G" : (i == 2 ? "B" : QString("[%1]").arg(i))));
    else if (parentItem->text() == "rot")
      itemKey->setText(i == 0 ? "L" : (i == 1 ? "I" : (i == 2 ? "J" : (i == 3 ? "K" : QString("[%1]").arg(i)))));
    else
      itemKey->setText(QString("[%1]").arg(i));
    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);
    itemType->setText(getJsonValueTypeAsString(jsonValue.type()));

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

void VeloDataParser::importJsonObject(QStandardItem* parentItem, const QJsonObject& dataObject, const uint gateOffset, const bool skipStartgrid)
{
  const QStringList keyList = dataObject.keys();
  const QString parentSuffixTemplate = "(%1)";
  QString parentSuffix = "";
  bool parentSuffixSet = false;

  QStandardItem* itemKey;
  QStandardItem* itemValue;
  QStandardItem* itemType;

  for (int i = 0; i < keyList.size(); ++i) {
    nodeCount++;

    parentSuffix = "";

    const QJsonValue jsonValue = dataObject.value(keyList.at(i));
    itemKey = new QStandardItem();
    itemValue = new QStandardItem();
    itemType = new QStandardItem();

    itemKey->setData(keyList.at(i), Qt::DisplayRole);
    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);

    if (keyList.at(i) == "prefab") {
      prefabCount++;

      const uint prefabId = jsonValue.toVariant().toUInt();
      if (prefabId > 0 ) {
        const PrefabData prefab = getPrefabData(jsonValue.toVariant().toUInt());

        if (skipStartgrid && NodeEditor::isStartGrid(prefab)) {
          model->removeRows(parentItem->row(), 1, parentItem->parent()->index());
          return;
        }

        itemValue->setData(prefab.name, Qt::DisplayRole);
        QVariant var;
        var.setValue(prefab);
        itemValue->setData(var, Qt::UserRole);

        if (prefab.name == "ControlCurve") {
          splineCount++;
        }
      }
    }

    if (keyList.at(i) == "gate") {
      gateCount++;
      const uint gateId = jsonValue.toVariant().toUInt() + gateOffset;
      if (gateOffset > 0) {
        itemValue->setData(gateId, Qt::EditRole);
      }
      parentSuffix = parentSuffixTemplate.arg(gateId, 0, 10);
    }

    itemType->setText(getJsonValueTypeAsString(jsonValue.type()));

    itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
    if (itemValue->data(Qt::DisplayRole).toString() == "")
      itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
    itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

    QList<QStandardItem*> childColumns;
    childColumns << itemKey << itemValue << itemType;
    parentItem->appendRow(childColumns);

    if ((itemKey->text() == "prefab") &&
        (parentItem->text() != "jo") &&
        (parentItem->text() != "ctrlp"))
    {
      if (itemValue->text() != "") {
        if (parentSuffix == "")
          parentSuffix = parentSuffixTemplate.arg(jsonValue.toVariant().toUInt(), 0, 10);
        parentItem->setText(itemValue->text());
      } else {
        if (parentSuffix == "")
          parentSuffix = parentSuffixTemplate.arg(jsonValue.toVariant().toUInt(), 0, 10);
        parentItem->setText(itemKey->text());
      }
    }

    if (!parentSuffixSet && (parentSuffix != ""))
    {
      parentSuffixSet = true;
      parentItem->setText(parentItem->text() + " " + parentSuffix);
    }

    if(jsonValue.type() == QJsonValue::Object)
      importJsonObject(itemKey, jsonValue.toObject(), gateOffset);
    else if (jsonValue.type() == QJsonValue::Array) {
      importJsonArray(itemKey, jsonValue.toArray(), gateOffset);
    }
  }
}

QString VeloDataParser::getJsonValueTypeAsString(const QJsonValue::Type type)
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
