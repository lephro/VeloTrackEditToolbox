#include "velodataparser.h"

VeloDataParser::VeloDataParser(QObject* parent)
  : QObject(parent)
{
}

QByteArray* VeloDataParser::exportToJson()
{
  nodeCount = 0;
  readPrefabCount = 0;
  readSplineCount = 0;
  readGateCount = 0;

  QJsonDocument jsonDoc;
  QJsonObject jsonObj(jsonDoc.object());

  // ToDo: Export
//  QStandardItem* rootKeyItem = model.invisibleRootItem();

//  for(int i = 0; i < rootKeyItem->rowCount(); ++i) {
//    nodeCount++;

//    QModelIndex index = model.index(i, NodeTreeColumns::KeyColumn, rootKeyItem->index());
//    QStandardItem* keyItem = rootKeyItem->child(index.row(), NodeTreeColumns::KeyColumn);
//    QStandardItem* valueItem = rootKeyItem->child(index.row(), NodeTreeColumns::ValueColumn);
//    QStandardItem* typeItem = rootKeyItem->child(index.row(), NodeTreeColumns::TypeColumn);

//    if (typeItem->text() == "Array") {
//      jsonObj.insert(keyItem->text(), *exportToDataArray(keyItem));
//    } else if (typeItem->text() == "Object") {
//      jsonObj.insert(keyItem->text(), *exportToObject(keyItem));
//    } else {
//      jsonObj.insert(keyItem->text(), valueItem->text());
//    }
//  }

//  jsonDoc.setObject(jsonObj);
  return new QByteArray(jsonDoc.toJson(QJsonDocument::Compact));
}

uint VeloDataParser::getGateCount() const
{
  return readGateCount;
}

uint VeloDataParser::getNodeCount() const
{
  return nodeCount;
}

uint VeloDataParser::getPrefabCount() const
{
  return readPrefabCount;
}

uint VeloDataParser::getSplineCount() const
{
  return readSplineCount;
}

//void VeloDataParser::importJson(const QByteArray &jsonData)
//{
//  model.clear();

//  nodeCount = 0;
//  readPrefabCount = 0;
//  readSplineCount = 0;
//  readGateCount = 0;

//  QStringList labels;
//  labels << tr("Node") << tr("Value") << tr("Type");
//  model.setHorizontalHeaderLabels(labels);

//  QStandardItem* rootItem = model.invisibleRootItem();

//  QJsonDocument doc = QJsonDocument(QJsonDocument::fromJson(jsonData));
//  if (doc.isNull())
//    throw TrackWithoutNodesException();

//  QJsonObject* jsonRootObject = new QJsonObject(doc.object());

//  importJsonObject(rootItem, *jsonRootObject);

//  if (rootItem->rowCount() == 0)
//    throw TrackWithoutNodesException();
//}

//void VeloDataParser::mergeJson(const QByteArray &jsonData,
//                                   const bool addBarriers,
//                                   const bool addGates)
//{
//  QStandardItem* rootItem = model.invisibleRootItem();

//  QJsonDocument doc = QJsonDocument(QJsonDocument::fromJson(jsonData));
//  if (doc.isNull())
//    throw TrackWithoutNodesException();

//  QJsonObject* jsonRootObject = new QJsonObject(doc.object());

//  QStringList keyList = jsonRootObject->keys();
//  for (int i = 0; i < keyList.size(); ++i) {
//    if (!addGates && (keyList.at(i) == "gates"))
//      continue;

//    if (!addBarriers && (keyList.at(i) == "barriers"))
//      continue;

//    QList<QStandardItem*> foundItems = model.findItems(keyList.at(i), Qt::MatchRecursive, NodeTreeColumns::KeyColumn);
//    if (foundItems.count() > 0) {
//      if (foundItems.first()->text() == "weather")
//        continue;

//      QJsonValue jsonValue = jsonRootObject->value(keyList.at(i));
//      if(jsonValue.type() == QJsonValue::Object)
//        importJsonObject(foundItems.first(), jsonValue.toObject(), getGatesInModelCount(), true);
//      else if (jsonValue.type() == QJsonValue::Array) {
//        importJsonArray(foundItems.first(), jsonValue.toArray(), getGatesInModelCount(), true);
//      }
//    } else {
//      QJsonValue jsonValue = jsonRootObject->value(keyList.at(i));
//      QStandardItem* itemKey = new QStandardItem();
//      QStandardItem* itemValue = new QStandardItem();
//      QStandardItem* itemType = new QStandardItem();

//      itemKey->setData(keyList.at(i), Qt::DisplayRole);
//      itemValue->setData(jsonValue.toVariant(), Qt::EditRole);

//      if (keyList.at(i) == "prefab") {
//        uint prefabId = jsonValue.toVariant().toUInt();
//        if (prefabId > 0 ) {
//          PrefabData prefab = getPrefabData(jsonValue.toVariant().toUInt());
//          itemValue->setData(prefab.name, Qt::DisplayRole);
//          QVariant var;
//          var.setValue(prefab);
//          itemValue->setData(var, Qt::UserRole);
//        }
//      }

//      itemType->setText(getJsonValueTypeAsString(jsonValue.type()));

//      itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
//      if (itemValue->data(Qt::DisplayRole).toString() == "")
//        itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
//      itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

//      QList<QStandardItem*> childColumns;
//      childColumns << itemKey << itemValue << itemType;
//      rootItem->appendRow(childColumns);

//      if (itemKey->text() == "prefab") {
//        if (itemValue->text() != "")
//          rootItem->setText(itemValue->text());
//        else
//        rootItem->setText(itemKey->text());
//      }

//      if(jsonValue.type() == QJsonValue::Object)
//        importJsonObject(itemKey, jsonValue.toObject(), getGatesInModelCount());
//      else if (jsonValue.type() == QJsonValue::Array) {
//        importJsonArray(itemKey, jsonValue.toArray(), getGatesInModelCount());
//      }
//    }
//  }

//  if (rootItem->rowCount() == 0)
//    throw TrackWithoutNodesException();
//}

Track& VeloDataParser::parseTrack(const QVector<PrefabData>& prefabs, const TrackData& trackData)
{
  readPrefabCount = 0;
  readSplineCount = 0;
  readGateCount = 0;

  const QJsonDocument doc = QJsonDocument(QJsonDocument::fromJson(trackData.value));
  if (doc.isNull())
    throw TrackWithoutNodesException();

  Track* track = new Track();
  track->setAvailablePrefabs(prefabs);

  const QJsonObject jsonRootObject(doc.object());
  QJsonArray jsonArray = jsonRootObject.value("barriers").toArray();
  for (int i = 0; i < jsonArray.size(); ++i) {
    track->addObject(parsePrefab(prefabs, jsonArray.at(i).toObject()));
  }

  jsonArray = jsonRootObject.value("gates").toArray();
  for (int i = 0; i < jsonArray.size(); ++i) {
    track->addObject(parsePrefab(prefabs, jsonArray.at(i).toObject()));
  }

  const QJsonObject& weatherObject = jsonRootObject.value("weather").toObject();
  if (!weatherObject.isEmpty()) {
    WeatherData& weather = track->Weather();
    weather.cloud = weatherObject.value("cloud").toDouble();
    weather.dambient = weatherObject.value("dambient").toDouble();
    weather.dlight = weatherObject.value("dlight").toDouble();
    weather.dshadow = weatherObject.value("dshadow").toDouble();
    weather.fog = weatherObject.value("fog").toBool();
    weather.hour = weatherObject.value("hour").toDouble();
    weather.latitude = weatherObject.value("latitude").toDouble();
    weather.longitude = weatherObject.value("longitude").toDouble();
    weather.month = weatherObject.value("month").toBool();
    weather.nambient = weatherObject.value("nambient").toDouble();
    weather.nlight = weatherObject.value("nlight").toDouble();
    weather.nshadow = weatherObject.value("nshadow").toDouble();
    weather.time = weatherObject.value("time").toBool();
    weather.utc = weatherObject.value("utc").toDouble();
  }

  return *track;
}

QJsonArray* VeloDataParser::exportToDataArray(const QStandardItem* treeItem)
{
  QJsonArray* array = new QJsonArray();

  // ToDo
//  for(int i = 0; i < treeItem->rowCount(); ++i) {
//    nodeCount++;

//    const QModelIndex index = model.index(i, NodeTreeColumns::KeyColumn, treeItem->index());
//    const QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
//    const QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
//    const QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);

//    if (typeItem->text() == "Array") {
//      array->append(*exportToDataArray(keyItem));
//    } else if (typeItem->text() == "Bool") {
//      array->append(valueItem->data(Qt::EditRole).toBool());
//    } else if (typeItem->text() == "Double") {
//      array->append(valueItem->data(Qt::EditRole).toDouble());
//    } else if (typeItem->text() == "Object") {
//      array->append(*exportToObject(keyItem));
//    } else {
//      array->append(keyItem->text());
//    }
//  }

  return array;
}

QJsonObject* VeloDataParser::exportToObject(const QStandardItem* treeItem)
{
  QJsonObject* object = new QJsonObject();

  // ToDo
//  for(int i = 0; i < treeItem->rowCount(); ++i) {
//    nodeCount++;

//    const QModelIndex index = model.index(i, NodeTreeColumns::KeyColumn, treeItem->index());
//    const QStandardItem* keyItem = treeItem->child(index.row(), NodeTreeColumns::KeyColumn);
//    const QStandardItem* valueItem = treeItem->child(index.row(), NodeTreeColumns::ValueColumn);
//    const QStandardItem* typeItem = treeItem->child(index.row(), NodeTreeColumns::TypeColumn);

//    if (typeItem->text() == "Array") {
//      object->insert(keyItem->text(),* exportToDataArray(treeItem->child(i)));
//    } else if (typeItem->text() == "Bool") {
//      object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toBool());
//    } else if (typeItem->text() == "Double") {
//      if (keyItem->text() == "prefab") {
//        readPrefabCount++;

//        PrefabData prefab = valueItem->data(Qt::UserRole).value<PrefabData>();
//        if (prefab.id > 0) {
//          object->insert(keyItem->text(), int(prefab.id));

//          if (prefab.name == "ControlCurve")
//            readSplineCount++;
//        } else {
//          //ToDo: Exception
//          qDebug() << "Unknown prefab id";
//        }
//      } else {
//        if (keyItem->text() == "gate")
//          readGateCount++;

//        object->insert(keyItem->text(), valueItem->data(Qt::EditRole).toDouble());
//      }
//    } else if (typeItem->text() == "Object") {
//      object->insert(keyItem->text(), *exportToObject(treeItem->child(i)));
//    } else {
//      object->insert(keyItem->text(), valueItem->text());
//    }
//  }

  return object;
}

void VeloDataParser::importJsonArray(QStandardItem* parentItem, const QJsonArray& dataArray, const uint gateOffset, const bool skipStartgrid)
{
//  for (int i = 0; i < dataArray.size(); ++i) {
//    nodeCount++;

//    const QJsonValue jsonValue = dataArray.at(i);
//    QStandardItem* itemKey = new QStandardItem();
//    QStandardItem* itemValue = new QStandardItem();
//    QStandardItem* itemType = new QStandardItem();

//    if (parentItem->text() == "pos" || parentItem->text() == "scale")
//      itemKey->setText(i == 0 ? "R" : (i == 1 ? "G" : (i == 2 ? "B" : QString("[%1]").arg(i))));
//    else if (parentItem->text() == "rot")
//      itemKey->setText(i == 0 ? "W" : (i == 1 ? "X" : (i == 2 ? "Y" : (i == 3 ? "Z" : QString("[%1]").arg(i)))));
//    else
//      itemKey->setText(QString("[%1]").arg(i));
//    itemValue->setData(jsonValue.toVariant(), Qt::EditRole);
//    itemType->setText(getJsonValueTypeAsString(jsonValue.type()));

//    itemKey->setFlags(itemKey->flags() ^ Qt::ItemIsEditable);
//    if (itemValue->data(Qt::EditRole).toString() == "") {
//      itemValue->setFlags(itemValue->flags() ^ Qt::ItemIsEditable);
//    }
//    itemType->setFlags(itemType->flags() ^ Qt::ItemIsEditable);

//    QList<QStandardItem*> childColumns;
//    childColumns << itemKey << itemValue << itemType;
//    parentItem->appendRow(childColumns);

//    if (jsonValue.type() == QJsonValue::Object)
//      importJsonObject(itemKey, jsonValue.toObject(), gateOffset, skipStartgrid);
//  }
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

EditorObject* VeloDataParser::parsePrefab(const QVector<PrefabData>& prefabs, const QJsonObject& dataObject)
{
  if (dataObject.isEmpty())
    return nullptr;

  bool prefabDataSet = false;
  bool prefabPosSet = false;
  bool prefabRotSet = false;
  bool prefabScaleSet = false;

  EditorObject* object = new EditorObject();

  QJsonArray jsonArray = dataObject.value("curve").toObject().value("lobjs").toArray();
  QJsonArray lobjArray;
  QJsonObject lobjObject;
  for (int lobjsItr = 0; lobjsItr < jsonArray.size(); ++lobjsItr) {
    lobjArray = jsonArray.at(lobjsItr).toObject().value("lojb").toArray();

    for (int lobjItr = 0; lobjItr < lobjArray.size(); ++lobjItr) {
      lobjObject = lobjArray.at(lobjItr).toObject();
      object->setSplineIndex(lobjObject.value("index").toInt());
      object->setIsMoving(lobjObject.value("tobj").toObject().value("isMoving").toBool());
      object->setSpeed(char(lobjObject.value("tobj").toObject().value("speed").toInt()));
      EditorObject* splineObject = parsePrefab(prefabs, lobjObject.value("jo").toObject());
      if (splineObject != nullptr && splineObject->getData().id > 0) {
        splineObject->setParentObject(object);
        object->getSplineObjects().append(splineObject);
      }

      EditorObject* splineParent = parsePrefab(prefabs, lobjObject.value("ctrlp").toObject());
      if (splineParent != nullptr && splineParent->getData().id > 0) {
        splineParent->setParentObject(object);
        object->getSplineParents().append(splineParent);
      }
    }
  }
  jsonArray = dataObject.value("curve").toObject().value("ctrls").toArray();
  for (int controlItr = 0; controlItr < jsonArray.size(); ++controlItr) {
    EditorObject* splineObject = parsePrefab(prefabs, jsonArray.at(controlItr).toObject());

    if (splineObject->getData().id <= 0)
      continue;

    splineObject->setParentObject(object);

    object->getSplineControls().append(splineObject);
  }

  object->setFinish(dataObject.value("finish").toBool());
  object->setGateNo(dataObject.value("gate").toInt(), false);
  if (object->isGate())
    readGateCount++;

  const uint prefabId = dataObject.value("prefab").toVariant().toUInt();
  if (prefabId > 0 ) {
    prefabDataSet = true;
    for(int i = 0; i < prefabs.count(); ++i) {
      if (prefabs[i].id != prefabId)
        continue;

      object->setData(prefabs[i]);
      break;
    }
  }

  object->setStart(dataObject.value("start").toBool());

  jsonArray = dataObject.value("trans").toObject().value("pos").toArray();
  for(int i = 0; i < jsonArray.size(); ++i)
    object->setPosition(i, jsonArray.at(i).toInt());
  prefabPosSet = jsonArray.size() == 3;

  jsonArray = dataObject.value("trans").toObject().value("rot").toArray();
  for(int i = 0; i < jsonArray.size(); ++i)
    object->setRotation(i, jsonArray.at(i).toInt());
  prefabRotSet = jsonArray.size() == 4;

  jsonArray = dataObject.value("trans").toObject().value("scale").toArray();
  for(int i = 0; i < jsonArray.size(); ++i)
    object->setScaling(i, jsonArray.at(i).toInt());
  prefabScaleSet = jsonArray.size() == 3;

  if (!prefabDataSet) {
    delete object;
    throw InvalidDataException(tr("The prefab data could not be parsed."));
  }

  if (!prefabPosSet) {
    delete object;
    throw InvalidDataException(tr("The prefab position could not be parsed."));
  }

  if (!prefabRotSet) {
    delete object;
    throw InvalidDataException(tr("The prefab rotation could not be parsed."));
  }

  if (!prefabScaleSet) {
    delete object;
    throw InvalidDataException(tr("The prefab scaling could not be parsed."));
  }

  readPrefabCount++;

  return object;
}
