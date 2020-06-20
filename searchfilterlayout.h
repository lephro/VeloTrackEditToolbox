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

  void addFilter(const FilterTypes filterType, int value, const QString displayValue = "");

  void Clear();

  QList<NodeFilter *> getFilterList() const;

  void setGeometry(const QRect &rect) override;

private slots:
  void removeFilter_released(NodeFilter *sender);

private:
  int maxWidth = 1;
  QList<NodeFilter*> filterList;
  QList<QHBoxLayout*> rowList;

  void doLayout(const QRect &rect);
  void doLayout(const int maxWidth);
};

#endif // SEARCHFILTERLAYOUT_H
