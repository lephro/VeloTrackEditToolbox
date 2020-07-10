#include "mainwindow.h"
#include "ui_mainwindow.h"

void MainWindow::on_transformBDoubleResetPushButton_released()
{
  ui->transformBDoubleSpinBox->setValue(ui->transformByComboBox->currentIndex() == 0 ? 100 : 1);
}

void MainWindow::on_transformByComboBox_currentTextChanged(const QString& transformBy)
{
  if (transformBy == "%") {

    // If a spin box already contains the percent suffix, that means that we are already in percent mode
    if (ui->transformRDoubleSpinBox->suffix() == "%")
      return;

    ui->transformRDoubleSpinBox->setDecimals(2);
    ui->transformGDoubleSpinBox->setDecimals(2);
    ui->transformBDoubleSpinBox->setDecimals(2);
    ui->transformRDoubleSpinBox->setMaximum(999.99);
    ui->transformGDoubleSpinBox->setMaximum(999.99);
    ui->transformBDoubleSpinBox->setMaximum(999.99);
    ui->transformRDoubleSpinBox->setMinimum(0.01);
    ui->transformGDoubleSpinBox->setMinimum(0.01);
    ui->transformBDoubleSpinBox->setMinimum(0.01);
    ui->transformRDoubleSpinBox->setSingleStep(1);
    ui->transformGDoubleSpinBox->setSingleStep(1);
    ui->transformBDoubleSpinBox->setSingleStep(1);
    ui->transformRDoubleSpinBox->setSuffix("%");
    ui->transformGDoubleSpinBox->setSuffix("%");
    ui->transformBDoubleSpinBox->setSuffix("%");
    ui->transformRDoubleSpinBox->setValue(ui->transformRDoubleSpinBox->value() * 100);
    ui->transformGDoubleSpinBox->setValue(ui->transformGDoubleSpinBox->value() * 100);
    ui->transformBDoubleSpinBox->setValue(ui->transformBDoubleSpinBox->value() * 100);
  } else if (transformBy == "RGB") {

    // If a spin box already has no suffix, that means that we are already in RGB mode
    if (ui->transformRDoubleSpinBox->suffix() == "")
      return;

    ui->transformRDoubleSpinBox->setDecimals(0);
    ui->transformGDoubleSpinBox->setDecimals(0);
    ui->transformBDoubleSpinBox->setDecimals(0);
    ui->transformRDoubleSpinBox->setMaximum(99999);
    ui->transformGDoubleSpinBox->setMaximum(99999);
    ui->transformBDoubleSpinBox->setMaximum(99999);
    ui->transformRDoubleSpinBox->setMinimum(-99999);
    ui->transformGDoubleSpinBox->setMinimum(-99999);
    ui->transformBDoubleSpinBox->setMinimum(-99999);
    ui->transformRDoubleSpinBox->setSingleStep(1);
    ui->transformGDoubleSpinBox->setSingleStep(1);
    ui->transformBDoubleSpinBox->setSingleStep(1);
    ui->transformRDoubleSpinBox->setSuffix("");
    ui->transformGDoubleSpinBox->setSuffix("");
    ui->transformBDoubleSpinBox->setSuffix("");
    ui->transformRDoubleSpinBox->setValue(ui->transformRDoubleSpinBox->value() / 100);
    ui->transformGDoubleSpinBox->setValue(ui->transformGDoubleSpinBox->value() / 100);
    ui->transformBDoubleSpinBox->setValue(ui->transformBDoubleSpinBox->value() / 100);
  }
}

void MainWindow::on_transformGDoubleResetPushButton_released()
{
  ui->transformGDoubleSpinBox->setValue(ui->transformByComboBox->currentIndex() == 0 ? 100 : 1);
}

void MainWindow::on_transformRDoubleResetPushButton_released()
{
  ui->transformRDoubleSpinBox->setValue(ui->transformByComboBox->currentIndex() == 0 ? 100 : 1);
}

void MainWindow::on_transformRotationBValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromAngleValues();
}

void MainWindow::on_transformRotationGValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromAngleValues();
}

void MainWindow::on_transformRotationRValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromAngleValues();
}

void MainWindow::on_transformRotationWValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromQuaternionValues();
}

void MainWindow::on_transformRotationXValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromQuaternionValues();
}

void MainWindow::on_transformRotationYValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromQuaternionValues();
}

void MainWindow::on_transformRotationZValueSpinBox_valueChanged(int value)
{
  Q_UNUSED(value)
  updateTransformFromQuaternionValues();
}

void MainWindow::updateTransformFromAngleValues()
{
  const int r = ui->transformRotationRValueSpinBox->value();
  const int g = ui->transformRotationGValueSpinBox->value();
  const int b = ui->transformRotationBValueSpinBox->value();
  QQuaternion quaternion = QQuaternion::fromEulerAngles(r, g, b);
  qDebug() << "From Euler:" << quaternion << r << g << b;
  ui->transformRotationWValueSpinBox->blockSignals(true);
  ui->transformRotationWValueSpinBox->setValue(int(std::round(quaternion.scalar() * 1000)));
  ui->transformRotationWValueSpinBox->blockSignals(false);

  ui->transformRotationXValueSpinBox->blockSignals(true);
  ui->transformRotationXValueSpinBox->setValue(int(std::round(quaternion.x() * 1000)));
  ui->transformRotationXValueSpinBox->blockSignals(false);

  ui->transformRotationYValueSpinBox->blockSignals(true);
  ui->transformRotationYValueSpinBox->setValue(int(std::round(quaternion.y() * 1000)));
  ui->transformRotationYValueSpinBox->blockSignals(false);

  ui->transformRotationZValueSpinBox->blockSignals(true);
  ui->transformRotationZValueSpinBox->setValue(int(std::round(quaternion.z() * 1000)));
  ui->transformRotationZValueSpinBox->blockSignals(false);
}

void MainWindow::updateTransformFromQuaternionValues()
{
  const float l = float(ui->transformRotationWValueSpinBox->value()) / 1000;
  const float i = float(ui->transformRotationXValueSpinBox->value()) / 1000;
  const float j = float(ui->transformRotationYValueSpinBox->value()) / 1000;
  const float k = float(ui->transformRotationZValueSpinBox->value()) / 1000;
  QQuaternion quaternion = QQuaternion(l, i , j, k);
  const QVector3D eulerAngles = quaternion.toEulerAngles();
  qDebug() << "From Quat:" << quaternion << eulerAngles;
  ui->transformRotationRValueSpinBox->blockSignals(true);
  ui->transformRotationRValueSpinBox->setValue(int(eulerAngles.x()));
  ui->transformRotationRValueSpinBox->blockSignals(false);

  ui->transformRotationGValueSpinBox->blockSignals(true);
  ui->transformRotationGValueSpinBox->setValue(int(eulerAngles.y()));
  ui->transformRotationGValueSpinBox->blockSignals(false);

  ui->transformRotationBValueSpinBox->blockSignals(true);
  ui->transformRotationBValueSpinBox->setValue(int(eulerAngles.z()));
  ui->transformRotationBValueSpinBox->blockSignals(false);
}
