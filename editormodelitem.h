#ifndef EDITORMODELITEM_H
#define EDITORMODELITEM_H

#include <QObject>
#include <QModelIndex>
#include "editormodel.h"
#include "editorobject.h"

class EditorObject;
enum class EditorModelColumns;

class EditorModelItem : public QObject
{
  Q_OBJECT

public:
  explicit EditorModelItem(EditorObject* object = nullptr, QObject* parent = nullptr);

  void addChild(EditorModelItem& child);
  EditorModelItem* child(const int number);
  int childCount() const;
  int childNumber() const;
  void removeChild(const int number, const bool deleteChild = true);

  QVariant getModelData(const EditorModelColumns column);
  bool setModelData(const EditorModelColumns column, const QVariant& value);

  EditorModelItem* getParentItem() const;
  void setParentItem(EditorModelItem* parent);

  bool hasObject() const;

  EditorObject* getObject() const;
  void setObject(EditorObject* newObj);

  QString getText() const;
  void setText(const QString& value);

  QModelIndex getIndex() const;
  void setIndex(const QModelIndex& index);

  bool isModified() const;
  void setModified(bool modified);

  bool isFilterMarked() const;
  void setFilterMarked(const bool marked);

signals:

private:
  QModelIndex index;
  QString text;
  EditorObject* object = nullptr;
  QVector<EditorModelItem*> children;
  EditorModelItem* parentItem = nullptr;
};

#endif // EDITORMODELITEM_H
