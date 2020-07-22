#ifndef PREFAB_H
#define PREFAB_H

#include <cmath>
#include <QModelIndex>
#include <QObject>
#include <QVector>

#include "nodeeditor.h"
#include "velodb.h"

class NodeEditor;

class PrefabItem
{
public:
  PrefabItem(NodeEditor* parentEditor = nullptr, const QModelIndex& prefabIndex = QModelIndex());

  void applyScaling(const QVector3D values);

  bool parseIndex(const QModelIndex& index, NodeEditor* editor = nullptr);

  PrefabData getData() const;
  void setData(const PrefabData& value);

  unsigned int getId() const;

  QModelIndex getIndex() const;

  int getPosition(const int row) const;  
  void setPosition(const int row, const int value);
  void setPosition(const int r, const int g, const int b);
  void setPosition(const QVector3D position);

  int getRotationVector(const int row) const;
  void setRotation(const int row,  const int value);
  void setRotation(const int w, const int x, const int y, const int z);
  void setRotation(const QQuaternion rotation);
  void setRotation(const QVector4D &rotation);

  int getScaling(const int row) const;
  void setScaling(const int row, const int value);  
  void setScaling(const int r, const int g, const int b);

  int getPositionR() const;
  void setPositionR(const int value);

  int getPositionG() const;
  void setPositionG(const int value);

  int getPositionB() const;
  void setPositionB(const int value);

  QVector3D getPositionVector() const;

  int getRotationW() const;
  void setRotationW(const int value);

  int getRotationX() const;
  void setRotationX(const int value);

  int getRotationY() const;
  void setRotationY(const int value);

  int getRotationZ() const;
  void setRotationZ(const int value);

  QQuaternion getRotationQuaterion() const;
  QVector4D getRotationVector() const;

  int getScalingR() const;
  void setScalingR(const int value);

  int getScalingG() const;
  void setScalingG(const int value);

  int getScalingB() const;
  void setScalingB(const int value);

  QVector3D getScalingVector() const;
  void setScaling(const QVector3D values);

  bool isGate() const;
  int getGateNo() const;
  void setGateNo(const int value, const bool updateGateOrder = true);

  bool getFinish() const;
  void setFinish(const bool value);

  bool getStart() const;
  void setStart(const bool value);

  bool isModified() const;
  void setModified(const bool value = true);

  bool dataEditable() const;

  bool isSpline() const;
  bool isOnSpline() const;
  bool isSplineControl() const;

  void setFilterMark(const bool found = true);
  bool isFilterMarked() const;

  bool operator ==(PrefabItem &b);

private:
  const QBrush defaultFontColor = QBrush(Qt::white);
  const QBrush defaultBackgroundColor = QBrush(QColor(255, 255, 255, 0));
  const int modifiedRole = 20000;

  PrefabData data = PrefabData();

  NodeEditor* editor = nullptr;
  QStandardItemModel* model = nullptr;
  QModelIndex index;
  QModelIndex positionIndex;
  QModelIndex rotationIndex;
  QModelIndex scaleIndex;
  QModelIndex curveIndex; 

  int positionR = 0;
  int positionG = 0;
  int positionB = 0;

  int rotationW = 0;
  int rotationX = 0;
  int rotationY = 0;
  int rotationZ = 0;

  int scalingR = 0;
  int scalingG = 0;
  int scalingB = 0;

  bool gate = false;
  bool onSpline = false;

  int gateNo = -1;
  bool finish = false;
  bool start = false;

  bool hasValidEditor() const;
  bool hasValidEditor(const QModelIndex index) const;

  QModelIndex getPropertyValueIndex(const QString propertyName) const;
  QModelIndex getPropertyValueItem(const QString propertyName) const;

  void setValue(const QString propertyName, const QVariant value);
  void setValueInModel(const int row, const QModelIndex index, QVariant value);
};

#endif // PREFAB_H
