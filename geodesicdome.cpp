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
  const QVector3D defaultDirection(1, 0, 0);
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
      const QVector3D position = vertices[int(index[i])];
      const QVector3D direction = vertices[int(index[i])] - vertices[int(index[(i + 1) % 3])];
      const QQuaternion result = QQuaternion::rotationTo(defaultDirection, direction);
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

QByteArray GeodesicDome::getVeloTrackDataTest() {
  const QVector3D up(0, 1, 0);
  const QVector3D defaultDirection(1, 0, 0);
  const QString trackTemplate = "{\"gates\":[{\"prefab\":88,\"trans\":{\"pos\":[1500,0,1000],\"rot\":[707,-707,0,0],\"scale\":[100,100,100]},\"gate\":0,\"start\":true,\"finish\":false},{\"prefab\":88,\"trans\":{\"pos\":[1500,0,1500],\"rot\":[707,-707,0,0],\"scale\":[100,100,100]},\"gate\":1,\"start\":false,\"finish\":true}],\"barriers\":[%1{\"prefab\":422,\"trans\":{\"pos\":[1500,0,500],\"rot\":[1000,0,0,0],\"scale\":[100,100,100]}}]}";
  const QString prefabTemplate = "{\"prefab\":%0,\"trans\":{\"pos\":[%1,%2,%3],\"rot\":[%4,%5,%6,%7],\"scale\":[%8,%9,%10]}}";
  QString prefabs;

  const QList<QVector3D> points = {{0, 0.01f, -1}, {1, 0.01f, 0}, {0, 0.01f, 1}, {0, 1.01f, 1}};
  const QList<QVector3D> pointIdx = {{0, 1, 2}, {0, 3, 2}, {1, 3, 2}};

  foreach (QVector3D point, points) {
    prefabs += prefabTemplate
            .arg(357)
            .arg(int(point.x() * 10000), 0, 10)
            .arg(int(point.y() * 10000), 0, 10)
            .arg(int(point.z() * 10000), 0, 10)
            .arg(1000)
            .arg(0)
            .arg(0)
            .arg(0)
            .arg(100)
            .arg(100)
            .arg(100) + ",";
  }

  for (int i = 0; i < points.count(); ++i) {
    for(int j = i + 1; j < points.count(); ++j) {
      const QVector3D position = points[i];
      const QVector3D direction = points[j] - points[i];
      //const QQuaternion result = QQuaternion::fromDirection(direction, up);
      //const QVector3D directionInv = points[i] - points[j];
      //const QQuaternion result = QQuaternion::rotationTo(defaultDirection, direction);
      const QQuaternion result = getPrefabRotation(direction);
      qDebug() << "Pos:" << position << " Tar:" << points[j] << "Dir:" << direction << " Quart:" << result;
      prefabs += prefabTemplate
              .arg(340)

              .arg(int((points[j].x() * 10000) + (points[j].distanceToPoint(points[i]) / 2)), 0, 10)
              .arg(int(points[j].y() * 10000), 0, 10)
              .arg(int(points[j].z() * 10000), 0, 10)

              .arg(int(result.x() * 1000))
              .arg(int(result.y() * 1000))
              .arg(int(result.z() * 1000))
              .arg(int(result.scalar() * 1000))


              .arg(15 + int(points[i].distanceToPoint(points[j]) * 10000))
              .arg(150)
              .arg(150) + ",";
    }
  }


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

QQuaternion GeodesicDome::getPrefabRotation(const QVector3D& direction) const
{
  const QVector3D up(0, 1, 0);
  const QVector3D forward(1, 0, 0);
  if (direction == up) {
    return QQuaternion::rotationTo(forward, direction);
  } else {
    QVector3D v = direction + up * -QVector3D::dotProduct(up, direction);
    QQuaternion rotation = QQuaternion::rotationTo(forward, v);
    return QQuaternion::rotationTo(v, direction) * rotation;
  }
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

