#include "prefabitem.h"

PrefabItem::PrefabItem(NodeEditor *parentEditor, const QModelIndex& prefabIndex)
  : editor(parentEditor)
{
  // Get the model from the editor, if set
  if (editor == nullptr)
    return;

  model = &editor->getStandardModel();

  parseIndex(prefabIndex);
}

void PrefabItem::applyScaling(const QVector3D values)
{
  if (values == QVector3D(1, 1, 1))
    return;

  setScaling(0, int(std::round(scalingR * values.x())));
  setScaling(1, int(std::round(scalingG * values.y())));
  setScaling(2, int(std::round(scalingB * values.z())));
}

bool PrefabItem::parseIndex(const QModelIndex& index, NodeEditor* editor)
{  
  // Check for valid parameters
  if (this->editor == nullptr && editor == nullptr)
    return false;

  if (!index.isValid())
    return false;

  this->index = QModelIndex();
  gate = false;

  if (this->editor == nullptr) {
    this->editor = editor;
    this->model = &editor->getStandardModel();
  }

  // Search for the prefab key, to use as our entry point
  // Only look for siblings and traverse up
  QModelIndex searchIndex = index;
  bool found = false;
  QModelIndex typeItem;
  do {
    typeItem = searchIndex.siblingAtColumn(NodeTreeColumns::TypeColumn);
    // If our typeItem is invalid, we reached the top  without finding the entry point
    if (!typeItem.isValid()) {
      this->editor = nullptr;
      this->model = nullptr;
      return false;
    }

    if (typeItem.data(Qt::DisplayRole) == "Object") {
      //qDebug() << typeItem << typeItem.data(Qt::DisplayRole) << searchIndex << searchIndex.data(Qt::DisplayRole);
      for(int i = 0; i < this->model->rowCount(searchIndex); ++i) {
        if (this->model->index(i, NodeTreeColumns::KeyColumn, searchIndex).data(Qt::DisplayRole) != "prefab")
          continue;

        // Bingo
        found = true;
        this->index = searchIndex;
        break;
      }            
    }

    searchIndex = searchIndex.parent();
  } while (!found);

  // Let the parsing begin!  
  //const QStandardItem* keyItem = model->itemFromIndex(this->index.siblingAtColumn(NodeTreeColumns::KeyColumn));
  const QModelIndex keyIndex = this->index.siblingAtColumn(NodeTreeColumns::KeyColumn);
  for(int i = 0; i < this->model->rowCount(keyIndex); ++i) {
    const QModelIndex rowKeyIndex = this->model->index(i, NodeTreeColumns::KeyColumn, keyIndex);
    const QModelIndex rowValueIndex = this->model->index(i, NodeTreeColumns::ValueColumn, keyIndex);

    if (rowKeyIndex.data(Qt::DisplayRole) == "prefab") {
      this->data = rowValueIndex.data(Qt::UserRole).value<PrefabData>();
    } else if(rowKeyIndex.data(Qt::DisplayRole) == "gate") {
      gate = true;
      this->gateNo = rowValueIndex.data(Qt::EditRole).toInt();
    } else if(rowKeyIndex.data(Qt::DisplayRole) == "finish") {
      this->finish = rowValueIndex.data(Qt::EditRole).toString() == "true" ? true : false;
    } else if(rowKeyIndex.data(Qt::DisplayRole) == "start") {
      this->start = rowValueIndex.data(Qt::EditRole).toString() == "true" ? true : false;
    } else if(rowKeyIndex.data(Qt::DisplayRole) == "curve") {
      this->curveIndex = rowKeyIndex;
    } else if(rowKeyIndex.data(Qt::DisplayRole) == "trans") {
      for(int transRow = 0; transRow < this->model->rowCount(rowKeyIndex); ++transRow) {
        const QModelIndex transRowKeyItem = this->model->index(transRow, NodeTreeColumns::KeyColumn, rowKeyIndex);
        if (transRowKeyItem.data(Qt::DisplayRole) == "pos") {
          positionIndex = transRowKeyItem;
          for(int posRow = 0; posRow < this->model->rowCount(transRowKeyItem); ++posRow) {
            const QModelIndex posRowValueItem = this->model->index(posRow, NodeTreeColumns::ValueColumn, transRowKeyItem);
            switch (posRow) {
            case 0: this->positionR = posRowValueItem.data(Qt::EditRole).toInt(); break;
            case 1: this->positionG = posRowValueItem.data(Qt::EditRole).toInt(); break;
            case 2: this->positionB = posRowValueItem.data(Qt::EditRole).toInt(); break;
            }
          }
        } else if (transRowKeyItem.data(Qt::DisplayRole) == "rot") {
          rotationIndex = transRowKeyItem;
          for(int rotRow = 0; rotRow < this->model->rowCount(transRowKeyItem); ++rotRow) {
            const QModelIndex rotRowValueItem = this->model->index(rotRow, NodeTreeColumns::ValueColumn, rotationIndex);
            switch (rotRow) {
            case 0: this->rotationW = rotRowValueItem.data(Qt::EditRole).toInt(); break;
            case 1: this->rotationX = rotRowValueItem.data(Qt::EditRole).toInt(); break;
            case 2: this->rotationY = rotRowValueItem.data(Qt::EditRole).toInt(); break;
            case 3: this->rotationZ = rotRowValueItem.data(Qt::EditRole).toInt(); break;
            }
          }
        } else if (transRowKeyItem.data(Qt::DisplayRole) == "scale") {
          scaleIndex = transRowKeyItem;
          for(int scaleRow = 0; scaleRow < this->model->rowCount(transRowKeyItem); ++scaleRow) {
            const QModelIndex scaleRowValueItem = this->model->index(scaleRow, NodeTreeColumns::ValueColumn, scaleIndex);
            switch (scaleRow) {
            case 0: this->scalingR = scaleRowValueItem.data(Qt::EditRole).toInt(); break;
            case 1: this->scalingG = scaleRowValueItem.data(Qt::EditRole).toInt(); break;
            case 2: this->scalingB = scaleRowValueItem.data(Qt::EditRole).toInt(); break;
            }
          }
        }
      }
    }
  }  

  return true;
}


PrefabData PrefabItem::getData() const
{
  return data;
}

unsigned int PrefabItem::getId() const
{
  return data.id;
}

QModelIndex PrefabItem::getIndex() const
{
  return index;
}

void PrefabItem::setData(const PrefabData &value)
{
  if (!dataEditable())
    return;

  data = value;

  // Update the model if set
  if (!hasValidEditor())
    return;

  setModified();

  QModelIndex propertyIndex = getPropertyValueIndex("prefab");
  if (!propertyIndex.isValid())
    return;

  model->setData(propertyIndex, data.name);
  QVariant var;
  var.setValue(data);
  model->setData(propertyIndex, var, Qt::UserRole);

  // Update parent description
  model->setData(propertyIndex.parent(), data.name + " " + QString("(%1)").arg(data.id, 0, 10));
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
  if (isSpline())
    return;

  switch (row) {
  case 0: positionR = value; break;
  case 1: positionG = value; break;
  case 2: positionB = value; break;
  }

  // Update the model if set
  if (!hasValidEditor(positionIndex))
    return;

  setModified();

  setValueInModel(row, positionIndex, value);
}

void PrefabItem::setPosition(const int r, const int g, const int b)
{
  if (isSpline())
    return;

  positionR = r;
  positionG = g;
  positionB = b;

  // Update the model if set
  if (!hasValidEditor(positionIndex))
    return;

  setModified();

  setValueInModel(0, positionIndex, r);
  setValueInModel(1, positionIndex, g);
  setValueInModel(2, positionIndex, b);
}

int PrefabItem::getRotationVector(const int row) const
{
  switch (row) {
  case 0: return rotationW;
  case 1: return rotationX;
  case 2: return rotationY;
  case 3: return rotationZ;
  }
  return 0;
}

void PrefabItem::setRotation(const int row, const int value)
{
  if (isSpline())
    return;

  switch (row) {
  case 0: rotationW = value; break;
  case 1: rotationX = value; break;
  case 2: rotationY = value; break;
  case 3: rotationZ = value; break;
  }

  // Update the model if set
  if (!hasValidEditor(rotationIndex))
    return;

  setModified();

  setValueInModel(row, rotationIndex, value);
}

void PrefabItem::setRotation(const int w, const int x, const int y, const int z)
{
  if (isSpline())
    return;

  rotationW = w;
  rotationX = x;
  rotationY = y;
  rotationZ = z;

  // Update the model if set
  if (!hasValidEditor(rotationIndex))
    return;

  setModified();

  setValueInModel(0, rotationIndex, w);
  setValueInModel(1, rotationIndex, x);
  setValueInModel(2, rotationIndex, y);
  setValueInModel(3, rotationIndex, z);
}

void PrefabItem::setRotation(const QQuaternion rotation)
{
  setRotation(int(std::round(rotation.scalar() * 1000)),
              int(std::round(rotation.x() * 1000)),
              int(std::round(rotation.y() * 1000)),
              int(std::round(rotation.z() * 1000)));
}

int PrefabItem::getScaling(const int row) const
{
  switch (row) {
  case 0: return scalingR;
  case 1: return scalingG;
  case 2: return scalingB;
  }
  return 1;
}

void PrefabItem::setScaling(const int row, const int value)
{
  if (isSpline())
    return;

  switch (row) {
  case 0: scalingR = value; break;
  case 1: scalingG = value; break;
  case 2: scalingB = value; break;
  }

  // Update the model if set
  if (!hasValidEditor(scaleIndex))
    return;

  setModified();

  setValueInModel(row, scaleIndex, value);
}

void PrefabItem::setScaling(const int r, const int g, const int b)
{
  if (isSpline())
    return;

  scalingR = r;
  scalingG = g;
  scalingB = b;

  // Update the model if set
  if (!hasValidEditor(scaleIndex))
    return;

  setModified();

  setValueInModel(0, scaleIndex, r);
  setValueInModel(1, scaleIndex, g);
  setValueInModel(2, scaleIndex, b);
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

QVector3D PrefabItem::getPositionVector() const
{
  return QVector3D(positionR, positionG, positionB);
}

void PrefabItem::setPosition(const QVector3D position)
{
  setPosition(0, int(std::round(position.x())));
  setPosition(1, int(std::round(position.y())));
  setPosition(2, int(std::round(position.z())));
}

int PrefabItem::getRotationW() const
{
  return rotationW;
}

void PrefabItem::setRotationW(const int value)
{
  setRotation(0, value);
}

int PrefabItem::getRotationX() const
{
  return rotationX;
}

void PrefabItem::setRotationX(const int value)
{
  setRotation(1, value);
}

int PrefabItem::getRotationY() const
{
  return rotationY;
}

void PrefabItem::setRotationY(const int value)
{
  setRotation(2, value);
}

int PrefabItem::getRotationZ() const
{
  return rotationZ;
}

void PrefabItem::setRotationZ(const int value)
{
  setRotation(3, value);
}

QQuaternion PrefabItem::getRotationQuaterion() const
{
  return QQuaternion(float(rotationW) / 1000,
                     float(rotationX) / 1000,
                     float(rotationY) / 1000,
                     float(rotationZ) / 1000);
}

QVector4D PrefabItem::getRotationVector() const
{
  return QVector4D(rotationW, rotationX, rotationY, rotationZ);
}

void PrefabItem::setRotation(const QVector4D& rotation)
{
  setRotation(0, int(std::round(rotation.w())));
  setRotation(1, int(std::round(rotation.w())));
  setRotation(2, int(std::round(rotation.w())));
  setRotation(3, int(std::round(rotation.w())));
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

QVector3D PrefabItem::getScalingVector() const
{
  return QVector3D(int(std::round(scalingR)), int(std::round(scalingB)), int(std::round(scalingG)));
}

void PrefabItem::setScaling(const QVector3D values)
{
  setScaling(0, int(values.x()));
  setScaling(1, int(values.y()));
  setScaling(2, int(values.z()));
}

bool PrefabItem::isGate() const
{
  return gate;
}

int PrefabItem::getGateNo() const
{
  return gateNo;
}

void PrefabItem::setGateNo(const int value, const bool updateGateOrder)
{
  // Update gate order
  const uint oldGateNo = uint(gateNo);

  gateNo = value;
  gate = true;  

  // Update the model if set
  setModified();

  if (updateGateOrder && hasValidEditor()) {
    editor->changeGateOrder(uint(oldGateNo), uint(value));
  }
}

bool PrefabItem::getFinish() const
{
  return finish;
}

void PrefabItem::setFinish(const bool value)
{
  // Prevent disabling a finish
  if (finish && !value)
    return;

  finish = value;

  if (hasValidEditor() && value)
    editor->resetFinishGates();

  setValue("finish", value);
}

bool PrefabItem::getStart() const
{
  return start;
}

void PrefabItem::setStart(const bool value)
{
  start = value;

  if (hasValidEditor() && value)
    editor->resetStartGates();

  setValue("start", value);
}

bool PrefabItem::operator==(PrefabItem &b) {
  return (data.id == b.data.id) &&
         (gateNo == b.gateNo) &&
         (start == b.start) &&
         (finish == b.finish) &&
         (positionR == b.positionR) &&
         (positionG == b.positionG) &&
         (positionB == b.positionB) &&
         (rotationW == b.rotationW) &&
         (rotationX == b.rotationX) &&
         (rotationY == b.rotationY) &&
         (rotationZ == b.rotationZ) &&
         (scalingR == b.scalingR) &&
         (scalingG == b.scalingG) &&
         (scalingB == b.scalingB);
}

void PrefabItem::setModified(const bool value)
{
  if (!hasValidEditor())
    return;

  model->setData(index, value, modifiedRole);
}

bool PrefabItem::dataEditable() const
{
  if (data.name == "CtrlParent" ||
      data.name == "ControlCurve" ||
      data.name == "ControlPoint" ||
      data.name == "DefaultStartGrid" ||
      data.name == "DefaultKDRAStartGrid" ||
      data.name == "DR1StartGrid" ||
      data.name == "PolyStartGrid" ||
      data.name == "MicroStartGrid")
    return false;

  return true;
}

bool PrefabItem::isModified() const
{
  if (!hasValidEditor(index))
    return false;

  return model->data(index, modifiedRole).toBool();
}

bool PrefabItem::isOnSpline() const
{
  if (!hasValidEditor())
    return false;

  return (this->index.data(Qt::DisplayRole) == "jo");
}

bool PrefabItem::isSplineControl() const
{
  return (data.id == 345);
}

void PrefabItem::setFilterMark(const bool found)
{
  if (!hasValidEditor())
    return;

  QStandardItem* item = model->itemFromIndex(index);
  if (item == nullptr)
    return;

  item->setData(found, USERROLE_FILTER);
  item->setForeground(found ? filterFontColor : defaultFontColor);
  item->setBackground(found ? filterBackgroundColor : defaultBackgroundColor);
  model->itemFromIndex(item->index().siblingAtColumn(NodeTreeColumns::TypeColumn))->setBackground(found ? filterBackgroundColor : defaultBackgroundColor);
  model->itemFromIndex(item->index().siblingAtColumn(NodeTreeColumns::ValueColumn))->setBackground(found ? filterBackgroundColor : defaultBackgroundColor);

  QStandardItem* parent = model->itemFromIndex(item->index().parent());
  while(parent != nullptr) {
    if (parent->data(USERROLE_FILTER).toBool())
      break;

    parent->setForeground(found ? filterFontColor : defaultFontColor);
    parent->setBackground(found ? filterContentBackgroundColor : defaultBackgroundColor);
    model->itemFromIndex(parent->index().siblingAtColumn(NodeTreeColumns::TypeColumn))->setBackground(found ? filterContentBackgroundColor : defaultBackgroundColor);
    model->itemFromIndex(parent->index().siblingAtColumn(NodeTreeColumns::ValueColumn))->setBackground(found ? filterContentBackgroundColor : defaultBackgroundColor);
    parent = parent->parent();
  }
}

bool PrefabItem::isFilterMarked() const
{
  if (!hasValidEditor())
    return false;

  return index.data(USERROLE_FILTER).toBool();
}

bool PrefabItem::isSpline() const
{
  return (curveIndex.isValid() ||
          data.name == "ControlCurve");
}

bool PrefabItem::hasValidEditor() const
{
  return (editor != nullptr &&
          model != nullptr &&
          index.isValid());
}

bool PrefabItem::hasValidEditor(const QModelIndex index) const
{
  return (editor != nullptr &&
          model != nullptr &&
          index.isValid());
}

QModelIndex PrefabItem::getPropertyValueIndex(const QString propertyName) const
{
  if (!hasValidEditor())
    return QModelIndex();

  // Search for a property with the given name and return its value index
  for(int i = 0; i < model->rowCount(index); ++i) {
    const QModelIndex keyIndex = model->index(i, NodeTreeColumns::KeyColumn, index);
    if (keyIndex.isValid() && keyIndex.data(Qt::DisplayRole) == propertyName)
      return keyIndex.siblingAtColumn(NodeTreeColumns::ValueColumn);
  }

  return QModelIndex();
}

void PrefabItem::setValue(const QString propertyName, const QVariant value) {
  if (!hasValidEditor())
    return;

  setModified();

  // Search for a property with the given name and update it
  QModelIndex propertyIndex = getPropertyValueIndex(propertyName);
  if (propertyIndex.isValid())
    model->setData(propertyIndex, value);
}

void PrefabItem::setValueInModel(const int row, const QModelIndex index, QVariant value)
{
  QModelIndex item = model->index(row, NodeTreeColumns::ValueColumn, index);
  if (item.isValid())
    model->setData(item, value);
}

