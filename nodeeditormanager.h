#ifndef NODEEDITORMANAGER_H
#define NODEEDITORMANAGER_H

#include <QHeaderView>
#include <QObject>
#include <QMessageBox>
#include <QTabWidget>
#include <QTreeView>

#include "nodeeditor.h"

class NodeEditorManager : QObject
{
  Q_OBJECT

public:
  NodeEditorManager(QTabWidget& tabWidget);

  void addEditor(const QVector<PrefabData> prefabs, const TrackData trackData);

  NodeEditor* getNodeEditor() const;

  void setViewNodeType(const bool enabled);

private:
  bool viewNodeTypeColumn;

private:
  QTabWidget& tabWdiget;
  QVector<NodeEditor*> loadedEditors;
  QVector<TrackData> loadedTracks;
  QVector<QTreeView*> treeViews;
};

#endif // NODEEDITORMANAGER_H
