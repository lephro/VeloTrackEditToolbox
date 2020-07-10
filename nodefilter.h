#ifndef FILTERNODEWIDGET_H
#define FILTERNODEWIDGET_H

#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QModelIndexList>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QStyle>
#include <QVariant>
#include <QWidget>

enum FilterTypes {
  Object      = 0,
  AnyPosition = 1,
  PositionR   = 2,
  PositionG   = 3,
  PositionB   = 4,
  AnyRotation = 5,
  RotationW   = 6,
  RotationX   = 7,
  RotationY   = 8,
  RotationZ   = 9,
  AnyScaling  = 10,
  ScalingR    = 11,
  ScalingG    = 12,
  ScalingB    = 13,
  GateNo      = 14,
  IsDublicate = 15,
  IsOnSpline  = 16,
  CustomIndex = 255
};

enum FilterMethods {
  Contains = 0,
  Is = 1,
  SmallerThan = 2,
  BiggerThan = 3,
};

class NodeFilter : public QHBoxLayout
{
  Q_OBJECT

public:
  explicit NodeFilter(const QModelIndex& customIndex, QObject *parent = nullptr);
  explicit NodeFilter(const FilterTypes filterType, const FilterMethods filterMethod, const int filterValue, const QString filterDisplayValue = "", QObject *parent = nullptr);
  explicit NodeFilter(const FilterTypes filterType, const FilterMethods filterMethod, const int filterValue, const QString filterDisplayValue, const QModelIndex customIndex, QObject *parent = nullptr);

  void addCustomIndex(const QModelIndex& index);

  bool containsCustomIndex(const QModelIndex& index);

  void destroy();    

  QModelIndexList getCustomIndexList() const;
  FilterTypes getFilterType() const;
  FilterMethods getFilterMethod() const;
  int getFilterValue() const;
  QString getFilterDisplayValue() const;

  QSize sizeHint() const override;    

  void setFilterType(const FilterTypes &value);
  void setFilterMethod(const FilterMethods &value);
  void setFilterValue(const int value);
  void setFilterDisplayValue(const QString);

  static QString getFilterMethodDescription(const FilterMethods method);
  static FilterTypes getFilterTypeByDescription(const QString descrption);
  static QString getDescriptionFromFilterType(const FilterTypes type);

  void removeCustomIndex(const QModelIndex &index);
signals:
  void removeReleased(NodeFilter*);

private slots:
  void removeButtonPushed();

private:
  FilterTypes filterType;
  FilterMethods filterMethod;
  QPushButton* removeFilterButton;
  QLabel* filterLabel;
  int filterValue;
  QString filterDisplayValue;
  QModelIndexList customIndexList;

  void createFilterLabel();
  void createRemoveButton();
  void updateFilterLabel();
};

#endif // FILTERNODEWIDGET_H
