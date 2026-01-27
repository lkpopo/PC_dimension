#include "InteractionManager.h"

#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Point>
#include <osgUtil/LineSegmentIntersector>

InteractionManager::InteractionManager(osgViewer::Viewer* viewer,
                                       osg::Group* dataGroup,
                                       QObject* parent)
    : QObject(parent), m_viewer(viewer), m_dataGroup(dataGroup) {
  m_tempGroup = new osg::Group();
  // 将临时绘图层加入场景，确保它不会被遮挡
  if (m_dataGroup.valid()) {
    m_dataGroup->addChild(m_tempGroup);
  }

  // 初始化线几何体
  m_lineGeom = new osg::Geometry();
  m_lineCoords = new osg::Vec3Array();
  m_lineCoords->push_back(osg::Vec3(0, 0, 0));
  m_lineCoords->push_back(osg::Vec3(0, 0, 0));
  m_lineGeom->setVertexArray(m_lineCoords);
  m_lineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));

  osg::Geode* geode = new osg::Geode();
  geode->addDrawable(m_lineGeom);
  geode->getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(20.0f));
  geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
  m_tempGroup->addChild(geode);
  //m_dataGroup->addChild(geode);
}

void InteractionManager::setMode(InterMode mode) {
  m_currentMode = mode;
  m_isFirstClick = true;  // 切换模式时重置状态
}

void InteractionManager::clearAll() {
  m_tempGroup->removeChild(0, m_tempGroup->getNumChildren());
  m_lineCoords->clear();
  m_lineGeom->dirtyDisplayList();
  m_isFirstClick = true;
}

bool InteractionManager::handle(const osgGA::GUIEventAdapter& ea,
                                osgGA::GUIActionAdapter& aa) {
  if (m_currentMode == InterMode::VIEW) return false;

  // 仅响应鼠标左键按下
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
    osg::Vec3d hitPos;
    if (pick(ea.getX(), ea.getY(), hitPos)) {
      if (m_currentMode == InterMode::MEASURE) {
        if (m_isFirstClick) {
          clearAll();
          m_startPoint = hitPos;
          addMeasurePoint(hitPos);
          m_isFirstClick = false;
        } else {
          addMeasurePoint(hitPos);
          updateMeasureLine(m_startPoint, hitPos);

          // 计算距离
          // (如果是地理坐标，这里需要计算大地线距离，目前先做简单欧氏距离)
          double dist = (hitPos - m_startPoint).length();
          emit signalDistanceMeasured(dist);

          m_isFirstClick = true;  // 测完一次重置
        }
      } else if (m_currentMode == InterMode::PICK_POINT) {
        emit signalPointPicked(hitPos);
      }
      return true;  // 事件已处理，不向下传递
    }
  }
  return false;
}

bool InteractionManager::pick(float x, float y, osg::Vec3d& out_pos) {
  // 创建相交检测器，针对点云建议使用小窗口容差
  osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
      new osgUtil::LineSegmentIntersector(osgUtil::Intersector::WINDOW, x, y);

  osgUtil::IntersectionVisitor iv(intersector.get());
  m_viewer->getCamera()->accept(iv);

  if (intersector->containsIntersections()) {
    auto result = intersector->getFirstIntersection();
    out_pos = result.getWorldIntersectPoint();
    return true;
  }
  return false;
}

void InteractionManager::addMeasurePoint(const osg::Vec3d& pos) {
  // 绘制一个小点或球体表示点击位置
  osg::Geode* geode = new osg::Geode();
  osg::Geometry* geom = new osg::Geometry();
  osg::Vec3Array* vertices = new osg::Vec3Array();
  vertices->push_back(pos);
  geom->setVertexArray(vertices);
  geom->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, 1));

  geode->addDrawable(geom);
  geode->getOrCreateStateSet()->setAttributeAndModes(new osg::Point(8.0f));
  m_tempGroup->addChild(geode);
}

void InteractionManager::updateMeasureLine(const osg::Vec3d& start,
                                           const osg::Vec3d& end) {
  m_lineCoords->clear();
  m_lineCoords->push_back(start);
  m_lineCoords->push_back(end);
  m_lineGeom->setVertexArray(m_lineCoords);
  m_lineGeom->dirtyDisplayList();
  m_lineGeom->dirtyBound();
}