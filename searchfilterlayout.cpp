#include "searchfilterlayout.h"

SearchFilterLayout::SearchFilterLayout(QWidget *parent)
{
  setMargin(5);
  setParent(parent);
}

void SearchFilterLayout::addFilter(const QModelIndex &index)
{
  addFilter(FilterTypes::CustomIndex, FilterMethods::Is, 0, "", index);
}

void SearchFilterLayout::addFilter(const FilterTypes filterType, const FilterMethods filterMethod, int filterValue, const QString displayValue, const QModelIndex& customIndex)
{
  FilterMethods method = filterMethod;
  // Objects is always filtered with "is"
  if (filterType == FilterTypes::Object)
    method = FilterMethods::Is;

  // If a filter with a given method is already set, we update it and jump out
  foreach(NodeFilter* setFilter, filterList) {
    // Seek for other filter of the same type and method
    if (setFilter->getFilterType() != filterType)
      continue;

    if (setFilter->getFilterMethod() != method)
      continue;

    // Filter must be unique if its not a custom index
    if (filterType != FilterTypes::CustomIndex && setFilter->getFilterValue() == filterValue)
      return;

    // Unlimited contains-filter
    if (setFilter->getFilterMethod() == FilterMethods::Contains)
      continue;

    // This filter is already set, just update it and jump out
    if (filterType == FilterTypes::CustomIndex) {
      setFilter->addCustomIndex(customIndex);
    } else {
      setFilter->setFilterValue(filterValue);
      setFilter->setFilterDisplayValue(displayValue);
    }
    return;
  }

  // Create a new filter, add it to our list and hook up to the remove signal
  NodeFilter* filter = new NodeFilter(filterType, method, filterValue, displayValue, customIndex);
  filterList.append(filter);
  connect(filter, SIGNAL(removeReleased(NodeFilter*)), this, SLOT(removeFilter_released(NodeFilter*)));

  // Find a new home for the filter
  doLayout(maxWidth);
}

void SearchFilterLayout::clear()
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

QVector<NodeFilter *> SearchFilterLayout::getFilterList() const
{
  return filterList;
}

void SearchFilterLayout::removeCustomIndex(const QModelIndex &customIndex)
{
  foreach(NodeFilter* setFilter, filterList) {
    if (setFilter->getFilterType() != FilterTypes::CustomIndex)
      continue;

    setFilter->removeCustomIndex(customIndex);
    return;
  }
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

  // Emit the filter changed signal
  emit filterChanged();
}

void SearchFilterLayout::doLayout(const QRect &rect)
{
  // No filter = no work
  if (filterList.count() == 0)
    return;

  // Check if any row is now too big
  int marginleft, marginRight;
  getContentsMargins(&marginleft, nullptr, &marginRight, nullptr);
  for(int i = 0; i < rowList.count(); ++i) {
    QHBoxLayout* row = rowList.at(i);
    //qDebug() << "===";
    //qDebug() << "check1" << i << ":" << row->sizeHint().width() << marginleft << marginRight << spacing() << ">" << (rect.width());
    if (row->sizeHint().width() + marginleft + marginRight + spacing() > rect.width()) {
      // Lay the shit out of it
      //qDebug() << "update1" << i << row->sizeHint().width() + marginleft + marginRight + spacing() << (rect.width());
      doLayout(rect.width());
      //qDebug() << "after update1" << i << ":" << row->sizeHint().width() << rect.width();
      return;
    }

    // If there is no next row, we jump out
    if (rowList.count() < i + 2)
      continue;

    // Get the first filter of the next row and check it
    const int firstFilterOfRow = rowList.at(i + 1)->property("firstFilterIndex").toInt();
    if (firstFilterOfRow < 1 || firstFilterOfRow >= filterList.count())
      continue;

    NodeFilter* firstFilter = filterList.at(firstFilterOfRow);
    if (firstFilter == nullptr)
      continue;

    // If the first filter of the next row would still fit in our row with margins, we need to redo the layout
    //qDebug() << "check2" << i << ":" << row->sizeHint().width() << firstFilter->sizeHint().width() << marginleft << marginRight << spacing() << "<" << (rect.width());
    if (row->sizeHint().width() + firstFilter->sizeHint().width() + marginleft + marginRight + spacing() < rect.width()) {
      //qDebug() << "update2" << i << row->sizeHint().width() + spacing() << (rect.width());
      doLayout(rect.width());
      //qDebug() << "after update2" << i << ":" << row->sizeHint().width() << rect.width();
    }
  }
}

void SearchFilterLayout::doLayout(const int maxWidth)
{
  // No filter = no work
  if (filterList.count() == 0)
    return;

  int marginLeft, marginRight;
  getContentsMargins(&marginLeft, nullptr, &marginRight, nullptr);
  this->maxWidth = maxWidth - marginLeft - marginRight;

  // Rebuild the layout
  int width = marginLeft + marginRight;
  int row = 0;
  QHBoxLayout* curRow = nullptr;
  NodeFilter* filter = nullptr;
  for(int i = 0; i < filterList.count(); ++i) {
    // Get the filter and check it
    filter = filterList.at(i);
    if (filter == nullptr)
      continue;

    // Jump to the next row if we reach the end of our column
    width += filter->sizeHint().width() + spacing();
    if (width - spacing() > maxWidth) {
      width = marginLeft + marginRight + filter->sizeHint().width() + spacing();
      row++;
      curRow = nullptr;
    }

    // Get a row if any exists
    if (row < rowList.count())
      curRow = rowList.at(row);

    // Create a new row if we dont have any set, yet
    if (curRow == nullptr) {
      curRow = new QHBoxLayout();      
      curRow->setProperty("firstFilterIndex", i);
      curRow->insertStretch(-1, 2);
      rowList.append(curRow);
      addLayout(curRow);
    }

    // Remove the filter from the parent if the parent differs from its target row
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
