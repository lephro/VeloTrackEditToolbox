#include "editormodel.h"

EditorModel::EditorModel(const Track* track, QObject* parent)
  : QAbstractItemModel(parent)
{  
  const QSettings settings("settings.ini", QSettings::IniFormat);
  filterBackgroundColor = QColor(settings.value("general/filterColorR", 226).toInt(),
                                 settings.value("general/filterColorG", 128).toInt(),
                                 settings.value("general/filterColorB", 53).toInt());
  filterFontColor = QColor(settings.value("general/filterFontColorR", 226).toInt(),
                           settings.value("general/filterFontColorG", 128).toInt(),
                           settings.value("general/filterFontColorB", 53).toInt());
  filterContentBackgroundColor = QColor(settings.value("general/filterParentColorR", 226).toInt(),
                                        settings.value("general/filterParentColorG", 128).toInt(),
                                        settings.value("general/filterParentColorB", 53).toInt());
  filterContentFontColor = QColor(settings.value("general/filterParentFontColorR", 226).toInt(),
                                  settings.value("general/filterParentFontColorG", 128).toInt(),
                                  settings.value("general/filterParentFontColorB", 53).toInt());

  rootItem = new EditorModelItem();

  loadTrack(track);
}

EditorModel::~EditorModel()
{
  while (!modelItems.isEmpty()) {
    EditorModelItem* item = modelItems.takeLast();
    delete item;
  }
  delete rootItem;
}

EditorModelItem* EditorModel::getModelItem(const QModelIndex& index) const
{
  if (!index.isValid())
    return rootItem;

  return static_cast<EditorModelItem*>(index.internalPointer());
}

int EditorModel::columnCount(const QModelIndex& parent) const
{
  Q_UNUSED(parent)
  return 1;
}

QVariant EditorModel::data(const QModelIndex& index, int role) const
{
  if (!index.isValid())
    return QVariant();

  EditorModelItem* item = getModelItem(index);
  if (item && item->isFilterMarked()) {
    switch (role) {
    case Qt::BackgroundRole:
      return QVariant(QColor(filterBackgroundColor.color()));
    case Qt::ForegroundRole:
      return QVariant(QColor(filterFontColor.color()));
    default:
      break;
    }
  }

  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();

  if (item)
    return item->getModelData(EditorModelColumns(index.column()));

  return QVariant();
}

Qt::ItemFlags EditorModel::flags(const QModelIndex& index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  switch (EditorModelColumns(index.column())) {
  case EditorModelColumns::Name:
    return QAbstractItemModel::flags(index);
  case EditorModelColumns::PositionX:
  case EditorModelColumns::PositionY:
  case EditorModelColumns::PositionZ:
  case EditorModelColumns::RotationW:
  case EditorModelColumns::RotationX:
  case EditorModelColumns::RotationY:
  case EditorModelColumns::RotationZ:
  case EditorModelColumns::ScalingX:
  case EditorModelColumns::ScalingY:
  case EditorModelColumns::ScalingZ:
    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
  }

  return QAbstractItemModel::flags(index);
}

QBrush EditorModel::getFilterBackgroundColor() const
{
  return filterBackgroundColor;
}
QBrush EditorModel::getFilterContentBackgroundColor() const
{
  return filterContentBackgroundColor;
}
QBrush EditorModel::getFilterContentFontColor() const
{
  return filterContentFontColor;
}
QBrush EditorModel::getFilterFontColor() const
{
  return filterFontColor;
}

QVariant EditorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (EditorModelColumns(section)) {
    case EditorModelColumns::Name:
      return tr("Name");
    case EditorModelColumns::PositionX:
      return tr("Position X");
    case EditorModelColumns::PositionY:
      return tr("Position Y");
    case EditorModelColumns::PositionZ:
      return tr("Position Z");
    case EditorModelColumns::RotationW:
      return tr("Rotation W");
    case EditorModelColumns::RotationX:
      return tr("Rotation X");
    case EditorModelColumns::RotationY:
      return tr("Rotation Y");
    case EditorModelColumns::RotationZ:
      return tr("Rotation Z");
    case EditorModelColumns::ScalingX:
      return tr("Scaling X");
    case EditorModelColumns::ScalingY:
      return tr("Scaling Y");
    case EditorModelColumns::ScalingZ:
      return tr("Scaling Z");
    }
  }

  return QVariant();
}

QModelIndex EditorModel::index(int row, int column, const QModelIndex& parent) const
{
  if (parent.isValid() && parent.column() != 0)
    return QModelIndex();

  EditorModelItem* parentItem = getModelItem(parent);
  if (!parentItem)
    return QModelIndex();

  EditorModelItem* childItem = parentItem->child(row);
  if (childItem)
    return createIndex(row, column, childItem);

  return QModelIndex();
}

bool EditorModel::isEditable(const QModelIndex& keyIndex) const
{
  const EditorModelItem* item = itemFromIndex(keyIndex);
  if (!item || !item->hasObject())
    return false;

  EditorObject* object =  item->getObject();

  const PrefabData& prefab = object->getData();

  if (NodeEditor::isStartGrid(prefab))
    return false;

  if (prefab.id > 0) {
    return ((prefab.name != "CtrlParent") &&
            (prefab.name != "ControlCurve") &&
            (prefab.name != "ControlPoint"));
  } else if (item->getObject()->getStart()) {
    return false;
  } else if (item->getObject()->getFinish()) {
    return false;
  }

  return true;
}

EditorModelItem* EditorModel::itemFromIndex(const QModelIndex index) const
{
  return getModelItem(index);
}

QList<EditorModelItem*> EditorModel::itemsFromIndex(const QModelIndex index) const
{
  QList<EditorModelItem*> foundPrefabs;
  EditorModelItem* item = getModelItem(index);

  if (!item)
    return foundPrefabs;

  foundPrefabs.append(item);
  for(int i = 0; i < item->childCount(); ++i) {
    EditorModelItem* child = item->child(i);
    if (child->hasObject())
      foundPrefabs.append(child);
  }

  return foundPrefabs;
}

QList<EditorModelItem*> EditorModel::itemsFromIndexList(const QModelIndexList indexList) const
{
  QList<EditorModelItem*> foundPrefabs;
  foreach(QModelIndex index, indexList) {
    foundPrefabs += itemsFromIndex(index);
  }

  return foundPrefabs;
}

void EditorModel::loadTrack(const Track* track)
{
  while (!modelItems.isEmpty()) {
    delete modelItems.takeLast();
  }

  if (!track)
    return;

  foreach(EditorObject* object, track->getObjects()) {
    EditorModelItem* item = new EditorModelItem(object, this);
    modelItems.append(item);
    rootItem->addChild(*item);
    if (object->getSplineControls().count() > 0) {
      EditorModelItem* controlsRoot = new EditorModelItem();
      controlsRoot->setText(tr("Controls"));
      EditorModelItem* test = new EditorModelItem();
      test->setText(tr("test"));
      controlsRoot->addChild(*test);
      modelItems.append(controlsRoot);
      modelItems.append(test);
      foreach(EditorObject* child, object->getSplineControls()) {
        EditorModelItem* childItem = new EditorModelItem(child, controlsRoot);
        modelItems.append(childItem);
        controlsRoot->addChild(*childItem);
      }
      item->addChild(*controlsRoot);
    }
    if (object->getSplineObjects().count() > 0) {
      EditorModelItem* objectsRoot = new EditorModelItem();
      objectsRoot->setText(tr("Objects"));
      modelItems.append(objectsRoot);
      foreach(EditorObject* child, object->getSplineObjects()) {
        EditorModelItem* childItem = new EditorModelItem(child, objectsRoot);
        objectsRoot->addChild(*childItem);
        modelItems.append(childItem);
      }
      item->addChild(*objectsRoot);
    }
    if (object->getSplineParents().count() > 0) {
      EditorModelItem* objectParentsRoot = new EditorModelItem();
      objectParentsRoot->setText(tr("Object Parents"));
      modelItems.append(objectParentsRoot);
      foreach(EditorObject* child, object->getSplineParents()) {
        EditorModelItem* childItem = new EditorModelItem(child, objectParentsRoot);
        objectParentsRoot->addChild(*childItem);
        modelItems.append(childItem);
      }
      item->addChild(*objectParentsRoot);
    }
  }
}

QModelIndex EditorModel::parent(const QModelIndex& index) const
{
  if (!index.isValid())
    return QModelIndex();

  EditorModelItem* parentItem = getModelItem(index)->getParentItem();

  if (!parentItem || parentItem->getIndex() == rootItem->getIndex())
    return QModelIndex();

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

EditorModelItem *EditorModel::getRootItem()
{
  return rootItem;
}

int EditorModel::rowCount(const QModelIndex &parent) const
{
  if (parent.column() > 0)
    return 0;

  return getModelItem(parent)->childCount();
}

bool EditorModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  if (role != Qt::EditRole)
    return false;

  EditorModelItem* object = getModelItem(index);
  if (!object)
    return false;

  const bool success = object->setModelData(EditorModelColumns(index.column()), value);
  if (success)
    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

  return success;
}

void EditorModel::setFilterBackgroundColor(const QBrush& value)
{
  filterBackgroundColor = value;
}

void EditorModel::setFilterContentBackgroundColor(const QBrush& value)
{
  filterContentBackgroundColor = value;
}

void EditorModel::setFilterContentFontColor(const QBrush& value)
{
  filterContentFontColor = value;
}

void EditorModel::setFilterFontColor(const QBrush& value)
{
  filterFontColor = value;
}


