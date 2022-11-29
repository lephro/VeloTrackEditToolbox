#include "delegates.h"

JsonTreeViewItemDelegate::JsonTreeViewItemDelegate(QObject* parent, NodeEditor* dataParser)
  : QStyledItemDelegate(parent)
{
  this->nodeEditor = dataParser;
}

QWidget* JsonTreeViewItemDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  Q_UNUSED(option)

  if (!index.isValid())
    return nullptr;

  const FilterProxyModel* proxyModel = static_cast<const FilterProxyModel*>(index.model());
  const QModelIndex baseIndex = proxyModel->mapToSource(index);
  const EditorModel* model = static_cast<const EditorModel*>(baseIndex.model());

  QWidget* editor = nullptr;
  EditorModelItem* item = model->itemFromIndex(baseIndex);
  if (!item)
    return nullptr;

  if (!item->hasObject() || !item->getObject()->isEditable())
    return nullptr;

  QComboBox* comboBox = nullptr;
  switch (EditorModelColumns(index.column())) {
  case EditorModelColumns::Name:
    editor = new QComboBox(parent);
    comboBox = static_cast<QComboBox*>(editor);
    comboBox->setFrame(false);
    comboBox->setMinimumWidth(200);
    comboBox->setMaximumWidth(300);
    break;
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
    editor = new QSpinBox(parent);
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setFrame(false);
    spinBox->setMinimumWidth(60);
    spinBox->setMaximumWidth(100);
    spinBox->setRange(-999999, 999999);
    break;
  }
  return editor;
}

void JsonTreeViewItemDelegate::setEditorData(QWidget* editor, const QModelIndex& valueIndex) const
{
  const FilterProxyModel* proxyModel = static_cast<const FilterProxyModel*>(valueIndex.model());
  const QModelIndex baseIndex = proxyModel->mapToSource(valueIndex);
  const EditorModel* model = static_cast<const EditorModel*>(baseIndex.model());

  EditorModelItem* item = model->itemFromIndex(baseIndex);
  if (!item)
    return;

  if (!item->hasObject())
    return;

  EditorObject* object = item->getObject();

  PrefabData selectedPrefab = object->getData();

  QComboBox* comboBox = nullptr;
  QSpinBox* spinBox = nullptr;

  int index = 0;

  switch (EditorModelColumns(valueIndex.column())) {
  case EditorModelColumns::Name:
    comboBox = static_cast<QComboBox*>(editor);
    index = 0;
    for (int i = 0; i < nodeEditor->getAllPrefabData().size(); ++i) {
      PrefabData prefab = nodeEditor->getAllPrefabData().value(i);
      if (prefab.gate == selectedPrefab.gate) {
        QVariant var;
        var.setValue(prefab);
        comboBox->insertItem(index, prefab.name, var);
        if (prefab.id == selectedPrefab.id) {
          comboBox->setCurrentIndex(index);
        }
        index++;
      }
    }
    break;
  case EditorModelColumns::PositionX:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getPositionR());
    break;
  case EditorModelColumns::PositionY:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getPositionG());
    break;
  case EditorModelColumns::PositionZ:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getPositionB());
    break;
  case EditorModelColumns::RotationW:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getRotationW());
    break;
  case EditorModelColumns::RotationX:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getRotationX());
    break;
  case EditorModelColumns::RotationY:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getRotationY());
    break;
  case EditorModelColumns::RotationZ:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getRotationZ());
    break;
  case EditorModelColumns::ScalingX:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getScalingR());
    break;
  case EditorModelColumns::ScalingY:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getScalingG());
    break;
  case EditorModelColumns::ScalingZ:
    spinBox = static_cast<QSpinBox*>(editor);
    spinBox->setValue(object->getScalingB());
    break;
  }
}

void JsonTreeViewItemDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& valueIndex) const
{
  const FilterProxyModel* proxyModel = static_cast<const FilterProxyModel*>(valueIndex.model());
  const QModelIndex baseIndex = proxyModel->mapToSource(valueIndex);
  const EditorModel* editorModel = static_cast<const EditorModel*>(baseIndex.model());
  EditorModelItem* item = editorModel->itemFromIndex(baseIndex);
  if (!item)
    return;

  if (!item->hasObject())
    return;

  EditorObject* object = item->getObject();

  PrefabData selectedPrefab = object->getData();

  QVariant value;
  PrefabData prefab;
  QComboBox* comboBox;
  QSpinBox* spinBox;

  switch (EditorModelColumns(valueIndex.column())) {
  case EditorModelColumns::Name:
    comboBox = static_cast<QComboBox*>(editor);
    prefab = comboBox->currentData(Qt::UserRole).value<PrefabData>();
    object->getData() = prefab;
    model->setData(valueIndex, value, Qt::UserRole);
    value = prefab.name;
    break;
  case EditorModelColumns::PositionX:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setPositionR(spinBox->value());
    break;
  case EditorModelColumns::PositionY:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setPositionG(spinBox->value());
    break;
  case EditorModelColumns::PositionZ:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setPositionB(spinBox->value());
    break;
  case EditorModelColumns::RotationW:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setRotationW(spinBox->value());
    break;
  case EditorModelColumns::RotationX:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setRotationX(spinBox->value());
    break;
  case EditorModelColumns::RotationY:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setRotationY(spinBox->value());
    break;
  case EditorModelColumns::RotationZ:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setRotationZ(spinBox->value());
    break;
  case EditorModelColumns::ScalingX:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setScalingR(spinBox->value());
    break;
  case EditorModelColumns::ScalingY:
    spinBox = static_cast<QSpinBox*>(editor);
    object->setScalingG(spinBox->value());
    break;
  case EditorModelColumns::ScalingZ:
    QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
    object->setScalingB(spinBox->value());
    break;
  }
}

void JsonTreeViewItemDelegate::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& valueIndex) const
{
  Q_UNUSED(valueIndex)

  editor->setGeometry(option.rect);
}
