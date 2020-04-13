#include "geodesicdome.h"

GeodesicDome::GeodesicDome(QObject *parent, const unsigned int frequency) : QObject(parent)
{
  foreach(const QVector3D index, IcosahedronIndices) {
    subdivideVertex(NormalizedIcosahedronVertices[int(index[0])],
                    NormalizedIcosahedronVertices[int(index[1])],
                    NormalizedIcosahedronVertices[int(index[2])],
                    frequency);
  }
}

QByteArray GeodesicDome::getVeloTrackData()
{
  const QVector3D up(0, 1, 0);
  const QString trackTemplate = "{\"gates\":[{\"prefab\":88,\"trans\":{\"pos\":[1500,0,1000],\"rot\":[707,-707,0,0],\"scale\":[100,100,100]},\"gate\":0,\"start\":true,\"finish\":false},{\"prefab\":88,\"trans\":{\"pos\":[1500,0,1500],\"rot\":[707,-707,0,0],\"scale\":[100,100,100]},\"gate\":1,\"start\":false,\"finish\":true}],\"barriers\":[%1{\"prefab\":422,\"trans\":{\"pos\":[1500,0,500],\"rot\":[1000,0,0,0],\"scale\":[100,100,100]}}]}";
  const QString prefabTemplate = "{\"prefab\":%0,\"trans\":{\"pos\":[%1,%2,%3],\"rot\":[%4,%5,%6,%7],\"scale\":[%8,%9,%10]}}";
  QString prefabs;

  foreach(QVector3D index, indices) {

    for(int i = 0; i < 3; ++i) {
      prefabs += prefabTemplate
              .arg(357)
              .arg(int(vertices[int(index[i])].x() * 10000), 0, 10)
              .arg(int(vertices[int(index[i])].y() * 10000), 0, 10)
              .arg(int(vertices[int(index[i])].z() * 10000), 0, 10)
              .arg(1000)
              .arg(0)
              .arg(0)
              .arg(0)
              .arg(100)
              .arg(100)
              .arg(100) + ",";
    }

    for (int i = 0; i < 3; ++i) {
      const QVector3D position = vertices[int(index[i])];//getMidpoint(vertices[int(index[i])], vertices[int(index[(i + 1) % 3])]);
      QVector3D direction = vertices[int(index[(i + 1) % 3])] - vertices[int(index[i])];
      //direction.normalize();
      QQuaternion rotation = getPrefabRotation(direction);
      //QQuaternion rotation = QQuaternion::fromDirection(direction, up);
      const QVector3D defaultRotVec(0, 1, 0);
      const QQuaternion defaultRotation = QQuaternion::fromDirection(defaultRotVec, up);
      QQuaternion result = rotation.rotationTo(up, direction);
      //QQuaternion result = defaultRotation * rotation;
      prefabs += prefabTemplate
              .arg(340)

              .arg(int((position.x() * 10000) - (vertices[int(index[i])].distanceToPoint(vertices[int(index[(i + 1) % 3])]) / 2)), 0, 10)
              .arg(int(position.y() * 10000), 0, 10)
              .arg(int(position.z() * 10000), 0, 10)

              .arg(int(result.x() * 1000))
              .arg(int(result.y() * 1000))
              .arg(int(result.z() * 1000))
              .arg(int(result.scalar() * 1000))

              .arg(15 + int(vertices[int(index[i])].distanceToPoint(vertices[int(index[(i + 1) % 3])]) * 10000))
              .arg(150)
              .arg(150) + ",";
    }
  }
//  foreach(QVector3D vertex, vertices) {

//    prefabs += prefabTemplate
//            .arg(int(vertex.x() * 10000), 0, 10)
//            .arg(int(vertex.y() * 10000), 0, 10)
//            .arg(int(vertex.z() * 10000), 0, 10)
//            .arg(1000)
//            .arg(0)
//            .arg(0)
//            .arg(0)
//            .arg(150)
//            .arg(150)
//            .arg(150) + ",";
//  }
  qDebug() << "Prefabs: " << prefabs;
  return trackTemplate.arg(prefabs).toStdString().c_str();
}

QVector3D& GeodesicDome::getMidpoint(const QVector3D& v1, const QVector3D& v2) const
{
  QVector3D* vector = new QVector3D();
  vector->setX(((v1.x() + v2.x()) / 2));
  vector->setY(((v1.y() + v2.y()) / 2));
  vector->setZ(((v1.z() + v2.z()) / 2));
  return *vector;
}

QQuaternion& GeodesicDome::getPrefabRotation(const QVector3D& direction) const
{
  QQuaternion* rotation = new QQuaternion();
  const QVector3D up(0, 1, 0);
  *rotation = QQuaternion::rotationTo(up, direction);
//  QVector3D xaxis = QVector3D::crossProduct(up, direction);
//  xaxis.normalize();

//  QVector3D yaxis = QVector3D::crossProduct(direction, xaxis);
//  yaxis.normalize();

//  QVector3D column1;
//  column1.setX(xaxis.x());
//  column1.setY(yaxis.x());
//  column1.setZ(direction.x());

//  QVector3D column2;
//  column2.setX(xaxis.y());
//  column2.setY(yaxis.y());
//  column2.setZ(direction.y());

//  QVector3D column3;
//  column3.setX(xaxis.z());
//  column3.setY(yaxis.z());
//  column3.setZ(direction.z());

//  QMatrix3x3 matrix;
  return *rotation;
}

void GeodesicDome::subdivideVertex(const QVector3D &v1, const QVector3D &v2, const QVector3D &v3, const unsigned int depth)
{
  if (depth == 0) {
    vertices.append(v1);
    vertices.append(v2);
    vertices.append(v3);
    int index = vertices.count();
    indices.append(QVector3D(index - 3, index - 2, index - 1));
    return;
  }

  QVector3D v12, v23, v31;

  for (int i = 0; i < 3; ++i) {
    v12[i] = v1[i] + v2[i];
    v23[i] = v2[i] + v3[i];
    v31[i] = v3[i] + v1[i];
  }

  v12.normalize();
  v23.normalize();
  v31.normalize();

  subdivideVertex( v1, v12, v31, depth - 1);
  subdivideVertex( v2, v23, v12, depth - 1);
  subdivideVertex( v3, v31, v23, depth - 1);
  subdivideVertex(v12, v23, v31, depth - 1);
}

