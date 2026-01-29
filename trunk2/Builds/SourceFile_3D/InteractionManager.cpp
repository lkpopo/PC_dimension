#include "InteractionManager.h"

#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PolygonOffset>
#include <osgUtil/LineSegmentIntersector>

InteractionManager::InteractionManager(osgViewer::Viewer* viewer,
                                       osg::Group* dataGroup, QObject* parent)
    : QObject(parent), m_viewer(viewer), m_dataGroup(dataGroup) {
  m_tempGroup = new osg::Group();
  // 将临时绘图层加入场景，确保它不会被遮挡
  if (m_dataGroup.valid()) {
    m_dataGroup->addChild(m_tempGroup);
  }
  initMeasureGeometry();
  initClipGeometry();
}

void InteractionManager::setMode(InterMode mode) {
  m_currentMode = mode;
  if (mode == InterMode::VIEW) {
    clearAll();
    m_tempGroup->setNodeMask(0);  // 关键：0 表示彻底不渲染，不论坐标在哪
  } else {
    m_tempGroup->setNodeMask(0xffffffff);  // 恢复渲染
  }
  m_isFirstClick = true;  // 切换模式时重置状态
}

void InteractionManager::clearAll() {
  // 保留前两个节点：0是线，最后一个（或特定的）是文字
  // 简单的做法是直接清空文字
  if (m_distanceText.valid()) m_distanceText->setText("");

  // 线条归零
  if (!m_lineCoords->empty()) {
    m_lineCoords->clear();
    m_lineGeom->dirtyBound();
    m_lineGeom->dirtyDisplayList();
  }
  m_isFirstClick = true;
}

bool InteractionManager::handle(const osgGA::GUIEventAdapter& ea,
                                osgGA::GUIActionAdapter& aa) {
  if (m_currentMode == InterMode::VIEW) return false;

  if (ea.getEventType() == osgGA::GUIEventAdapter::DRAG ||
      ea.getEventType() == osgGA::GUIEventAdapter::SCROLL) {
    return true;
  }

  switch (m_currentMode) {
    case InterMode::MEASURE:
      return handleMeasure(ea, aa);
    case InterMode::CLIP:
      return handleClip(ea, aa);
    default:
      return false;
  }
}

bool InteractionManager::pick(float x, float y, osg::Vec3d& out_pos) {
  double delta = 5.0;
  osg::ref_ptr<osgUtil::PolytopeIntersector> intersector =
      new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, x - delta,
                                       y - delta, x + delta, y + delta);

  // 重点：设置拾取优先级，让它尽可能寻找点云数据
  intersector->setIntersectionLimit(osgUtil::Intersector::LIMIT_NEAREST);

  osgUtil::IntersectionVisitor iv(intersector.get());
  if (m_currentMode == InterMode::CLIP)
    iv.setTraversalMask(MASK_TERRAIN);
  else
    iv.setTraversalMask(0xffffffff);

  m_viewer->getCamera()->accept(iv);

  if (intersector->containsIntersections()) {
    auto& intersections = intersector->getIntersections();

    // 遍历所有交点，找到第一个属于点云的交点
    for (auto& intersection : intersections) {
      // 过滤掉 osgEarth 的地形，只找你的点云节点
      // 假设你的点云节点名字包含 "Point" 或属于某个特定 Group
      // if (intersection.nodePath.empty()) continue;

      osg::Vec3 localPoint = intersection.localIntersectionPoint;
      osg::Matrix localToWorld =
          osg::computeLocalToWorld(intersection.nodePath);
      out_pos = localPoint * localToWorld;
      return true;
    }
  }
  return false;
}

void InteractionManager::updateMeasureLine(const osg::Vec3d& start,
                                           const osg::Vec3d& end) {
  if (m_lineCoords->size() < 2) {
    m_lineCoords->resize(2);
  }
  (*m_lineCoords)[0] = start;
  (*m_lineCoords)[1] = end;

  m_lineCoords->dirty();
  m_lineGeom->dirtyBound();
  m_lineGeom->dirtyDisplayList();
}

void InteractionManager::updateText(const osg::Vec3d& pos, double distance) {
  if (!m_distanceText.valid()) return;

  // 设置内容
  QString text = QString("%1 m").arg(distance, 0, 'f', 2);
  m_distanceText->setText(text.toStdString(), osgText::String::ENCODING_UTF8);

  // 设置位置（在拾取点稍微偏移一点，避免被鼠标指针遮挡）
  m_distanceText->setPosition(pos);
}

void InteractionManager::initClipGeometry() {
  m_clipGeom = new osg::Geometry();
  m_clipCoords = new osg::Vec3Array();
  m_clipGeom->setVertexArray(m_clipCoords);

  // 使用 GL_LINE_LOOP 会自动将最后一个点与第一个点连接，形成闭合
  m_clipGeom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 0));

  m_clipGeode = new osg::Geode();
  m_clipGeode->addDrawable(m_clipGeom);

  // 设置裁剪框样式：亮绿色，带点粗细，且永远显示在最前
  osg::StateSet* ss = m_clipGeode->getOrCreateStateSet();
  ss->setMode(GL_DEPTH_TEST,
              osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
  ss->setAttributeAndModes(new osg::LineWidth(2.5f), osg::StateAttribute::ON);
  ss->setRenderBinDetails(10000, "RenderBin");

  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
  m_clipGeom->setColorArray(colors, osg::Array::BIND_OVERALL);

  m_tempGroup->addChild(m_clipGeode);
}

void InteractionManager::initMeasureGeometry() {
  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));

  // 初始化线几何体
  m_lineGeom = new osg::Geometry();
  m_lineCoords = new osg::Vec3Array();
  m_lineCoords->push_back(osg::Vec3(0, 0, 0));
  m_lineCoords->push_back(osg::Vec3(0, 0, 0));
  m_lineGeom->setVertexArray(m_lineCoords);
  m_lineGeom->addPrimitiveSet(new osg::DrawArrays(GL_LINES, 0, 2));
  m_lineGeom->setColorArray(colors, osg::Array::BIND_OVERALL);  // 设置线条颜色

  osg::Geode* geode = new osg::Geode();
  geode->setName("MeasureLineGeode");
  osg::StateSet* ss = geode->getOrCreateStateSet();

  ss->setAttributeAndModes(new osg::LineWidth(2.0f));  // 设置线条宽度
  osg::ref_ptr<osg::PolygonOffset> polyoffset =
      new osg::PolygonOffset(-1.0f, -1.0f);
  ss->setAttributeAndModes(polyoffset, osg::StateAttribute::ON);
  ss->setRenderBinToInherit();
  geode->addDrawable(m_lineGeom);

  m_distanceText = new osgText::Text();
  m_distanceText->setCharacterSize(5.0f);                       // 字体大小
  m_distanceText->setColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));  // 红色
  // 设置字体对齐方式（跟随屏幕旋转，始终面向用户）
  m_distanceText->setAxisAlignment(osgText::Text::SCREEN);
  // 设置背景，防止文字在复杂背景下看不清
  m_distanceText->setBackdropType(osgText::Text::OUTLINE);

  m_textGeode = new osg::Geode();
  m_textGeode->addDrawable(m_distanceText);
  m_textGeode->getOrCreateStateSet()->setRenderBinDetails(
      12, "RenderBin");  // 比线条层级更高一点

  m_tempGroup->addChild(m_textGeode);  // 加入临时组

  m_tempGroup->addChild(geode);  // 默认是第0个子节点
}

bool InteractionManager::handleMeasure(const osgGA::GUIEventAdapter& ea,
                                       osgGA::GUIActionAdapter& aa) {
  if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE) {
    // 只有在选了第一个点之后，才开始动态拉线
    if (!m_isFirstClick) {
      osg::Vec3d hitPos;
      if (pick(ea.getX(), ea.getY(), hitPos)) {
        m_currentMousePoint = hitPos;
        updateMeasureLine(m_startPoint, m_currentMousePoint);
        double dist = (m_currentMousePoint - m_startPoint).length();
        updateText(m_currentMousePoint, dist);
      }
    }
    return false;  // 移动事件不拦截，否则地图没法转了
  }

  // 仅响应鼠标左键按下
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
    osg::Vec3d hitPos;
    if (pick(ea.getX(), ea.getY(), hitPos)) {
      if (m_currentMode == InterMode::MEASURE) {
        if (m_isFirstClick) {
          clearAll();
          m_startPoint = hitPos;
          m_isFirstClick = false;
        } else {
          updateMeasureLine(m_startPoint, hitPos);
          m_isFirstClick = true;  // 测完一次重置
        }
      }
      return true;
    }
  }
  return false;
}

bool InteractionManager::handleClip(const osgGA::GUIEventAdapter& ea,
                                    osgGA::GUIActionAdapter& aa) {
  float x = ea.getX();
  float y = ea.getY();

  // --- 1. 鼠标移动：更新橡皮筋 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE) {
    if (m_clipCoords->size() >= 2 && !m_isClipFinished) {
      osg::Vec3d hitPos;
      if (pick(x, y, hitPos)) {
        if (m_clipCoords->size() >= 2) {
          m_clipCoords->back() = hitPos;
          refreshClipGeometry();
        }
      }
    }
    return false;
  }

  // --- 2. 鼠标左键：固定顶点 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
    osg::Vec3d hitPos;
    if (pick(x, y, hitPos)) {
      if (m_isClipFinished) {
        m_clipCoords->clear();
        m_isClipFinished = false;
      }

      if (m_clipCoords->empty()) {
        // 第一次点击：放两个点（一个固定起点，一个跟随鼠标的终点）
        m_clipCoords->push_back(hitPos);
        m_clipCoords->push_back(hitPos);
      } else {
        m_clipCoords->push_back(hitPos);
      }
      refreshClipGeometry();
      return true;
    }
  }

  // --- 3. 鼠标右键：闭合区域 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
    if (m_clipCoords->size() > 2) {
      m_clipCoords->pop_back();  // 移除最后那个跟随鼠标的临时点
      m_isClipFinished = true;
      refreshClipGeometry();
    }
    return true;
  }

  return false;
}

/**
 * 刷新几何体状态
 * 作用：更新顶点计数，并通知 OSG 重新渲染
 */
void InteractionManager::refreshClipGeometry() {
  if (!m_clipGeom.valid() || !m_clipCoords.valid()) return;

  // 1. 更新 DrawArrays 的顶点数量
  osg::DrawArrays* da =
      dynamic_cast<osg::DrawArrays*>(m_clipGeom->getPrimitiveSet(0));
  if (da) {
    da->setCount(m_clipCoords->size());
  }

  // 2. 标记数据已更新
  m_clipGeom->dirtyBound();
  m_clipGeom->dirtyDisplayList();
}

/**
 * 更新橡皮筋线
 * @param hitPos 当前鼠标指向的 3D 世界坐标
 */
// void InteractionManager::updateClipRubberBand(const osg::Vec3d& hitPos) {
//   if (m_clipCoords->empty() || m_isClipFinished) return;
//
//   if (m_clipCoords->size() >= 2) {
//     m_clipCoords->back() = hitPos;
//     refreshClipGeometry();
//   }
// }