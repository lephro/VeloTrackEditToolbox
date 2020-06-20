#ifndef FILTERNODEWIDGET_H
#define FILTERNODEWIDGET_H

#include <QHBoxLayout>
#include <QDebug>
#include <QLabel>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QStyle>
#include <QVariant>
#include <QWidget>

enum FilterTypes {
  Object = 0,
  Position = 1,
  Rotation = 2,
  Scaling = 3,
  GateNo = 4,
  IsDublicate = 5,
  IsOnSpline = 6
};

class NodeFilter : public QHBoxLayout
{
  Q_OBJECT

public:
  explicit NodeFilter(QObject *parent = nullptr, const FilterTypes filterType = FilterTypes::Object, int value = 0, const QString displayValue = "");

  void destroy();

  FilterTypes getFilterType() const;
  int getFilterValue() const;

  QSize sizeHint() const override;

signals:
  void removeReleased(NodeFilter*);

private slots:
  void removeButtonPushed();

private:
  FilterTypes filterType;
  QPushButton* removeFilterButton;
  QLabel* filterLabel;
  int filterValue;

  void createFilterLabel(const FilterTypes filterType, const QString value);
  void createRemoveButton();
};

#endif // FILTERNODEWIDGET_H
