#ifndef PREFAB_H
#define PREFAB_H

#include <cmath>
#include <QModelIndex>
#include <QObject>
#include <QVector>

#include "editormodel.h"
#include "editormodelitem.h"
#include "nodeeditor.h"
#include "velodb.h"

class EditorModelItem;

enum class EditorModelColumns;
class NodeEditor;

class EditorObject : public QObject
{
  Q_OBJECT

public:
  explicit EditorObject(NodeEditor* parentEditor = nullptr);
  EditorObject(const EditorObject& b);
  ~EditorObject();

  EditorObject& operator =(const EditorObject& b);
  bool operator ==(const EditorObject& b);

  bool applyScaling(const QVector3D& values);

  void reset();

  PrefabData getData() const;
  bool setData(const PrefabData& value);

  unsigned int getId() const;
  QString getName() const;

  QModelIndex getIndex() const;
  void setIndex(const QModelIndex index);

  int getPosition(const int row) const;  
  void setPosition(const int row, const int value);
  void setPosition(const int r, const int g, const int b);
  void setPosition(const QVector3D& position);

  int getRotationVector(const int row) const;
  void setRotation(const int row,  const int value);
  void setRotation(const int w, const int x, const int y, const int z);
  void setRotation(const QQuaternion& rotation);
  void setRotation(const QVector4D& rotation);

  int getScaling(const int row) const;
  bool setScaling(const int row, const int value);
  bool setScaling(const int r, const int g, const int b);

  int getPositionR() const;
  void setPositionR(const int value);
  void setPositionR(const double value);

  int getPositionG() const;
  void setPositionG(const int value);
  void setPositionG(const double value);

  int getPositionB() const;
  void setPositionB(const int value);
  void setPositionB(const double value);

  QVector3D getPositionVector() const;

  int getRotationW() const;
  void setRotationW(const int value);
  void setRotationW(const double value);

  int getRotationX() const;
  void setRotationX(const int value);
  void setRotationX(const double value);

  int getRotationY() const;
  void setRotationY(const int value);
  void setRotationY(const double value);

  int getRotationZ() const;
  void setRotationZ(const int value);
  void setRotationZ(const double value);

  QQuaternion getRotationQuaterion() const;
  QVector4D getRotationVector() const;

  int getScalingR() const;
  void setScalingR(const int value);
  void setScalingR(const double value);

  int getScalingG() const;
  void setScalingG(const int value);
  void setScalingG(const double value);

  int getScalingB() const;
  void setScalingB(const int value);
  void setScalingB(const double value);

  QVector3D getScalingVector() const;
  void setScaling(const QVector3D& scaling);

  bool isGate() const;
  int getGateNo() const;
  void setGateNo(const int value, const bool updateGateOrder = true);

  bool getFinish() const;
  void setFinish(const bool value);

  bool getStart() const;
  void setStart(const bool value);

  bool isEditable() const;

  bool isSpline() const;
  bool isOnSpline() const;
  bool isSplineControl() const;

  QVector<EditorObject*>& getSplineControls();
  QVector<EditorObject*>& getSplineObjects();
  QVector<EditorObject*>& getSplineParents();

  bool getIsMoving() const;
  void setIsMoving(bool value);

  char getSpeed() const;
  void setSpeed(char value);

  int getSplineIndex() const;
  void setSplineIndex(const int value);

  bool isValid() const;  

  EditorModelItem* getParentModelItem() const;
  void setParentModelItem(EditorModelItem* item);

  EditorObject* getParentObject() const;
  void setParentObject(EditorObject* parent);

  bool isModified() const;
  void setModified(bool value = true);

  bool isFilterMarked() const;
  void setFilterMarked(const bool value = true);

private:
  bool modified = false;
  bool filterMarked = false;

  const int modifiedRole = 20000;

  PrefabData prefab = PrefabData();

  QVector<EditorObject*> splineControls;
  QVector<EditorObject*> splineObjects;
  QVector<EditorObject*> splineParents;

  EditorObject* parentObject;
  EditorModelItem* parentModelItem;

  NodeEditor* editor = nullptr;
  QModelIndex index;

  int positionR = 0;
  int positionG = 0;
  int positionB = 0;

  int rotationW = 1000;
  int rotationX = 0;
  int rotationY = 0;
  int rotationZ = 0;

  int scalingR = 0;
  int scalingG = 0;
  int scalingB = 0;

  int gateNo = -1;
  bool finish = false;
  bool start = false;

  bool isMoving = false;
  char speed = 0;
  int splineIndex = 0;
};

#endif // PREFAB_H
