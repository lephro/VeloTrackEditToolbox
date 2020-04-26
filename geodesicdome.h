#ifndef GEODESICDOME_H
#define GEODESICDOME_H

#include <QList>
#include <QObject>
#include <QMatrix>
#include <QQuaternion>
#include <QStandardItemModel>
#include <QVector3D>

#define X .525731112119133606
#define Z .850650808352039932

const QList<QVector3D> NormalizedIcosahedronVertices = {
  {-float(X),      0.0, float(Z)}, { float(X),      0.0,  float(Z)}, {-float(X),       0.0, -float(Z)}, { float(X),       0.0, -float(Z)},
  {      0.0, float(Z), float(X)}, {      0.0, float(Z), -float(X)}, {      0.0, -float(Z),  float(X)}, {      0.0, -float(Z), -float(X)},
  { float(Z), float(X),      0.0}, {-float(Z), float(X),       0.0}, { float(Z), -float(X),       0.0}, {-float(Z), -float(X),       0.0}};

const QList<QVector3D> IcosahedronIndices = {
  { 0,  4,  1}, { 0, 9,  4}, { 9,  5, 4}, { 4, 5, 8}, { 4, 8,  1},
  { 8, 10,  1}, { 8, 3, 10}, { 5,  3, 8}, { 5, 2, 3}, { 2, 7,  3},
  { 7, 10,  3}, { 7, 6, 10}, { 7, 11, 6}, {11, 0, 6}, { 0, 1,  6},
  { 6,  1, 10}, { 9, 0, 11}, { 9, 11, 2}, { 9, 2, 5}, { 7, 2, 11}};

class GeodesicDome : public QObject
{
  Q_OBJECT
public:
  explicit GeodesicDome(QObject *parent = nullptr, const unsigned int frequency = 1);

  QByteArray getVeloTrackData();  
  QByteArray getVeloTrackDataTest();

private:
  QList<QVector3D> indices;  
  QList<QVector3D> vertices;

  QVector3D& getMidpoint(const QVector3D &v1, const QVector3D &v2) const;
  QQuaternion& getPrefabRotation(const QVector3D &direction) const;

  void subdivideVertex(const QVector3D& v1,
                       const QVector3D& v2,
                       const QVector3D& v3,
                       const unsigned int depth);
};

#endif // GEODESICDOME_H
