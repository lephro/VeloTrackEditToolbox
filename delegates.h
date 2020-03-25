#ifndef DELEAGTES_H
#define DELEAGTES_H

#include <QComboBox>
#include <QCompleter>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QSpinBox>
#include <QStyledItemDelegate>

#include "velodb.h"
#include "velotrack.h"

class JsonTreeViewItemDelegate  : public QStyledItemDelegate
{
  Q_OBJECT

public:
  JsonTreeViewItemDelegate(QObject* parent = nullptr, VeloTrack* dataParser = nullptr);

  QWidget*  createEditor(QWidget* parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

  // QAbstractItemDelegate interface
public:
  void setEditorData(QWidget* editor, const QModelIndex &valueIndex) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex &valueIndex) const override;
  void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem &option, const QModelIndex &valueIndex) const override;

private:
  VeloTrack* dataParser;
};

class NoEditDelegate: public QStyledItemDelegate {
public:
  NoEditDelegate(QObject* parent = nullptr): QStyledItemDelegate(parent) {}
  virtual QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex &index) const {
    Q_UNUSED(parent)
    Q_UNUSED(option)
    Q_UNUSED(index)
    return nullptr;
  }
};

#endif // DELEAGTES_H
