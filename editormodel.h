#ifndef EDITORMODEL_H
#define EDITORMODEL_H

#include <QAbstractItemModel>
#include <QStandardItem>
#include <QVector>

#include "editormodelitem.h"
#include "editorobject.h"
#include "track.h"

enum class EditorModelColumns {
  Name = 0,
  PositionX = 1,
  PositionY = 2,
  PositionZ = 3,
  RotationW = 4,
  RotationX = 5,
  RotationY = 6,
  RotationZ = 7,
  ScalingX = 8,
  ScalingY = 9,
  ScalingZ = 10,
};

class Track;
class EditorModelItem;
class EditorObject;

class EditorModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  EditorModel(const Track* track = nullptr, QObject* parent = nullptr);
  ~EditorModel() override;

  void loadTrack(const Track* track);

  int                     columnCount(const QModelIndex& parent = QModelIndex()) const override;
  QVariant                data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags           flags(const QModelIndex& index) const override;
  QBrush                  getFilterFontColor() const;
  QBrush                  getFilterBackgroundColor() const;
  QBrush                  getFilterContentBackgroundColor() const;
  QBrush                  getFilterContentFontColor() const;
  QVariant                headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
  QModelIndex             index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  bool                    isEditable(const QModelIndex& keyIndex) const;
  EditorModelItem*        itemFromIndex(const QModelIndex index) const;
  QList<EditorModelItem*> itemsFromIndex(const QModelIndex index) const;
  QList<EditorModelItem*> itemsFromIndexList(const QModelIndexList indexList) const;
  QModelIndex             parent(const QModelIndex& index) const override;
  EditorModelItem*        getRootItem();
  int                     rowCount(const QModelIndex& parent = QModelIndex()) const override;
  bool                    setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
  void                    setFilterFontColor(const QBrush& value);
  void                    setFilterBackgroundColor(const QBrush& value);
  void                    setFilterContentBackgroundColor(const QBrush& value);
  void                    setFilterContentFontColor(const QBrush& value);

private:
  EditorModelItem* rootItem;
  QVector<EditorModelItem*> modelItems;

  QBrush filterFontColor = QBrush(QColor(Qt::black));
  QBrush filterBackgroundColor = QBrush(QColor(254, 203, 137));
  QBrush filterContentBackgroundColor = QBrush(QColor(192, 192, 192));
  QBrush filterContentFontColor = QBrush(Qt::black);

  EditorModelItem* getModelItem(const QModelIndex& index) const;
};

#endif // EDITORMODEL_H
