#include "editormodelitem.h"

EditorModelItem::EditorModelItem(EditorObject* object, QObject* parent)
{
  setObject(object);
  setParent(parent);
}

void EditorModelItem::addChild(EditorModelItem& child)
{
  child.setParentItem(this);
  children.append(&child);
}

EditorModelItem* EditorModelItem::child(const int number)
{
  if (number < children.count())
    return children[number];

  return nullptr;
}

int EditorModelItem::childCount() const
{
  return children.count();
}

int EditorModelItem::childNumber() const
{
  int number = 0;
  if (parentItem)
    number = children.indexOf(const_cast<EditorModelItem*>(this));

  return number;
}

void EditorModelItem::removeChild(const int number, const bool deleteChild)
{
  if (number > children.count())
    return;

  if (!deleteChild)
    return;

  delete children.takeAt(number);
}

QVariant EditorModelItem::getModelData(const EditorModelColumns column)
{
  if (hasObject()) {
    switch (column) {
    case EditorModelColumns::Name:
      return object->getData().name;
    case EditorModelColumns::PositionX:
      return object->getPositionR();
    case EditorModelColumns::PositionY:
      return object->getPositionG();
    case EditorModelColumns::PositionZ:
      return object->getPositionB();
    case EditorModelColumns::RotationW:
      return object->getRotationW();
    case EditorModelColumns::RotationX:
      return object->getRotationX();
    case EditorModelColumns::RotationY:
      return object->getRotationY();
    case EditorModelColumns::RotationZ:
      return object->getRotationZ();
    case EditorModelColumns::ScalingX:
      return object->getScalingR();
    case EditorModelColumns::ScalingY:
      return object->getScalingG();
    case EditorModelColumns::ScalingZ:
      return object->getScalingB();
    }
  } else if (column == EditorModelColumns::Name)
    return text;

  return QVariant();
}

bool EditorModelItem::setModelData(const EditorModelColumns column, const QVariant& value)
{
  bool success;
  if (hasObject()) {
    switch (EditorModelColumns(column)) {
    case EditorModelColumns::Name:
      return false;
    case EditorModelColumns::PositionX:
      object->setPositionR(value.toInt(&success));
      return success;
    case EditorModelColumns::PositionY:
      object->setPositionG(value.toInt(&success));
      return success;
    case EditorModelColumns::PositionZ:
      object->setPositionB(value.toInt(&success));
      return success;
    case EditorModelColumns::RotationW:
      object->setRotationW(value.toInt(&success));
      return success;
    case EditorModelColumns::RotationX:
      object->setRotationX(value.toInt(&success));
      return success;
    case EditorModelColumns::RotationY:
      object->setRotationY(value.toInt(&success));
      return success;
    case EditorModelColumns::RotationZ:
      object->setRotationZ(value.toInt(&success));
      return success;
    case EditorModelColumns::ScalingX:
      object->setScalingR(value.toInt(&success));
      return success;
    case EditorModelColumns::ScalingY:
      object->setScalingG(value.toInt(&success));
      return success;
    case EditorModelColumns::ScalingZ:
      object->setScalingB(value.toInt(&success));
      return success;
    }
  } else if (EditorModelColumns(column) == EditorModelColumns::Name) {
    setText(value.toString());
    return true;
  }

  return false;
}

EditorModelItem* EditorModelItem::getParentItem() const
{
  return parentItem;
}

void EditorModelItem::setParentItem(EditorModelItem* parent)
{
  if (parentItem != nullptr)
    parentItem->removeChild(childNumber(), false);

  parentItem = parent;
  setParent(parent);
}

bool EditorModelItem::hasObject() const
{
  return object != nullptr;
}

EditorObject* EditorModelItem::getObject() const
{
  return object;
}

void EditorModelItem::setObject(EditorObject* newObj)
{
  object = newObj;
  if (!object)
    return;

  object->setParentModelItem(this);
}

QString EditorModelItem::getText() const
{
  return text;
}

void EditorModelItem::setText(const QString& text)
{
  this->text = text;
}

QModelIndex EditorModelItem::getIndex() const
{
  return index;
}

void EditorModelItem::setIndex(const QModelIndex& index)
{
  this->index = index;
}

bool EditorModelItem::isModified() const
{
  if (!hasObject())
    return false;

  return object->isModified();
}

void EditorModelItem::setModified(bool modified)
{
  if (!hasObject())
    return;

  object->setModified(modified);
}

bool EditorModelItem::isFilterMarked() const
{
  if (!hasObject())
    return false;

  return object->isFilterMarked();
}

void EditorModelItem::setFilterMarked(const bool marked)
{
  if (!hasObject())
    return;

  object->setFilterMarked(marked);
}
