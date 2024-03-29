#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_searchAddFilterPushButton_released()
{
  // Get the description of the filter that has been chosen
  QString selectionText = ui->searchTypeComboBox->currentText();
  if (selectionText == tr("Position") ||
      selectionText == tr("Rotation") ||
      selectionText == tr("Scaling"))
  {
    selectionText = QString("%1 %2").arg(selectionText).arg(ui->searchSubtypeComboBox->currentText());
  }

  const FilterTypes filterType = NodeFilter::getFilterTypeByDescription(selectionText);

  // If we filter on "rotate from angle" we need to create 4 filter with a different data origin
  if (filterType == FilterTypes::AnyRotation && ui->searchSubtypeComboBox->currentText().toLower() == tr("from angle")) {
    // Create the filter widgets and add them to the layout
    searchFilterLayout->addFilter(FilterTypes::RotationW,
                                  FilterMethods::Is,
                                  ui->searchTypeRotationWValueSpinBox->value());
    searchFilterLayout->addFilter(FilterTypes::RotationX,
                                  FilterMethods::Is,
                                  ui->searchTypeRotationXValueSpinBox->value());
    searchFilterLayout->addFilter(FilterTypes::RotationY,
                                  FilterMethods::Is,
                                  ui->searchTypeRotationYValueSpinBox->value());
    searchFilterLayout->addFilter(FilterTypes::RotationZ,
                                  FilterMethods::Is,
                                  ui->searchTypeRotationZValueSpinBox->value());
  } else {
    QString displayValue = "";
    int value = 0;

    // Get the display-value and value for the filter from the controls
    switch (filterType) {
    case FilterTypes::Object:
      displayValue = ui->searchValueComboBox->currentText();
      value = int(ui->searchValueComboBox->currentData().toUInt());
      if (value == 0)
        return;
      break;
    default:
      value = ui->searchValueSpinBox->value();
      break;
    }

    currentCacheId++;

    // Create the filter widget and add it to the layout
    searchFilterLayout->addFilter(filterType, FilterMethods(ui->searchMethodComboBox->currentIndex()), value, displayValue);

    // Set our toolbox target to filtered objects
    ui->toolsTargetComboBox->setCurrentIndex(2);
  }

  // Update markings and status bar
  updateSearch();
  updateStatusBar();
}

void MainWindow::on_searchClearFilterPushButton_released()
{
  currentCacheId++;

  // Clear the filter and update the markings
  searchFilterLayout->clear();
  updateSearch(false);
  updateStatusBar();

  // Set the toolbox target to selected objects (if its on filtered)
  if (ui->toolsTargetComboBox->currentIndex() == 2)
    ui->toolsTargetComboBox->setCurrentIndex(0);
}

void MainWindow::on_searchFilterGroupBox_toggled(bool newState)
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // Update the markings with the given state
  updateSearch(newState);
  updateStatusBar();

  // Enable / Disable the filtered model
  nodeEditor->getFilteredModel().setSearchFilter(newState ? ui->searchOptionsShowOnlyFilteredCheckBox->isChecked() : false);

  // Set our toolbox target to filtered objects if filter is enabled, otherwise to selected
  ui->toolsTargetComboBox->setCurrentIndex(newState ? 2 : 1);
}

void MainWindow::on_searchOptionsShowOnlyFilteredCheckBox_stateChanged(int checked)
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  nodeEditor->getFilteredModel().setSearchFilter(checked > 0);
}

void MainWindow::on_searchSubtypeComboBox_currentIndexChanged(const QString &subtypeDesc)
{
  ui->searchFilterValueStackedWidget->setCurrentIndex(subtypeDesc == tr("From angle") ? 1 : 0);
}

void MainWindow::on_searchTypeComboBox_currentIndexChanged(const QString &filterDesc)
{  
  // Switch the visibilty of the filter-value controls according to the filter-type selected
  const FilterTypes type = NodeFilter::getFilterTypeByDescription(filterDesc);
  qDebug() << filterDesc << type;
  int minValue = -999999;
  int maxValue = 999999;
  switch (type) {
  case FilterTypes::Object:
    ui->searchMethodComboBox->hide();
    ui->searchSubtypeComboBox->hide();
    ui->searchValueComboBox->show();
    ui->searchValueLabel->hide();
    ui->searchValueSpinBox->hide();
    ui->searchValueLine->show();
    break;

  case FilterTypes::AnyRotation:
  case FilterTypes::AnyPosition:
  case FilterTypes::AnyScaling:
  case FilterTypes::GateNo:
    ui->searchMethodComboBox->show();
    ui->searchSubtypeComboBox->show();
    ui->searchSubtypeComboBox->clear();
    if (type == AnyRotation) {
      ui->searchSubtypeComboBox->addItem("Any");
      ui->searchSubtypeComboBox->addItem("W");
      ui->searchSubtypeComboBox->addItem("X");
      ui->searchSubtypeComboBox->addItem("Y");
      ui->searchSubtypeComboBox->addItem("Z");
      ui->searchSubtypeComboBox->addItem("From angle");
    } else if (type ==GateNo){
      ui->searchSubtypeComboBox->hide();
    } else {
      ui->searchSubtypeComboBox->addItem("Any");
      ui->searchSubtypeComboBox->addItem("R");
      ui->searchSubtypeComboBox->addItem("G");
      ui->searchSubtypeComboBox->addItem("B");
    }
    ui->searchValueLabel->show();
    ui->searchValueSpinBox->show();
    if (type == FilterTypes::GateNo) {
      minValue = 0;
      maxValue = 0;
      const NodeEditor* nodeEditor = nodeEditorManager->getEditor();
      if (nodeEditor != nullptr)
        maxValue = int(nodeEditor->getTrack()->getGateCount()) - 1;
    } else if (type == FilterTypes::AnyRotation) {
      minValue = -1000;
      maxValue = 1000;
    }
    ui->searchValueSpinBox->setMinimum(minValue);
    ui->searchValueSpinBox->setMaximum(maxValue);
    ui->searchValueComboBox->hide();
    ui->searchValueLine->show();
    break;

  case FilterTypes::IsOnSpline:
  case FilterTypes::IsDublicate:
    ui->searchMethodComboBox->hide();
    ui->searchSubtypeComboBox->hide();
    ui->searchValueComboBox->hide();
    ui->searchValueLabel->hide();
    ui->searchValueSpinBox->hide();
    ui->searchValueLine->hide();
    break;

  default:
    return;
  }
}

void MainWindow::on_searchTypeRotationWValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromQuaternionValues();
}

void MainWindow::on_searchTypeRotationXValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromQuaternionValues();
}

void MainWindow::on_searchTypeRotationYValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromQuaternionValues();
}

void MainWindow::on_searchTypeRotationZValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromQuaternionValues();
}

void MainWindow::on_searchTypeRotationRValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromAngleValues();
}

void MainWindow::on_searchTypeRotationGValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromAngleValues();
}

void MainWindow::on_searchTypeRotationBValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateSearchFromAngleValues();
}

void MainWindow::onSearchFilterChanged()
{
  currentCacheId++;

  // Update our search filter if a filter gets added or removed
  updateSearch();
  updateStatusBar();
}

void MainWindow::updateSearch()
{
  updateSearch(ui->searchFilterGroupBox->isChecked());
}

void MainWindow::updateSearchFilterControls()
{
  on_searchTypeComboBox_currentIndexChanged(ui->searchTypeComboBox->currentText());
}

void MainWindow::updateSearch(bool filterEnabled)
{
  NodeEditor* nodeEditor = nodeEditorManager->getEditor();
  if (nodeEditor == nullptr)
    return;

  // If we don't have any filter set, there is no need for the controls to be visible
  if (searchFilterLayout->getFilterList().count() == 0) {
    ui->searchFilterGroupBox->hide();
    ui->searchClearFilterPushButton->hide();

    // Just reset the markings and leave this place
    nodeEditor->beginNodeEdit();
    nodeEditor->clearSearch(currentCacheId);
    nodeEditor->endNodeEdit();
    return;
  }

  // We got a filter, so we need to make sure, that we see the controls
  ui->searchFilterGroupBox->show();
  ui->searchClearFilterPushButton->show();

  // If marks are disabled we simply just reset all marks
  if (!filterEnabled) {
    nodeEditor->beginNodeEdit();
    nodeEditor->clearFilterMarks();
    nodeEditor->endNodeEdit();
    return;
  }

  // Update the search cache if nescessary
  if (nodeEditor->getSearchCacheId() != currentCacheId) {
    nodeEditor->setSearchResult(currentCacheId, nodeEditor->search(nodeEditor->getTrack()->getObjects(), searchFilterLayout->getFilterList()));
  }

  // Mark all found items
  nodeEditor->beginNodeEdit();
  nodeEditor->setFilterMarks();
  nodeEditor->endNodeEdit();
}

void MainWindow::updateSearchFromAngleValues()
{
  const int r = ui->searchTypeRotationRValueSpinBox->value();
  const int g = ui->searchTypeRotationGValueSpinBox->value();
  const int b = ui->searchTypeRotationBValueSpinBox->value();

  QQuaternion quaternion = QQuaternion::fromEulerAngles(r, g, b);
  //qDebug() << "From Euler:" << quaternion << r << g << b;
  ui->searchTypeRotationWValueSpinBox->blockSignals(true);
  ui->searchTypeRotationWValueSpinBox->setValue(int(std::round(quaternion.scalar() * 1000)));
  ui->searchTypeRotationWValueSpinBox->blockSignals(false);

  ui->searchTypeRotationXValueSpinBox->blockSignals(true);
  ui->searchTypeRotationXValueSpinBox->setValue(int(std::round(quaternion.x() * 1000)));
  ui->searchTypeRotationXValueSpinBox->blockSignals(false);

  ui->searchTypeRotationYValueSpinBox->blockSignals(true);
  ui->searchTypeRotationYValueSpinBox->setValue(int(std::round(quaternion.y() * 1000)));
  ui->searchTypeRotationYValueSpinBox->blockSignals(false);

  ui->searchTypeRotationZValueSpinBox->blockSignals(true);
  ui->searchTypeRotationZValueSpinBox->setValue(int(std::round(quaternion.z() * 1000)));
  ui->searchTypeRotationZValueSpinBox->blockSignals(false);
}

void MainWindow::updateSearchFromQuaternionValues()
{
  QQuaternion quaternion = QQuaternion(float(ui->searchTypeRotationWValueSpinBox->value()) / 1000,
                                       float(ui->searchTypeRotationXValueSpinBox->value()) / 1000,
                                       float(ui->searchTypeRotationYValueSpinBox->value()) / 1000,
                                       float(ui->searchTypeRotationZValueSpinBox->value()) / 1000);

  const QVector3D eulerAngles = quaternion.toEulerAngles();
  //qDebug() << "From Quat:" << quaternion << eulerAngles;
  ui->searchTypeRotationRValueSpinBox->blockSignals(true);
  ui->searchTypeRotationRValueSpinBox->setValue(int(eulerAngles.x()));
  ui->searchTypeRotationRValueSpinBox->blockSignals(false);

  ui->searchTypeRotationGValueSpinBox->blockSignals(true);
  ui->searchTypeRotationGValueSpinBox->setValue(int(eulerAngles.y()));
  ui->searchTypeRotationGValueSpinBox->blockSignals(false);

  ui->searchTypeRotationBValueSpinBox->blockSignals(true);
  ui->searchTypeRotationBValueSpinBox->setValue(int(eulerAngles.z()));
  ui->searchTypeRotationBValueSpinBox->blockSignals(false);
}
