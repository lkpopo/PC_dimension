#pragma once
#include <QObject>
#include <QPainter>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Group>
#include <osg/LineWidth>
#include <osg/Point>
#include <osg/PolygonOffset>
#include <osgDB/FileUtils>
#include <osgEarth/MapNode>
#include <osgGA/GUIEventHandler>
#include <osgText/Text>
#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>

#include "utils.h"

// 定义交互模式
enum class InterMode {
  VIEW,     // 普通查看模式
  MEASURE,  // 测距模式
  CLIP      // 剖切模式
};

class InteractionManager : public QObject, public osgGA::GUIEventHandler {
  Q_OBJECT
 public:
  explicit InteractionManager(osgViewer::Viewer* viewer, osg::Group* dataGroup,
                              osg::Group* interactionGroup,
                              QObject* parent = nullptr);

  // 切换模式
  void setMode(InterMode mode);

  // 清除所有测量痕迹
  void clearAll();

  // OSG 事件处理回调
  virtual bool handle(const osgGA::GUIEventAdapter& ea,
                      osgGA::GUIActionAdapter& aa) override;
  void updateClipUniforms(osg::Group* dataGroup, int mode);

 signals:
  void requestContextMenu(osg::Vec3d worldPos);

 private:
  bool handleMeasure(const osgGA::GUIEventAdapter& ea,
                     osgGA::GUIActionAdapter& aa);
  bool handleClip(const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter& aa);
  void refreshGeometry(osg::Geometry* geom);

  bool pick(float x, float y, osg::Vec3d& out_pos);
  void updateMeasureLine(const osg::Vec3d& start, const osg::Vec3d& end);

  void updateText(const osg::Vec3d& worldPos, double distance);
  void initMeasureGeometry();  // 初始化测距线和文字样式
  void initClipGeometry();     // 初始化裁剪线框样式

  void setupClipShader(osg::Group* dataGroup);

 private:
  osg::observer_ptr<osgViewer::Viewer> m_viewer;

  InterMode m_currentMode = InterMode::VIEW;

  // 测距相关的临时变量
  bool m_isFirstClick = true;
  osg::Vec3d m_startPoint;

  // 测距相关的变量
  osg::ref_ptr<osg::Group> m_tempGroup;  // 存放测量线和点的组
  osg::ref_ptr<osg::Vec3Array> m_lineCoords;
  osg::ref_ptr<osg::Geometry> m_lineGeom;
  osgText::Text* m_distanceText = nullptr;
  osg::ref_ptr<osg::Geode> m_textGeode;
  osg::ref_ptr<osg::AutoTransform> m_textAT;

  // 裁剪相关的变量
  osg::ref_ptr<osg::Vec3Array> m_clipCoords;  // 存储多边形顶点
  osg::ref_ptr<osg::Geometry> m_clipGeom;     // 绘制线框的几何体
  osg::ref_ptr<osg::Geode> m_clipGeode;       // 容器
  bool m_isClipFinished = false;              // 标记是否闭合完成

  // 记录当前鼠标实时拾取的终点
  osg::Vec3d m_currentMousePoint;
};