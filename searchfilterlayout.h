#ifndef SEARCHFILTERLAYOUT_H
#define SEARCHFILTERLAYOUT_H

#include <QDebug>
#include <QHBoxLayout>
#include <QList>
#include <QObject>
#include <QSpacerItem>
#include <QString>
#include <QVariant>
#include <QVBoxLayout>
#include <QWidget>

#include "nodefilter.h"

class SearchFilterLayout : public QVBoxLayout
{
  Q_OBJECT

public:
  SearchFilterLayout(QWidget *parent = nullptr);

  void addFilter(const QModelIndex& index);
  void addFilter(const FilterTypes filterType, const FilterMethods filterMethod, int value, const QString displayValue = "", const QModelIndex &customIndex = QModelIndex());

  void clear();

  QVector<NodeFilter *> getFilterList() const;

  void removeCustomIndex(const QModelIndex& customIndex);

  void setGeometry(const QRect &rect) override;

signals:
  void filterChanged();

private slots:
  void removeFilter_released(NodeFilter *sender);

private:
  int maxWidth = 1;
  QVector<NodeFilter*> filterList;
  QVector<QHBoxLayout*> rowList;

  void doLayout(const QRect &rect);
  void doLayout(const int maxWidth);
};

#endif // SEARCHFILTERLAYOUT_H
