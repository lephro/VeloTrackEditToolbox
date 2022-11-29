#ifndef NODEEDITORMANAGER_H
#define NODEEDITORMANAGER_H

#include <QDebug>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QObject>
#include <QMenu>
#include <QMessageBox>
#include <QTabWidget>
#include <QTreeView>

#include "editormodel.h"
#include "delegates.h"
#include "mainwindow.h"
#include "nodeeditor.h"
#include "velodataparser.h"

class MainWindow;

class NodeEditor;

class EditorManager : public QObject
{
  Q_OBJECT

public:
  EditorManager(MainWindow& mainWindow, QTabWidget& tabWidget);

  void addEditor(const QVector<PrefabData>& prefabs, const TrackData trackData);
  void closeEditor(const int index);

  NodeEditor* getEditor() const;
  NodeEditor* getEditor(const int index) const;
  const QVector<EditorObject>& getSearchResult() const;

  int getEditorCount() const;

  void setFilterColor(const QColor &value);
  void setFilterFontColor(const QColor &value);
  void setFilterParentColor(const QColor &value);
  void setFilterParentFontColor(const QColor &value);

private slots:
  void onNodeEditorContextMenu(const QPoint &point);
  void onNodeEditorContextMenuAddObjectAsFilterAction();
  void onNodeEditorContextMenuAddPositionAsFilterAction();
  void onNodeEditorContextMenuAddRotationAsFilterAction();
  void onNodeEditorContextMenuAddScaleAsFilterAction();
  void onNodeEditorContextMenuAddToFilterAction();
  void onNodeEditorContextMenuDeleteAction();
  void onNodeEditorContextMenuDublicateAction();
  void onNodeEditorContextMenuMassDuplicateAction();

private:  
  const QBrush defaultFontColor = QBrush(Qt::white);
  const QBrush defaultBackgroundColor = QBrush(QColor(255, 255, 255, 0));

  MainWindow& mainWindow;
  QTabWidget& tabWdiget;

  QMenu nodeEditorContextMenu;

  QVector<NodeEditor*> editors;

  int editorCount = 0;

  bool viewNodeTypeColumn = false;

  QColor filterColor;
  QColor filterFontColor;
  QColor filterParentColor;
  QColor filterParentFontColor;
};

#endif // NODEEDITORMANAGER_H
