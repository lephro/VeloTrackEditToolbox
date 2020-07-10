#include "nodefilter.h"

NodeFilter::NodeFilter(const QModelIndex &customIndex, QObject *parent) :
  NodeFilter(FilterTypes::CustomIndex, FilterMethods::Is, 0, "", customIndex, parent)
{

}

NodeFilter::NodeFilter(const FilterTypes filterType, const FilterMethods filterMethod, const int filterValue, const QString filterDisplayValue, QObject *parent) :
  NodeFilter(filterType, filterMethod, filterValue, filterDisplayValue, QModelIndex(), parent)
{

}

NodeFilter::NodeFilter(const FilterTypes filterType, const FilterMethods filterMethod, const int filterValue, const QString filterDisplayValue, const QModelIndex customIndex, QObject *parent)
{  
  this->setMargin(0);
  this->setSpacing(0);
  this->setStretch(-1, 1);
  this->setParent(parent);
  this->filterType = filterType;
  this->filterMethod = filterMethod;
  this->filterValue = filterValue;
  this->filterDisplayValue = filterDisplayValue;

  if (filterType == FilterTypes::Object)
    this->filterMethod = FilterMethods::Is;

  if (filterType == FilterTypes::CustomIndex) {
    this->filterDisplayValue = "";
    if (customIndex.isValid())
      customIndexList.append(customIndex);
  }

  // Calculate the display value if it has not been provided
  if (filterType != FilterTypes::IsOnSpline &&
      filterType != FilterTypes::IsDublicate &&
      filterType != FilterTypes::CustomIndex &&
      filterDisplayValue == "")
    this->filterDisplayValue = QString("%1").arg(filterValue);

  // Create controls
  createFilterLabel();
  updateFilterLabel();
  createRemoveButton();

  // Add controls
  addWidget(filterLabel);
  addWidget(removeFilterButton);
}

void NodeFilter::addCustomIndex(const QModelIndex &index)
{
  if (filterType == FilterTypes::CustomIndex && index.isValid())
    customIndexList.append(index);
}

bool NodeFilter::containsCustomIndex(const QModelIndex &index)
{
  return customIndexList.contains(index);
}

void NodeFilter::createFilterLabel()
{
  // Create a label describing the filter
  filterLabel = new QLabel(this->widget());
  filterLabel->setAlignment(Qt::AlignLeft);
  filterLabel->setAlignment(Qt::AlignVCenter);
  //filterLabel->setStyleSheet("background-color: rgb(240,240,240)");
  QSizePolicy policy = filterLabel->sizePolicy();
  policy.setHorizontalPolicy(QSizePolicy::Ignored);
  filterLabel->setSizePolicy(policy);
}

void NodeFilter::createRemoveButton()
{
  // Create a nice push button to remove the filter
  removeFilterButton = new QPushButton(this->widget());
  removeFilterButton->setMinimumSize(QSize(20, 20));
  removeFilterButton->setMaximumSize(QSize(20, 20));
  //removeFilterButton->setIconSize(QSize(20, 20));
  removeFilterButton->setIcon(QIcon(":/icons/delete"));
  removeFilterButton->setStyleSheet("border: 0");
  QSizePolicy policy = removeFilterButton->sizePolicy();
  policy.setHeightForWidth(true);
  policy.setHorizontalPolicy(QSizePolicy::Ignored);
  removeFilterButton->setSizePolicy(policy);

  // Hook up our removeButtonPushed function to the button released event
  connect(removeFilterButton, SIGNAL(released()), this, SLOT(removeButtonPushed()));
}

void NodeFilter::destroy()
{
  delete removeFilterButton;
  delete filterLabel;

  this->setEnabled(false);
}

QModelIndexList NodeFilter::getCustomIndexList() const
{
  return customIndexList;
}

FilterTypes NodeFilter::getFilterType() const
{
  return filterType;
}

FilterMethods NodeFilter::getFilterMethod() const
{
  return filterMethod;
}

int NodeFilter::getFilterValue() const
{
  return filterValue;
}

QString NodeFilter::getFilterDisplayValue() const
{
  return filterDisplayValue;
}

void NodeFilter::removeButtonPushed()
{
  // Remove button was pushed, so fire the remove signal!
  emit removeReleased(this);
}

void NodeFilter::removeCustomIndex(const QModelIndex& index)
{
  customIndexList.removeOne(index);
}

void NodeFilter::setFilterDisplayValue(const QString value)
{
  filterDisplayValue = value;
  updateFilterLabel();
}

void NodeFilter::setFilterMethod(const FilterMethods &value)
{
  filterMethod = value;
  updateFilterLabel();
}

void NodeFilter::setFilterType(const FilterTypes &value)
{
  filterType = value;
  updateFilterLabel();
}

void NodeFilter::setFilterValue(const int value)
{
  filterValue = value;
  updateFilterLabel();
}

QSize NodeFilter::sizeHint() const
{
  QSize size;
  size.setHeight(std::max(removeFilterButton->sizeHint().height(), filterLabel->sizeHint().height()));
  size.setWidth(removeFilterButton->maximumWidth() + filterLabel->sizeHint().width());
  return size;
}

FilterTypes NodeFilter::getFilterTypeByDescription(const QString descrption)
{
  QString desc = descrption.toLower();
  if (desc.indexOf(tr("object")) > -1) {  return FilterTypes::Object; }
  else if (desc == ("position") || desc.indexOf(tr("position any")) > -1) { return FilterTypes::AnyPosition; }
  else if (desc.indexOf(tr("position r")) > -1) { return FilterTypes::PositionR; }
  else if (desc.indexOf(tr("position g")) > -1) { return FilterTypes::PositionG; }
  else if (desc.indexOf(tr("position b")) > -1) { return FilterTypes::PositionB; }
  else if (desc == tr("rotation") || desc == tr("rotation any")) { return FilterTypes::AnyRotation; }
  else if (desc.indexOf(tr("rotation w")) > -1) { return FilterTypes::RotationW; }
  else if (desc.indexOf(tr("rotation x")) > -1) { return FilterTypes::RotationX; }
  else if (desc.indexOf(tr("rotation y")) > -1) { return FilterTypes::RotationY; }
  else if (desc.indexOf(tr("rotation z")) > -1) { return FilterTypes::RotationZ; }
  else if (desc.indexOf(tr("rotation from angle")) > -1) { return FilterTypes::AnyRotation; }
  else if (desc == "scaling" || desc.indexOf("scaling any") > -1) { return FilterTypes::AnyScaling; }
  else if (desc.indexOf(tr("scaling r")) > -1) { return FilterTypes::ScalingR; }
  else if (desc.indexOf(tr("scaling g")) > -1) { return FilterTypes::ScalingG; }
  else if (desc.indexOf(tr("scaling b")) > -1) { return FilterTypes::ScalingB; }
  else if (desc.indexOf(tr("gate no")) > -1) { return FilterTypes::GateNo; }
  else if (desc.indexOf(tr("is on spline")) > -1) { return FilterTypes::IsOnSpline; }
  else if (desc.indexOf(tr("is dublicate")) > -1) { return FilterTypes::IsDublicate; }
  else if (desc.indexOf(tr("manual selection")) > -1) { return FilterTypes::IsDublicate; }
  else { return FilterTypes::Object; }
}

QString NodeFilter::getDescriptionFromFilterType(const FilterTypes type)
{
  // Set a description depending on the type
  QString description = "";
  switch (type) {
  case FilterTypes::Object: description = tr("Object"); break;
  case FilterTypes::AnyPosition: description = tr("Any Position"); break;
  case FilterTypes::PositionR: description = tr("Position R"); break;
  case FilterTypes::PositionG: description = tr("Position G"); break;
  case FilterTypes::PositionB: description = tr("Position B"); break;
  case FilterTypes::AnyRotation: description = tr("Any Rotation"); break;
  case FilterTypes::RotationW: description = tr("Rotation W"); break;
  case FilterTypes::RotationX: description = tr("Rotation X"); break;
  case FilterTypes::RotationY: description = tr("Rotation Y"); break;
  case FilterTypes::RotationZ: description = tr("Rotation Z"); break;
  case FilterTypes::AnyScaling: description = tr("Any Scaling"); break;
  case FilterTypes::ScalingR: description = tr("Scaling R"); break;
  case FilterTypes::ScalingG: description = tr("Scaling G"); break;
  case FilterTypes::ScalingB: description = tr("Scaling B"); break;
  case FilterTypes::GateNo: description = tr("Gate No."); break;
  case FilterTypes::IsOnSpline: description = tr("is on Spline"); break;
  case FilterTypes::IsDublicate: description = tr("is dublicate"); break;
  case FilterTypes::CustomIndex: description = tr("Manual Selection"); break;
  }

  return description;
}

QString NodeFilter::getFilterMethodDescription(const FilterMethods method)
{
  QString description = "";
  switch (method) {
  case FilterMethods::Contains: description = "%"; break;
  case FilterMethods::Is: description = "="; break;
  case FilterMethods::SmallerThan: description = "<"; break;
  case FilterMethods::BiggerThan: description = ">"; break;
  }
  return description;
}

void NodeFilter::updateFilterLabel()
{
  // Build the description string for the label
  QString method = "";
  if (filterType != FilterTypes::IsOnSpline &&
      filterType != FilterTypes::IsDublicate &&
      filterType != FilterTypes::CustomIndex)
    method = getFilterMethodDescription(filterMethod);
  const QString filterDesc = QString("%1 %2 %3")
      .arg(getDescriptionFromFilterType(filterType))
      .arg(method)
      .arg(filterDisplayValue)
      .trimmed();
  filterLabel->setText(filterDesc);
}
