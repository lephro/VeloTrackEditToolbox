#include "prefabitem.h"

PrefabItem::PrefabItem()
{
}

void PrefabItem::parseIndex(QStandardItemModel* model, const QModelIndex &index)
{
  // Check for valid parameters
  if (model == nullptr)
    return;

  if (!index.isValid())
    return;

  // Search for the prefab key, to use as our entry point
  // Only look for siblings and traverse up
  QModelIndex searchIndex = index;
  bool found = false;
  while (!found) {
    const QStandardItem* typeItem = model->itemFromIndex(searchIndex.sibling(searchIndex.row(), NodeTreeColumns::TypeColumn));    
    if (typeItem == nullptr)
      return;

    if (typeItem->text() == "Array" || typeItem->text() == "Double") {
      searchIndex = searchIndex.parent();
      continue;
    } else if (typeItem->text() == "Object") {
      const QStandardItem* searchKeyItem = model->itemFromIndex(searchIndex.sibling(searchIndex.row(), NodeTreeColumns::KeyColumn));
      qDebug() << typeItem << typeItem->text() << searchKeyItem << searchKeyItem->text();
      for(int i = 0; i < searchKeyItem->rowCount(); ++i) {
        const QStandardItem* rowKeyItem = searchKeyItem->child(i, NodeTreeColumns::KeyColumn);

        if (rowKeyItem->text() != "prefab")
          continue;

        // Bingo
        found = true;
        break;
      }
    }
  }

  // We found something if we got a valid index
  if (!found || !searchIndex.isValid()) {
    // ToDo: Exception
    return;
  }

  // Let the parsing begin!
  this->model = model;
  this->index = searchIndex;
  const QStandardItem* keyItem = model->itemFromIndex(searchIndex.sibling(searchIndex.row(), NodeTreeColumns::KeyColumn));
  for(int i = 0; i < keyItem->rowCount(); ++i) {
    const QModelIndex rowKeyIndex = model->index(i, NodeTreeColumns::KeyColumn, keyItem->index());
    const QStandardItem* rowKeyItem = keyItem->child(rowKeyIndex.row(), NodeTreeColumns::KeyColumn);
    const QStandardItem* rowValueItem = keyItem->child(rowKeyIndex.row(), NodeTreeColumns::ValueColumn);

    if (rowKeyItem->text() == "prefab") {
      this->data = rowValueItem->data(Qt::UserRole).value<PrefabData>();
      if (data != nullptr && data.id > 0)
        this->id = data.id;
    } else if(rowKeyItem->text() == "gate") {
      this->gateNo = rowValueItem->data(Qt::EditRole).toUInt();
    } else if(rowKeyItem->text() == "finish") {
      this->finish = rowValueItem->data(Qt::EditRole).toBool();
    } else if(rowKeyItem->text() == "start") {
      this->start = rowValueItem->data(Qt::EditRole).toBool();
    } else if(rowKeyItem->text() == "gate") {
      this->gateNo = rowValueItem->data(Qt::EditRole).toUInt();
    } else if(rowKeyItem->text() == "curve") {
      this->curveIndex = rowKeyItem->index();
    } else if(rowKeyItem->text() == "trans") {
      for(int transRow = 0; transRow < rowKeyItem->rowCount(); ++transRow) {
        const QModelIndex transRowKeyIndex = model->index(transRow, NodeTreeColumns::KeyColumn, rowKeyItem->index());
        const QStandardItem* transRowKeyItem = rowKeyItem->child(transRowKeyIndex.row(), NodeTreeColumns::KeyColumn);
        if (transRowKeyItem->text() == "pos") {
          positionIndex = transRowKeyIndex;
          for(int posRow = 0; posRow < transRowKeyItem->rowCount(); ++posRow) {
            const QModelIndex posRowKeyIndex = model->index(transRow, NodeTreeColumns::KeyColumn, rowKeyItem->index());
            const QStandardItem* posRowValueItem = transRowKeyItem->child(posRowKeyIndex.row(), NodeTreeColumns::ValueColumn);
            switch (posRow) {
            case 0: this->positionR = posRowValueItem->data(Qt::EditRole).toInt(); break;
            case 1: this->positionG = posRowValueItem->data(Qt::EditRole).toInt(); break;
            case 2: this->positionB = posRowValueItem->data(Qt::EditRole).toInt(); break;
            }
          }
        } else if (transRowKeyItem->text() == "rot") {
          rotationIndex = transRowKeyIndex;
          for(int rotRow = 0; rotRow < transRowKeyItem->rowCount(); ++rotRow) {
            const QModelIndex rotRowKeyIndex = model->index(transRow, NodeTreeColumns::KeyColumn, rowKeyItem->index());
            const QStandardItem* rotRowValueItem = transRowKeyItem->child(rotRowKeyIndex.row(), NodeTreeColumns::ValueColumn);
            switch (rotRow) {
            case 0: this->rotationL = rotRowValueItem->data(Qt::EditRole).toInt(); break;
            case 1: this->rotationI = rotRowValueItem->data(Qt::EditRole).toInt(); break;
            case 2: this->rotationJ = rotRowValueItem->data(Qt::EditRole).toInt(); break;
            case 3: this->rotationK = rotRowValueItem->data(Qt::EditRole).toInt(); break;
            }
          }
        } else if (transRowKeyItem->text() == "scale") {
          scaleIndex = transRowKeyIndex;
          for(int scaleRow = 0; scaleRow < transRowKeyItem->rowCount(); ++scaleRow) {
            const QModelIndex scaleRowKeyIndex = model->index(transRow, NodeTreeColumns::KeyColumn, rowKeyItem->index());
            const QStandardItem* scaleRowValueItem = transRowKeyItem->child(scaleRowKeyIndex.row(), NodeTreeColumns::ValueColumn);
            switch (scaleRow) {
            case 0: this->scalingR = scaleRowValueItem->data(Qt::EditRole).toInt(); break;
            case 1: this->scalingG = scaleRowValueItem->data(Qt::EditRole).toInt(); break;
            case 2: this->scalingB = scaleRowValueItem->data(Qt::EditRole).toInt(); break;
            }
          }
        }
      }
    }
  }
}

unsigned int PrefabItem::getId() const
{
  return id;
}

void PrefabItem::setId(unsigned int value)
{
  id = value;

}

PrefabData PrefabItem::getData() const
{
  return data;
}

void PrefabItem::setData(const PrefabData &value)
{
  data = value;
}

int PrefabItem::getPosition(const int row) const
{
  switch (row) {
  case 0: return positionR;
  case 1: return positionG;
  case 2: return positionB;
  }
  return 0;
}

void PrefabItem::setPosition(const int row, const int value)
{
  switch (row) {
  case 0: positionR = value; break;
  case 1: positionG = value; break;
  case 2: positionB = value; break;
  }


  // Update the model if set
  if (!positionIndex.isValid())
    return;

  if (model == nullptr)
    return;

  model->itemFromIndex(model->index(row, NodeTreeColumns::KeyColumn, positionIndex))->setData(Qt::EditRole, value);
}

int PrefabItem::getRotation(const int row) const
{
  switch (row) {
  case 0: return rotationL;
  case 1: return rotationI;
  case 2: return rotationJ;
  case 3: return rotationK;
  }
  return 0;
}

void PrefabItem::setRotation(const int row, const int value)
{
  switch (row) {
  case 0: rotationL = value; break;
  case 1: rotationI = value; break;
  case 2: rotationJ = value; break;
  case 3: rotationK = value; break;
  }

  // Update the model if set
  if (!rotationIndex.isValid())
    return;

  if (model == nullptr)
    return;

  model->itemFromIndex(model->index(row, NodeTreeColumns::KeyColumn, rotationIndex))->setData(Qt::EditRole, value);
}

int PrefabItem::getScaling(const int row) const
{
  switch (row) {
  case 0: return scalingR;
  case 1: return scalingG;
  case 2: return scalingB;
  }
}

void PrefabItem::setScaling(const int row, const int value)
{
  switch (row) {
  case 0: scalingR = value; break;
  case 1: scalingG = value; break;
  case 2: scalingB = value; break;
  }

  // Update the model if set
  if (!scaleIndex.isValid())
    return;

  if (model == nullptr)
    return;

  model->itemFromIndex(model->index(row, NodeTreeColumns::KeyColumn, scaleIndex))->setData(Qt::EditRole, value);
}

int PrefabItem::getPositionR() const
{
  return positionR;
}

void PrefabItem::setPositionR(const int value)
{
  setPosition(0, value);
}

int PrefabItem::getPositionG() const
{
  return positionG;
}

void PrefabItem::setPositionG(int value)
{
  setPosition(1, value);
}

int PrefabItem::getPositionB() const
{
  return positionB;
}

void PrefabItem::setPositionB(const int value)
{
  setPosition(2, value);
}

int PrefabItem::getRotationL() const
{
  return rotationL;
}

void PrefabItem::setRotationL(const int value)
{
  setRotation(0, value);
}

int PrefabItem::getRotationI() const
{
  return rotationI;
}

void PrefabItem::setRotationI(const int value)
{
  setRotation(1, value);
}

int PrefabItem::getRotationJ() const
{
  return rotationJ;
}

void PrefabItem::setRotationJ(const int value)
{
  setRotation(2, value);
}

int PrefabItem::getRotationK() const
{
  return rotationK;
}

void PrefabItem::setRotationK(const int value)
{
  setRotation(3, value);
}

int PrefabItem::getScalingR() const
{
  return scalingR;
}

void PrefabItem::setScalingR(const int value)
{
  setScaling(0, value);
}

int PrefabItem::getScalingG() const
{
  return scalingG;
}

void PrefabItem::setScalingG(const int value)
{
  setScaling(1, value);
}

int PrefabItem::getScalingB() const
{
  return scalingB;
}

void PrefabItem::setScalingB(const int value)
{
  setScaling(2, value);
}

unsigned int PrefabItem::getGateNo() const
{
  return gateNo;
}

void PrefabItem::setGateNo(const unsigned int value)
{
  gateNo = value;
  setValue("gate", value);
}

bool PrefabItem::getFinish() const
{
  return finish;
}

void PrefabItem::setFinish(const bool value)
{
  finish = value;
  setValue("finish", value);
}

bool PrefabItem::getStart() const
{
  return start;
}

void PrefabItem::setStart(const bool value)
{
  start = value;
  setValue("start", value);
}

bool PrefabItem::operator==(PrefabItem &b) {
  return (index == b.index) &&
         (gateNo == b.gateNo) &&
         (start == b.start) &&
         (finish == b.finish) &&
         (positionR == b.positionR) &&
         (positionG == b.positionG) &&
         (positionB == b.positionB) &&
         (rotationL == b.rotationL) &&
         (rotationI == b.rotationI) &&
         (rotationJ == b.rotationJ) &&
         (rotationK == b.rotationK) &&
         (scalingR == b.scalingR) &&
         (scalingG == b.scalingG) &&
         (scalingB == b.scalingB);
}

void PrefabItem::setValue(QString propertyName, QVariant value) {
  if (model == nullptr)
    return;

  // Search for a property with the given name and update it
  const QStandardItem* rootItem = model->itemFromIndex(index);
  for(int i = 0; i < rootItem->rowCount(); ++i) {
    const QModelIndex valueIndex = model->index(i, NodeTreeColumns::ValueColumn, rootItem->index());
    const QStandardItem* keyItem = rootItem->child(model->index(i, NodeTreeColumns::KeyColumn, rootItem->index()).row(), NodeTreeColumns::KeyColumn);
    if (keyItem->text() == propertyName) {
      model->itemFromIndex(valueIndex)->setData(value, Qt::EditRole);
    }
  }
}

