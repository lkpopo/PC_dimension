#include "CoordinateHandler.h"

CoordinateHandler::CoordinateHandler(osgEarth::MapNode* mapNode)
    : m_mapNode(mapNode) {}

bool CoordinateHandler::handle(const osgGA::GUIEventAdapter& ea,
                               osgGA::GUIActionAdapter& aa) {
  if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE) {
    osgViewer::View* view = static_cast<osgViewer::View*>(&aa);

    // 1. 发射一条射线检测与地球的交点
    osgUtil::LineSegmentIntersector::Intersections intersections;
    if (view->computeIntersections(ea.getX(), ea.getY(), intersections)) {
      // 获取第一个交点（离相机最近的点）
      osg::Vec3d worldPos = intersections.begin()->getWorldIntersectPoint();

      // 2. 将世界坐标转为经纬度
      osgEarth::GeoPoint geoPos;
      geoPos.fromWorld(m_mapNode->getMapSRS(), worldPos);

      emit signalMouseCoords(geoPos.x(), geoPos.y(), geoPos.z(), false);
    } else {
      emit signalMouseCoords(0, 0, 0, true);
    }
  }
  return false;
}