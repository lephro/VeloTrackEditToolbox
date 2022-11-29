#include "editorobject.h"

EditorObject::EditorObject(NodeEditor* parentEditor)
{
  editor = parentEditor;
}

EditorObject::EditorObject(const EditorObject& b)
  : QObject(b.parent())
{
  if (&b == this)
    return;

  prefab = b.prefab;

  editor = b.editor;

  positionR = b.positionR;
  positionG = b.positionG;
  positionB = b.positionB;

  rotationW = b.rotationW;
  rotationX = b.rotationX;
  rotationY = b.rotationY;
  rotationZ = b.rotationZ;

  scalingR = b.scalingR;
  scalingG = b.scalingG;
  scalingB = b.scalingB;

  gateNo = b.gateNo;
  finish = b.finish;
  start = b.start;

  isMoving = b.isMoving;
  speed = b.speed;

  filterMarked = b.filterMarked;

  foreach(EditorObject* obj, b.splineControls) {
    splineControls.append(new EditorObject(*obj));
  }
  foreach(EditorObject* obj, b.splineObjects) {
    splineObjects.append(new EditorObject(*obj));
  }
  foreach(EditorObject* obj, b.splineParents) {
    splineParents.append(new EditorObject(*obj));
  }
}

EditorObject::~EditorObject()
{
  qDebug() << "Editor Object destroyed";
}

EditorObject& EditorObject::operator =(const EditorObject& b)
{
  if (&b == this)
    return *this;

  prefab = b.prefab;

  editor = b.editor;

  positionR = b.positionR;
  positionG = b.positionG;
  positionB = b.positionB;

  rotationW = b.rotationW;
  rotationX = b.rotationX;
  rotationY = b.rotationY;
  rotationZ = b.rotationZ;

  scalingR = b.scalingR;
  scalingG = b.scalingG;
  scalingB = b.scalingB;

  gateNo = b.gateNo;
  finish = b.finish;
  start = b.start;

  isMoving = b.isMoving;
  speed = b.speed;

  modified = b.modified;
  filterMarked = b.filterMarked;

  foreach(EditorObject* obj, b.splineControls) {
    splineControls.append(new EditorObject(*obj));
  }
  foreach(EditorObject* obj, b.splineObjects) {
    splineObjects.append(new EditorObject(*obj));
  }
  foreach(EditorObject* obj, b.splineParents) {
    splineParents.append(new EditorObject(*obj));
  }

  return *this;
}

bool EditorObject::operator==(const EditorObject &b) {
  return (prefab.id == b.prefab.id) &&
         (gateNo == b.gateNo) &&
         (start == b.start) &&
         (finish == b.finish) &&
         (positionR == b.positionR) &&
         (positionG == b.positionG) &&
         (positionB == b.positionB) &&
         (rotationW == b.rotationW) &&
         (rotationX == b.rotationX) &&
         (rotationY == b.rotationY) &&
         (rotationZ == b.rotationZ) &&
         (scalingR == b.scalingR) &&
         (scalingG == b.scalingG) &&
         (scalingB == b.scalingB) &&
         (isMoving == b.isMoving) &&
         (speed == b.speed) &&
         (splineControls.count() == b.splineControls.count()) &&
         (splineObjects.count() == b.splineObjects.count()) &&
         (splineParents.count() == b.splineParents.count());
}

bool EditorObject::applyScaling(const QVector3D& values)
{
  if (values == QVector3D(1, 1, 1))
    return false;

  setScaling(0, int(std::round(scalingR * values.x())));
  setScaling(1, int(std::round(scalingG * values.y())));
  setScaling(2, int(std::round(scalingB * values.z())));

  return true;
}

void EditorObject::reset()
{
  prefab = PrefabData();

  index = QModelIndex();

  positionR = 0;
  positionG = 0;
  positionB = 0;

  rotationW = 1000;
  rotationX = 0;
  rotationY = 0;
  rotationZ = 0;

  scalingR = 0;
  scalingG = 0;
  scalingB = 0;

  gateNo = -1;
  finish = false;
  start = false;
}


PrefabData EditorObject::getData() const
{
  return prefab;
}

unsigned int EditorObject::getId() const
{
  return prefab.id;
}

QString EditorObject::getName() const
{
  return prefab.name;
}

QModelIndex EditorObject::getIndex() const
{
  return index;
}

void EditorObject::setIndex(const QModelIndex index)
{
  this->index = index;
}

bool EditorObject::setData(const PrefabData& value)
{
  if (!isEditable())
    return false;

  prefab = value;

  // Update the model if set
  setModified();

  return true;
}

int EditorObject::getPosition(const int row) const
{
  switch (row) {
  case 0: return positionR;
  case 1: return positionG;
  case 2: return positionB;
  }
  return 0;
}

void EditorObject::setPosition(const int row, const int value)
{
  switch (row) {
  case 0: positionR = value; break;
  case 1: positionG = value; break;
  case 2: positionB = value; break;
  }

  // Update the model if set
  setModified();
}

void EditorObject::setPosition(const int r, const int g, const int b)
{
  positionR = r;
  positionG = g;
  positionB = b;

  // Update the model if setsetModified();
}

int EditorObject::getRotationVector(const int row) const
{
  switch (row) {
  case 0: return rotationW;
  case 1: return rotationX;
  case 2: return rotationY;
  case 3: return rotationZ;
  }
  return 0;
}

void EditorObject::setRotation(const int row, const int value)
{
  switch (row) {
  case 0: rotationW = value; break;
  case 1: rotationX = value; break;
  case 2: rotationY = value; break;
  case 3: rotationZ = value; break;
  }

  // Update the model if set
  setModified();
}

void EditorObject::setRotation(const int w, const int x, const int y, const int z)
{
  rotationW = w;
  rotationX = x;
  rotationY = y;
  rotationZ = z;

  // Update the model if set
  setModified();
}

void EditorObject::setRotation(const QQuaternion& rotation)
{
  setRotation(int(std::round(rotation.scalar() * 1000)),
              int(std::round(rotation.x() * 1000)),
              int(std::round(rotation.y() * 1000)),
              int(std::round(rotation.z() * 1000)));
}

int EditorObject::getScaling(const int row) const
{
  switch (row) {
  case 0: return scalingR;
  case 1: return scalingG;
  case 2: return scalingB;
  }
  return 1;
}

bool EditorObject::setScaling(const int row, const int value)
{
  switch (row) {
  case 0: scalingR = value; break;
  case 1: scalingG = value; break;
  case 2: scalingB = value; break;
  }

  // Update the model if set
  setModified();

  return true;
}

bool EditorObject::setScaling(const int r, const int g, const int b)
{
  scalingR = r;
  scalingG = g;
  scalingB = b;

  // Update the model if set
  setModified();

  return true;
}

int EditorObject::getPositionR() const
{
  return positionR;
}

void EditorObject::setPositionR(const int value)
{
  setPosition(0, value);
}

void EditorObject::setPositionR(const double value)
{
  setPosition(0, int(std::round(value)));
}

int EditorObject::getPositionG() const
{
  return positionG;
}

void EditorObject::setPositionG(int value)
{
  setPosition(1, value);
}

void EditorObject::setPositionG(const double value)
{
  setPosition(1, int(std::round(value)));
}

int EditorObject::getPositionB() const
{
  return positionB;
}

void EditorObject::setPositionB(const int value)
{
  setPosition(2, value);
}

void EditorObject::setPositionB(const double value)
{
  setPosition(2, int(std::round(value)));
}

QVector3D EditorObject::getPositionVector() const
{
  return QVector3D(positionR, positionG, positionB);
}

void EditorObject::setPosition(const QVector3D& position)
{
  setPosition(0, int(std::round(position.x())));
  setPosition(1, int(std::round(position.y())));
  setPosition(2, int(std::round(position.z())));
}

int EditorObject::getRotationW() const
{
  return rotationW;
}

void EditorObject::setRotationW(const int value)
{
  setRotation(0, value);
}

void EditorObject::setRotationW(const double value)
{
  setRotation(0, int(std::round(value)));
}

int EditorObject::getRotationX() const
{
  return rotationX;
}

void EditorObject::setRotationX(const int value)
{
  setRotation(1, value);
}

void EditorObject::setRotationX(const double value)
{
  setRotation(1, int(std::round(value)));
}

int EditorObject::getRotationY() const
{
  return rotationY;
}

void EditorObject::setRotationY(const int value)
{
  setRotation(2, value);
}

void EditorObject::setRotationY(const double value)
{
  setRotation(2, int(std::round(value)));
}

int EditorObject::getRotationZ() const
{
  return rotationZ;
}

void EditorObject::setRotationZ(const int value)
{
  setRotation(3, value);
}

void EditorObject::setRotationZ(const double value)
{
  setRotation(3, int(std::round(value)));
}

QQuaternion EditorObject::getRotationQuaterion() const
{
  return QQuaternion(float(rotationW) / 1000,
                     float(rotationX) / 1000,
                     float(rotationY) / 1000,
                     float(rotationZ) / 1000);
}

QVector4D EditorObject::getRotationVector() const
{
  return QVector4D(rotationW, rotationX, rotationY, rotationZ);
}

void EditorObject::setRotation(const QVector4D& rotation)
{
  setRotation(0, int(std::round(rotation.w())));
  setRotation(1, int(std::round(rotation.w())));
  setRotation(2, int(std::round(rotation.w())));
  setRotation(3, int(std::round(rotation.w())));
}

int EditorObject::getScalingR() const
{
  return scalingR;
}

void EditorObject::setScalingR(const int value)
{
  setScaling(0, value);
}

void EditorObject::setScalingR(const double value)
{
  setScaling(0, int(std::round(value * 100)));
}

int EditorObject::getScalingG() const
{
  return scalingG;
}

void EditorObject::setScalingG(const int value)
{
  setScaling(1, value);
}

void EditorObject::setScalingG(const double value)
{
  setScaling(1, int(std::round(value * 100)));
}

int EditorObject::getScalingB() const
{
  return scalingB;
}

void EditorObject::setScalingB(const int value)
{
  setScaling(2, value);
}

void EditorObject::setScalingB(const double value)
{
  setScaling(2, int(std::round(value * 100)));
}

QVector3D EditorObject::getScalingVector() const
{
  return QVector3D(scalingR, scalingB, scalingG);
}

void EditorObject::setScaling(const QVector3D& scaling)
{
  setScaling(0, int(scaling.x()));
  setScaling(1, int(scaling.y()));
  setScaling(2, int(scaling.z()));
}

bool EditorObject::isGate() const
{
  return prefab.gate && gateNo > 0;
}

int EditorObject::getGateNo() const
{
  return gateNo;
}

void EditorObject::setGateNo(const int value, const bool updateGateOrder)
{
  // Update gate order
  const uint oldGateNo = uint(gateNo);

  gateNo = value;

  // Update the model if set
  setModified();

  if (updateGateOrder) {
    editor->changeGateOrder(uint(oldGateNo), uint(value));
  }
}

bool EditorObject::getFinish() const
{
  return finish;
}

void EditorObject::setFinish(const bool value)
{
  // Prevent disabling a finish
  if (finish && !value)
    return;

  finish = value;

  if (editor == nullptr || !value)
    return;

  editor->resetFinishGates();
}

bool EditorObject::getStart() const
{
  return start;
}

void EditorObject::setStart(const bool value)
{
  start = value;

  if (editor == nullptr || !value)
    return;

  editor->resetStartGates();
}

QVector<EditorObject*>& EditorObject::getSplineControls()
{
  return splineControls;
}

QVector<EditorObject*>& EditorObject::getSplineObjects()
{
  return splineObjects;
}

bool EditorObject::getIsMoving() const
{
  return isMoving;
}

void EditorObject::setIsMoving(bool value)
{
  isMoving = value;
}

char EditorObject::getSpeed() const
{
  return speed;
}

void EditorObject::setSpeed(char value)
{
  speed = value;
}

int EditorObject::getSplineIndex() const
{
  return splineIndex;
}

void EditorObject::setSplineIndex(const int value)
{
  splineIndex = value;
}

bool EditorObject::isValid() const
{
  return (prefab.id > 0);
}

EditorModelItem* EditorObject::getParentModelItem() const
{
  return parentModelItem;
}

void EditorObject::setParentModelItem(EditorModelItem* item)
{
  parentModelItem = item;
}

EditorObject* EditorObject::getParentObject() const
{
  return parentObject;
}

void EditorObject::setParentObject(EditorObject* parent)
{
  parentObject = parent;
}

bool EditorObject::isModified() const
{
  return modified;
}

void EditorObject::setModified(bool value)
{
  modified = value;

  if (modified == true)
    return;

  foreach(EditorObject* object, splineObjects) {
    object->setModified(false);
  }

  foreach(EditorObject* object, splineParents) {
    object->setModified(false);
  }

  foreach(EditorObject* object, splineControls) {
    object->setModified(false);
  }
}

bool EditorObject::isFilterMarked() const
{
  return filterMarked;
}

void EditorObject::setFilterMarked(const bool value)
{
  filterMarked = value;
}

QVector<EditorObject*>& EditorObject::getSplineParents()
{
  return splineParents;
}

bool EditorObject::isEditable() const
{
  if (prefab.name == "CtrlParent" ||
      prefab.name == "ControlCurve" ||
      prefab.name == "ControlPoint" ||
      prefab.name == "DefaultStartGrid" ||
      prefab.name == "DefaultKDRAStartGrid" ||
      prefab.name == "DR1StartGrid" ||
      prefab.name == "PolyStartGrid" ||
      prefab.name == "MicroStartGrid")
    return false;

  return true;
}

bool EditorObject::isOnSpline() const
{
  return (this->index.data(Qt::DisplayRole) == "jo");
}

bool EditorObject::isSplineControl() const
{
  return (prefab.id == 345);
}

bool EditorObject::isSpline() const
{
  return (prefab.name == "ControlCurve");
}
