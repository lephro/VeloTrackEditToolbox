#include "searchfilterlayout.h"

SearchFilterLayout::SearchFilterLayout(QWidget *parent)
{
  setParent(parent);
}

void SearchFilterLayout::addFilter(const FilterTypes filterType, int value, const QString displayValue)
{
  // Dont set any filter twice except position, rotation and scaling
  // which can be set 3x (each) if the differ
  int posFilterCount = 0;
  int rotFilterCount = 0;
  int scaFilterCount = 0;
  foreach(NodeFilter* setFilter, filterList) {
    if (setFilter->getFilterType() == filterType) {
      switch (filterType) {
      case FilterTypes::Position:
        posFilterCount++;
        if (posFilterCount == 3)
          return;
        break;
      case FilterTypes::Rotation:
        rotFilterCount++;
        if (rotFilterCount == 3)
          return;
        break;
      case FilterTypes::Scaling:
        scaFilterCount++;
        if (scaFilterCount == 3)
          return;
        break;
      default:
        return;
      }
    }
  }

  // Create a new filter, add it to our list and hook up to the remove signal
  NodeFilter* filter = new NodeFilter(nullptr, filterType, value, displayValue);
  filterList.append(filter);
  connect(filter, SIGNAL(removeReleased(NodeFilter*)), this, SLOT(removeFilter_released(NodeFilter*)));

  // Find a new home for the filter
  doLayout(maxWidth);
}

void SearchFilterLayout::Clear()
{
  // Remove and delete all filter and rows
  while (filterList.count() > 0) {
    NodeFilter* filter = filterList.last();
    filterList.removeAt(filterList.count() - 1);
    filter->destroy();
    delete filter;
  }
  while (rowList.count() > 0) {
    QHBoxLayout* row = rowList.last();
    rowList.removeAt(rowList.count() - 1);
    delete row;
  }
}

QList<NodeFilter *> SearchFilterLayout::getFilterList() const
{
  return filterList;
}

void SearchFilterLayout::setGeometry(const QRect &rect)
{
  // On any geometric change, we do our layout and also pass the values to our base layout
  doLayout(rect);
  QVBoxLayout::setGeometry(rect);
}

void SearchFilterLayout::removeFilter_released(NodeFilter* sender)
{
  // Destory, remove from list and delete
  sender->destroy();
  filterList.removeOne(sender);
  delete sender;

  // Rearrange our filter
  doLayout(maxWidth);
}

void SearchFilterLayout::doLayout(const QRect &rect)
{
  // No filter = no work
  if (filterList.count() == 0)
    return;

  // Check if any row is now too big
  foreach(QHBoxLayout* row, rowList) {
    if (row->sizeHint().width() < (rect.width() + margin())) {
      // Lay the shit out of i
      doLayout(rect.width());
      return;
    }
  }
}

void SearchFilterLayout::doLayout(const int maxWidth)
{
  // No filter = no work
  if (filterList.count() == 0)
    return;

  this->maxWidth = maxWidth;

  // Rebuild the layout
  int width = 0;
  int row = 0;
  QHBoxLayout* curRow = nullptr;
  const int rowCount = rowList.count();
  foreach(NodeFilter* filter, filterList) {
    // Jump to the next row if we reach the end of our column
    width += filter->sizeHint().width();
    if (width > maxWidth) {
      width = filter->sizeHint().width();
      row++;
      curRow = nullptr;
    }

    // Get a row if any exists
    if (row < rowCount)
      curRow = rowList.at(row);

    // Create a new row if we dont have any set, yet
    if (curRow == nullptr) {
      curRow = new QHBoxLayout();
      curRow->insertStretch(-1, 2);
      rowList.append(curRow);
      addLayout(curRow);
    }

    // Remove the filter from the parent if it differs
    if (filter->parent() != nullptr && filter->parent() != curRow) {
      QLayout* parentLayout = static_cast<QLayout*>(filter->parent());
      parentLayout->removeItem(filter);
    }

    // Add the filter to the row if it has been removed
    if (filter->parent() == nullptr)
      curRow->insertLayout(curRow->count() - 1, filter);
  }

  // Remove and delete empty leftover rows
  for(int i = 0; i < rowList.count(); ++i) {
    QHBoxLayout* layout = rowList[i];

    if (layout != nullptr && layout->count() > 0)
      continue;

    rowList.removeAt(i);
    delete layout;
  }
}
