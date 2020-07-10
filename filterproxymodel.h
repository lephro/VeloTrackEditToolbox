#ifndef PREFABSORTFILTERPROXYMODEL_H
#define PREFABSORTFILTERPROXYMODEL_H

#include <QDebug>
#include <QModelIndex>
#include <QStandardItem>
#include <QSortFilterProxyModel>

#define USERROLE_FILTER 1000

class FilterProxyModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  FilterProxyModel(QObject *parent = nullptr);

  bool isSearchFilterEnabled() const;
  void setSearchFilter(bool enabled);

protected:
  bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
  bool searchFilterEnabled = false;
};

#endif // PREFABSORTFILTERPROXYMODEL_H
