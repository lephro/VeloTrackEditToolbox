#include "delegates.h"

JsonTreeViewItemDelegate::JsonTreeViewItemDelegate(QObject *parent, VeloTrack* dataParser)
  : QStyledItemDelegate(parent)
{
  this->dataParser = dataParser;
}

QWidget *JsonTreeViewItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  Q_UNUSED(option)

  QModelIndex keyIndex = index.siblingAtColumn(NodeTreeColumns::KeyColumn);
  QModelIndex valueIndex = index.siblingAtColumn(NodeTreeColumns::ValueColumn);
  QModelIndex typeIndex = index.siblingAtColumn(NodeTreeColumns::TypeColumn);

  if (!keyIndex.isValid() || !valueIndex.isValid() || !typeIndex.isValid())
    return nullptr;

  if (VeloTrack::isEditableNode(keyIndex))
    return nullptr;

  QWidget* editor = nullptr;
  PrefabData prefab = valueIndex.data(Qt::UserRole).value<PrefabData>();
  if (prefab.id > 0) {
    editor = new QComboBox(parent);
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    comboBox->setFrame(false);
    comboBox->setMinimumWidth(200);
    comboBox->setMaximumWidth(300);    

//    comboBox->setEditable(true);
//    comboBox->setInsertPolicy(QComboBox::NoInsert);

//    QSortFilterProxyModel* proxy = new QSortFilterProxyModel(comboBox);
//    proxy->setSourceModel(comboBox->model());
//    proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
//    proxy->setFilterKeyColumn(comboBox->modelColumn());

//    QCompleter* completer = new QCompleter(comboBox);
//    completer->setCaseSensitivity(Qt::CaseInsensitive);
//    completer->setModel(proxy);
//    completer->setCompletionColumn(comboBox->modelColumn());
//    completer->setCompletionMode(QCompleter::UnfilteredPopupCompletion);

  } else if (typeIndex.data() == "Bool") {
    editor = new QComboBox(parent);
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    comboBox->setFrame(false);
    comboBox->setMinimumWidth(50);
    comboBox->setMaximumWidth(100);
  } else if (typeIndex.data() == "Double") {
    QModelIndex parentKeyItemIndex = keyIndex.parent();
    if (parentKeyItemIndex.isValid() &&
        (parentKeyItemIndex.data() == "weather"))
    {
      editor = new QDoubleSpinBox(parent);
      QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
      spinBox->setFrame(false);
      spinBox->setMinimumWidth(120);
      spinBox->setMaximumWidth(200);
      spinBox->setDecimals(15);
      spinBox->setRange(-99999, 99999);
    } else {
      editor = new QSpinBox(parent);
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      spinBox->setFrame(false);
      spinBox->setMinimumWidth(60);
      spinBox->setMaximumWidth(100);
      if (parentKeyItemIndex.isValid() && parentKeyItemIndex.data() != "Object") {
        spinBox->setRange(-99999, 99999);
      }
      if (keyIndex.data() == "gate") {
        spinBox->setRange(0, dataParser->getGateCount() - 1);
      }
    }
  } else {
    editor = new QLineEdit(parent);
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setFrame(false);
    lineEdit->setMinimumWidth(50);
    lineEdit->setMaximumWidth(300);
  }
  return editor;
}

void JsonTreeViewItemDelegate::setEditorData(QWidget *editor, const QModelIndex &valueIndex) const
{
  QModelIndex keyIndex = valueIndex.siblingAtColumn(NodeTreeColumns::KeyColumn);
  QModelIndex typeIndex = valueIndex.siblingAtColumn(NodeTreeColumns::TypeColumn);

  if (!keyIndex.isValid() || !valueIndex.isValid() || !typeIndex.isValid())
    return;

  PrefabData selectedPrefab = valueIndex.data(Qt::UserRole).value<PrefabData>();
  if (selectedPrefab.id > 0) {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    if (dataParser->getPrefabs() != nullptr) {
      int index = 0;
      for (int i = 0; i < dataParser->getPrefabs()->size(); ++i) {
        PrefabData prefab = dataParser->getPrefabs()->value(i);
        if (selectedPrefab.gate == prefab.gate) {
          QVariant var;
          var.setValue(prefab);
          comboBox->insertItem(index, prefab.name, var);
          if (prefab.id == selectedPrefab.id) {
           comboBox->setCurrentIndex(index);
          }
          index++;
        }
      }
    }
  } else if (typeIndex.data() == "Bool") {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    comboBox->insertItem(0, "false", false);
    comboBox->insertItem(1, "true", true);
    comboBox->setCurrentIndex((valueIndex.data().toBool() == true) ? 1 : 0);
  } else if (typeIndex.data() == "Double") {
    QModelIndex parentKeyIndex = keyIndex.parent();
    if (parentKeyIndex.isValid() &&
        (parentKeyIndex.data() ==  "weather"))
    {
      QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
      spinBox->setValue(valueIndex.data().toDouble());
    } else {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      spinBox->setValue(valueIndex.data().toInt());
    }
  } else {
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    lineEdit->setText(valueIndex.data().toString());
  }
}

void JsonTreeViewItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &valueIndex) const
{
  QModelIndex keyIndex = valueIndex.siblingAtColumn(NodeTreeColumns::KeyColumn);
  QModelIndex typeIndex = valueIndex.siblingAtColumn(NodeTreeColumns::TypeColumn);

  if (!keyIndex.isValid() || !typeIndex.isValid())
    return;

  QVariant value;
  if (keyIndex.data() == "prefab") {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    PrefabData prefab = comboBox->currentData(Qt::UserRole).value<PrefabData>();
    QModelIndex parentKeyItemIndex = valueIndex.parent();
    if (parentKeyItemIndex.isValid()) {
      model->setData(parentKeyItemIndex, prefab.name, Qt::DisplayRole);
    }
    value.setValue(prefab);
    model->setData(valueIndex, value, Qt::UserRole);
    value = prefab.name;
  } else if (keyIndex.data() == "finish") {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    if ((dataParser != nullptr) && (comboBox->currentIndex() == 1))
      dataParser->resetFinishGates();
    value = (comboBox->currentIndex() == 1);
  } else if (keyIndex.data() == "start") {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    if ((dataParser != nullptr) && (comboBox->currentIndex() == 1))
      dataParser->resetStartGates();
    value = (comboBox->currentIndex() == 1);
  } else if (typeIndex.data() == "Bool") {
    QComboBox* comboBox = static_cast<QComboBox*>(editor);
    value = (comboBox->currentIndex() == 1);
  } else if (typeIndex.data() == "Double") {
    QModelIndex parentKeyItemIndex = keyIndex.parent();
    if (parentKeyItemIndex.isValid() &&
        (parentKeyItemIndex.data() ==  "weather"))
    {
      QDoubleSpinBox* spinBox = static_cast<QDoubleSpinBox*>(editor);
      value = QVariant(spinBox->value());
    } else {
      QSpinBox* spinBox = static_cast<QSpinBox*>(editor);
      if (keyIndex.data() == "gate") {
        dataParser->changeGateOrder(valueIndex.data().toUInt(), unsigned(spinBox->value()));
      }
      value = QVariant(spinBox->value());
    }
  } else {
    QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
    value = QVariant(lineEdit->text());
  }

  model->setData(keyIndex, true, Qt::UserRole);
  model->setData(valueIndex, value, Qt::EditRole);
}

void JsonTreeViewItemDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &valueIndex) const
{
  Q_UNUSED(valueIndex)

  editor->setGeometry(option.rect);
}
