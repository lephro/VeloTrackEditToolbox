#include "editormanager.h"

EditorManager::EditorManager(MainWindow& mainWindow, QTabWidget &widget) :
  mainWindow(mainWindow),
  tabWdiget(widget)
{
  // Create the context menu of the node editor
  nodeEditorContextMenu.addAction(QIcon(":/icons/filter"), tr("Add to filter"), this, SLOT(onNodeEditorContextMenuAddToFilterAction()));
  QMenu* addFilterSubMenu = nodeEditorContextMenu.addMenu(QIcon(":/icons/filter-add"), tr("Add property as filter"));
  addFilterSubMenu->addAction(QIcon(":/icons/object"), tr("Object"), this, SLOT(onNodeEditorContextMenuAddObjectAsFilterAction()));
  addFilterSubMenu->addAction(QIcon(":/icons/coordinate-system"), tr("Position"), this, SLOT(onNodeEditorContextMenuAddPositionAsFilterAction()));
  addFilterSubMenu->addAction(QIcon(":/icons/rotation"), tr("Rotation"), this, SLOT(onNodeEditorContextMenuAddRotationAsFilterAction()));
  addFilterSubMenu->addAction(QIcon(":/icons/resize"), tr("Scaling"), this, SLOT(onNodeEditorContextMenuAddScaleAsFilterAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/copy"), tr("D&ublicate"), this, SLOT(onNodeEditorContextMenuDublicateAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/copy-add"), tr("&Mass dublicate"), this, SLOT(onNodeEditorContextMenuMassDuplicateAction()));
  nodeEditorContextMenu.addAction(QIcon(":/icons/delete"), tr("&Delete"), this, SLOT(onNodeEditorContextMenuDeleteAction()));

  setParent(&mainWindow);
}

void EditorManager::addEditor(const QVector<PrefabData>& prefabs, const TrackData trackData)
{
  Track* track = nullptr;
  VeloDataParser parser;
  try {
    track = &parser.parseTrack(prefabs, trackData);
  } catch (VeloToolkitException& e) {
    e.Message();
    return;
  }

  if (track->getObjectCount() == 0) {
    delete track;
    return;
  }

  // Create a new editor and pass the parsed track
  NodeEditor* newEditor = new NodeEditor(*track);
  EditorModel& editorModel = newEditor->getEditorModel();
  editorModel.setFilterBackgroundColor(filterColor);
  editorModel.setFilterContentBackgroundColor(filterParentColor);
  editorModel.setFilterContentFontColor(filterParentFontColor);
  editorModel.setFilterFontColor(filterFontColor);

  QTreeView* newTreeView = &newEditor->getTreeView();
  newTreeView->setModel(&newEditor->getFilteredModel());
  newTreeView->header()->setSectionResizeMode(static_cast<int>(EditorModelColumns::Name), QHeaderView::ResizeToContents);
  newTreeView->setIndentation(15);
  newTreeView->setSortingEnabled(false);
  newTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

  // Hook up our delegations
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::Name), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::PositionX), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::PositionY), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::PositionZ), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::RotationW), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::RotationX), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::RotationY), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::RotationZ), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::ScalingX), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::ScalingY), new JsonTreeViewItemDelegate(nullptr, newEditor));
  newTreeView->setItemDelegateForColumn(static_cast<int>(EditorModelColumns::ScalingZ), new JsonTreeViewItemDelegate(nullptr, newEditor));


  // Connect the context menu of the node editor
  newTreeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(newTreeView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(onNodeEditorContextMenu(const QPoint&)));

  QHBoxLayout* layout = new QHBoxLayout();
  layout->addWidget(newTreeView);

  QWidget* newPage = new QWidget();
  newPage->setLayout(layout);

  // Add the track, editor and treeview to the according list
  editors.append(newEditor);
  editorCount++;

  const int newTabIndex = tabWdiget.addTab(newPage, trackData.name);
  tabWdiget.setCurrentIndex(newTabIndex);

//  // Expand the root nodes
//  for (int i = 0; i < newEditor->getEditorModel().getRootItem()->childCount(); ++i) {
//   newTreeView->expand(newEditor->getFilteredModel().mapFromSource(newEditor->getEditorModel().getRootItem()->child(i)->getIndex()));
//  }
}

void EditorManager::closeEditor(const int index)
{
  if (index < 0 || index >= editorCount)
    return;

  editorCount--;

  NodeEditor* editor = editors[index];
  editors.remove(index);
  delete editor;
}

NodeEditor* EditorManager::getEditor() const
{  
  return getEditor(tabWdiget.currentIndex());
}

NodeEditor* EditorManager::getEditor(const int index) const
{
  if (index < 0 || index >= editors.count())
    return nullptr;

  //qDebug() << "EditorManager::getEditor() -> Index < Count: " << index << " < " << editors.count();

  return editors[index];
}

int EditorManager::getEditorCount() const
{
  return editorCount;
}

void EditorManager::onNodeEditorContextMenu(const QPoint &point)
{
  const NodeEditor* editor = getEditor();
  if (editor == nullptr)
    return;

  const QTreeView& treeView = editor->getTreeView();

  // Check if we can get a index on the click point
  const QModelIndex index = treeView.indexAt(point);
  if (!index.isValid())
    return;

  // Map the point to global space and open the context menu at that point
  nodeEditorContextMenu.exec(treeView.viewport()->mapToGlobal(point));
}

void EditorManager::onNodeEditorContextMenuAddObjectAsFilterAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  EditorModelItem* item = nullptr;
  EditorObject* object = nullptr;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    // Create a new filter and add it to the layout
    mainWindow.addFilter(FilterTypes::Object, FilterMethods::Is, int(object->getId()), object->getData().name);

    mainWindow.updateSearch();
    mainWindow.updateStatusBar();

    // Jump out since only one filter of that type can be set at a time
    return;
  }
}

void EditorManager::onNodeEditorContextMenuAddPositionAsFilterAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  EditorModelItem* item = nullptr;
  EditorObject* object;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    // Create a new filter and add it to the layout
    mainWindow.addFilter(FilterTypes::PositionR, FilterMethods::Is, object->getPositionR());
    mainWindow.addFilter(FilterTypes::PositionG, FilterMethods::Is, object->getPositionG());
    mainWindow.addFilter(FilterTypes::PositionB, FilterMethods::Is, object->getPositionB());

    mainWindow.updateSearch();
    mainWindow.updateStatusBar();

    // Jump out since only one filter of that type can be set at a time
    return;
  }
}

void EditorManager::onNodeEditorContextMenuAddRotationAsFilterAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  EditorModelItem* item = nullptr;
  EditorObject* object;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    // Create a new filter and add it to the layout
    mainWindow.addFilter(FilterTypes::RotationW, FilterMethods::Is, object->getRotationW());
    mainWindow.addFilter(FilterTypes::RotationX, FilterMethods::Is, object->getRotationX());
    mainWindow.addFilter(FilterTypes::RotationY, FilterMethods::Is, object->getRotationY());
    mainWindow.addFilter(FilterTypes::RotationZ, FilterMethods::Is, object->getRotationZ());

    mainWindow.updateSearch();
    mainWindow.updateStatusBar();

    // Jump out since only one filter of that type can be set at a time
    return;
  }
}

void EditorManager::onNodeEditorContextMenuAddScaleAsFilterAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  EditorModelItem* item = nullptr;
  EditorObject* object;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    // Create a new filter and add it to the layout
    mainWindow.addFilter(FilterTypes::ScalingR, FilterMethods::Is, object->getScalingR());
    mainWindow.addFilter(FilterTypes::ScalingG, FilterMethods::Is, object->getScalingG());
    mainWindow.addFilter(FilterTypes::ScalingB, FilterMethods::Is, object->getScalingB());

    mainWindow.updateSearch();
    mainWindow.updateStatusBar();

    // Jump out since only one filter of that type can be set at a time
    return;
  }
}

void EditorManager::onNodeEditorContextMenuAddToFilterAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();

  if (selectedIndexes.count() == 0)
    return;

  EditorModelItem* item = nullptr;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Create a new filter and add it to the layout
    mainWindow.addFilter(item->getIndex());
  }

  mainWindow.updateSearch();
  mainWindow.updateStatusBar();
}

void EditorManager::onNodeEditorContextMenuDeleteAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // We delete from last to first, cause every deletion causes the underlying rows to shift,
  // which renders the selected indexes that point to those invalid.
  QModelIndexList selectedIndexes = nodeEditor->getTreeView().selectionModel()->selectedIndexes();
  EditorModelItem* item = nullptr;
  EditorObject* object;
  while(selectedIndexes.count() > 0) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(selectedIndexes.takeLast());
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    // Delete the root node of the prefab
    nodeEditor->deleteNode(object->getIndex());
  }

  mainWindow.updateSearch();
  mainWindow.updateStatusBar();
}

void EditorManager::onNodeEditorContextMenuDublicateAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // Go through each user selection, determinate the corresponding prefab and dublicate it
  EditorModelItem* item = nullptr;
  EditorObject* object;
  foreach(QModelIndex index, nodeEditor->getTreeView().selectionModel()->selectedIndexes()) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(index);
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    nodeEditor->duplicateObject(object);
  }

  // Update the status bar and filter markings, cause they could have changed
  mainWindow.updateSearch();
  mainWindow.updateStatusBar();

  // In case we duplicated a gate, the gate count has increased,
  // which means we also have to re-evaluate the add-gate-filter spinbox
  // in case its type is currently selected. Otherwise you couldn't select the
  // new gate as a filter.
  mainWindow.updateSearchFilterControls();
}

void EditorManager::onNodeEditorContextMenuMassDuplicateAction()
{
  NodeEditor* nodeEditor = getEditor();
  if (nodeEditor == nullptr)
    return;

  // Get the amount of duplicates we want to create
  bool ok;
  int amount = QInputDialog::getInt(nullptr, tr("Mass dublicate"), tr("Dublicates"), 1, 1, 1000, 1, &ok);
  if (!ok)
    return;

  // Go through each user selection, determinate the corresponding prefab and dublicate it
  EditorModelItem* item = nullptr;
  EditorObject* object;
  foreach(QModelIndex index, nodeEditor->getTreeView().selectionModel()->selectedIndexes()) {
    // Take the last index and check if its valid
    item = nodeEditor->getEditorModel().itemFromIndex(index);
    if (!item || !item->hasObject())
      continue;

    // Get the object that corresponds to that index
    object = item->getObject();

    for (int i = 0; i < amount; ++i) {
     nodeEditor->duplicateObject(object);
    }
  }

  // Update the status bar and filter markings, cause they could have changed
  mainWindow.updateSearch();
  mainWindow.updateStatusBar();

  // In case we duplicated a gate, the gate count has increased,
  // which means we also have to re-evaluate the add-gate-filter spinbox
  // in case its type is currently selected. Otherwise you couldn't select the
  // new gate as a filter.
  mainWindow.updateSearchFilterControls();
}

void EditorManager::setFilterColor(const QColor &value)
{
  filterColor = value;

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.beginGroup("general");
  settings.setValue("filterColorR", value.red());
  settings.setValue("filterColorG", value.green());
  settings.setValue("filterColorB", value.blue());
  settings.endGroup();

  foreach(NodeEditor* editor, editors) {
    editor->getEditorModel().setFilterBackgroundColor(value);
  }
}

void EditorManager::setFilterFontColor(const QColor &value)
{
  filterFontColor = value;

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.beginGroup("general");
  settings.setValue("filterFontColorR", value.red());
  settings.setValue("filterFontColorG", value.green());
  settings.setValue("filterFontColorB", value.blue());
  settings.endGroup();

  foreach(NodeEditor* editor, editors) {
    editor->getEditorModel().setFilterFontColor(value);
  }
}

void EditorManager::setFilterParentColor(const QColor &value)
{
  filterParentColor = value;

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.beginGroup("general");
  settings.setValue("filterParentColorR", value.red());
  settings.setValue("filterParentColorG", value.green());
  settings.setValue("filterParentColorB", value.blue());
  settings.endGroup();

  foreach(NodeEditor* editor, editors) {
    editor->getEditorModel().setFilterContentBackgroundColor(value);
  }
}

void EditorManager::setFilterParentFontColor(const QColor &value)
{
  filterParentFontColor = value;

  QSettings settings("settings.ini", QSettings::IniFormat);
  settings.beginGroup("general");
  settings.setValue("filterParentFontColorR", value.red());
  settings.setValue("filterParentFontColorG", value.green());
  settings.setValue("filterParentFontColorB", value.blue());
  settings.endGroup();

  foreach(NodeEditor* editor, editors) {
    editor->getEditorModel().setFilterContentFontColor(value);
  }
}
