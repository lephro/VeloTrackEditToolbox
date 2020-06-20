#include "nodefilter.h"

NodeFilter::NodeFilter(QObject *parent, const FilterTypes filterType, int filterValue, const QString displayValue)
{
  this->setMargin(0);
  this->setSpacing(0);
  this->setStretch(-1, 1);
  this->setParent(parent);
  this->filterType = filterType;
  this->filterValue = filterValue;

  // Create controls
  createFilterLabel(filterType, displayValue);
  createRemoveButton();

  // Add controls
  addWidget(removeFilterButton);
  addWidget(filterLabel);
}

void NodeFilter::destroy()
{
  delete removeFilterButton;
  delete filterLabel;

  this->setEnabled(false);
}

void NodeFilter::removeButtonPushed()
{
  // Remove button was pushed, so fire the remove signal!
  emit removeReleased(this);
}

FilterTypes NodeFilter::getFilterType() const
{
  return filterType;
}

 NodeFilter::getFilterValue() const
{
  return filterValue;
}

QSize NodeFilter::sizeHint() const
{
  QSize size;
  size.setHeight(std::max(removeFilterButton->sizeHint().height(), filterLabel->sizeHint().height()));
  size.setWidth(removeFilterButton->maximumWidth() + filterLabel->sizeHint().width());
  return size;
}

void NodeFilter::createFilterLabel(const FilterTypes type, const QString displayValue)
{
  // Set a description depending on the type
  QString filterDesc;
  switch (type) {
  case FilterTypes::Object:
    filterDesc = QObject::tr("Object: ");
    break;
  case FilterTypes::Position:
    filterDesc = QObject::tr("Position: ");
    break;
  case FilterTypes::Rotation:
    filterDesc = QObject::tr("Rotation: ");
    break;
  case FilterTypes::Scaling:
    filterDesc = QObject::tr("Scaling: ");
    break;
  case FilterTypes::GateNo:
    filterDesc = QObject::tr("Gate No.: ");
    break;
  case FilterTypes::IsOnSpline:
    filterDesc = QObject::tr("is on Spline");
    break;
  case FilterTypes::IsDublicate:
    filterDesc = QObject::tr("is dublicate");
    break;
  }

  // Create a label describing the filter
  filterLabel = new QLabel(this->widget());
  filterLabel->setText(filterDesc + displayValue);
  //filterLabel->setMaximumWidth(100);
  filterLabel->setAlignment(Qt::AlignLeft);
  filterLabel->setAlignment(Qt::AlignVCenter);
  filterLabel->setStyleSheet("background-color: rgb(240,240,240)");
  QSizePolicy policy = filterLabel->sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Ignored);
  filterLabel->setSizePolicy(policy);

}

void NodeFilter::createRemoveButton()
{
  // Create a nice push button to remove the filter
  removeFilterButton = new QPushButton(this->widget());
  removeFilterButton->setMaximumWidth(20);
  removeFilterButton->setText("X");
  //removeFilterButton->setStyleSheet("color: red; border: 1");
  QSizePolicy policy = removeFilterButton->sizePolicy();
  policy.setHeightForWidth(true);
  policy.setHorizontalPolicy(QSizePolicy::Ignored);
  removeFilterButton->setSizePolicy(policy);

  // Hook up our removeButtonPushed function to the button released event
  connect(removeFilterButton, SIGNAL(released()), this, SLOT(removeButtonPushed()));
}
