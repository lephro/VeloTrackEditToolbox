#ifndef PREFAB_H
#define PREFAB_H

#include <QModelIndex>
#include <QObject>
#include <QStandardItem>
#include <QVector>

#include "velodb.h"
#include "velotrack.h"

class PrefabItem
{
public:
  PrefabItem();

  void parseIndex(QStandardItemModel *model, const QModelIndex &index);

  unsigned int getId() const;
  void setId(unsigned int value);

  PrefabData getData() const;
  void setData(const PrefabData &value);

  int getPosition(const int row) const;
  void setPosition(const int row, const  int value);

  int getRotation(const int row) const;
  void setRotation(const int row,  const int value);

  int getScaling(const int row) const;
  void setScaling(const int row, const int value);

  int getPositionR() const;
  void setPositionR(const int value);

  int getPositionG() const;
  void setPositionG(const int value);

  int getPositionB() const;
  void setPositionB(const int value);

  int getRotationL() const;
  void setRotationL(const int value);

  int getRotationI() const;
  void setRotationI(const int value);

  int getRotationJ() const;
  void setRotationJ(const int value);

  int getRotationK() const;
  void setRotationK(const int value);

  int getScalingR() const;
  void setScalingR(const int value);

  int getScalingG() const;
  void setScalingG(const int value);

  int getScalingB() const;
  void setScalingB(const int value);

  unsigned int getGateNo() const;
  void setGateNo(const unsigned int value);

  bool getFinish() const;
  void setFinish(const bool value);

  bool getStart() const;
  void setStart(const bool value);

  bool operator ==(PrefabItem &b);

private:
  PrefabData data = PrefabData();

  QStandardItemModel* model = nullptr;
  QModelIndex index;
  QModelIndex positionIndex = QModelIndex();
  QModelIndex rotationIndex = QModelIndex();
  QModelIndex scaleIndex = QModelIndex();
  QModelIndex curveIndex = QModelIndex();

  unsigned int id = 0;

  int positionR = 0;
  int positionG = 0;
  int positionB = 0;

  int rotationL = 0;
  int rotationI = 0;
  int rotationJ = 0;
  int rotationK = 0;

  int scalingR = 0;
  int scalingG = 0;
  int scalingB = 0;

  unsigned int gateNo = 0;
  bool finish = false;
  bool start = false;

  void setValue(QString propertyName, QVariant value);
};

#endif // PREFAB_H
