#include "filterproxymodel.h"

FilterProxyModel::FilterProxyModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{

}

bool FilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  Q_UNUSED(sourceRow)

  // Only show items that have the filter mark set, if the filter is enabled
  if (!searchFilterEnabled)
    return true;

  if (sourceParent.data(USERROLE_FILTER).toBool())
    return true;

  QModelIndex parent = sourceParent.parent();
  while(parent.isValid()) {
    if (parent.data(USERROLE_FILTER).toBool())
      return true;
    parent = parent.parent();
  }

  return false;
}

bool FilterProxyModel::isSearchFilterEnabled() const
{
  return searchFilterEnabled;
}

void FilterProxyModel::setSearchFilter(bool enabled)
{
  searchFilterEnabled = enabled;
  invalidateFilter();
}
