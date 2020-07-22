#include "nodeeditormanager.h"

NodeEditorManager::NodeEditorManager(QTabWidget &widget) :
  tabWdiget(widget)
{

}

void NodeEditorManager::addEditor(const QVector<PrefabData> prefabs, const TrackData trackData)
{
  // Create a new editor and pass the prefabs
 NodeEditor* newEditor = new NodeEditor();
 newEditor->setPrefabs(prefabs);

 // Import the json from the loaded track
 try {
   newEditor->importJsonData(&trackData.value);
 } catch (VeloToolkitException& e) {
   e.Message();
 }

 // Create a new treeview and attach it to the new editor
 QTreeView* newTreeView = new QTreeView(&tabWdiget);
 newTreeView->setModel(&newEditor->getFilteredModel());
 newTreeView->header()->setSectionResizeMode(NodeTreeColumns::KeyColumn, QHeaderView::ResizeToContents);
 if (!viewNodeTypeColumn)
   newTreeView->hideColumn(NodeTreeColumns::TypeColumn);

 // Add the track, editor and treeview to the according list
 loadedTracks.append(trackData);
 loadedEditors.append(newEditor);
 treeViews.append(newTreeView);
}

NodeEditor* NodeEditorManager::getNodeEditor() const
{
  const int currentIndex = tabWdiget.currentIndex();
  if (currentIndex == -1) {
    QMessageBox::warning(nullptr, QObject::tr("No track opened!"), QObject::tr("You have to open a track first."));
    return nullptr;
  }

  if (currentIndex >= loadedEditors.count())
    return nullptr;

  return loadedEditors[currentIndex];
}

void NodeEditorManager::setViewNodeType(const bool enabled)
{
  viewNodeTypeColumn = enabled;

  foreach(QTreeView* treeView, treeViews) {
    if (enabled)
      treeView->showColumn(NodeTreeColumns::TypeColumn);
    else
      treeView->hideColumn(NodeTreeColumns::TypeColumn);
  }
}
