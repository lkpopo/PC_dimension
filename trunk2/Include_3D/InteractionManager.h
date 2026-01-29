#pragma once
#include <QObject>
#include <osg/Geode>
#include <osg/Group>
#include <osgDB/FileUtils>
#include <osgEarth/MapNode>
#include <osgGA/GUIEventHandler>
#include <osgText/Text>
#include <osgViewer/Viewer>

#include "utils.h"

// 定义交互模式
enum class InterMode {
  VIEW,        // 普通查看模式
  MEASURE,     // 测距模式
  CLIP         // 剖切模式
};

class InteractionManager : public QObject, public osgGA::GUIEventHandler {
  Q_OBJECT
 public:
  explicit InteractionManager(osgViewer::Viewer* viewer, osg::Group* dataGroup,
                              QObject* parent = nullptr);

  // 切换模式
  void setMode(InterMode mode);

  // 清除所有测量痕迹
  void clearAll();

  // OSG 事件处理回调
  virtual bool handle(const osgGA::GUIEventAdapter& ea,
                      osgGA::GUIActionAdapter& aa) override;

 signals:
  // 发出信号给 UI 显示结果
  void signalPointPicked(osg::Vec3d worldPos);

 private:
  bool handleMeasure(const osgGA::GUIEventAdapter& ea,
                     osgGA::GUIActionAdapter& aa);
  bool handleClip(const osgGA::GUIEventAdapter& ea,
                  osgGA::GUIActionAdapter& aa);
  void refreshClipGeometry();

  bool pick(float x, float y, osg::Vec3d& out_pos);
  void updateMeasureLine(const osg::Vec3d& start, const osg::Vec3d& end);

  void updateText(const osg::Vec3d& pos,
                  double distance);  // 更新文字位置和内容
  void initMeasureGeometry();        // 初始化测距线和文字样式
  void initClipGeometry();           // 初始化裁剪线框样式

 private:
  osg::observer_ptr<osgViewer::Viewer> m_viewer;
  osg::observer_ptr<osg::Group> m_dataGroup;

  InterMode m_currentMode = InterMode::VIEW;

  // 测距相关的临时变量
  bool m_isFirstClick = true;
  osg::Vec3d m_startPoint;

  // 测距相关的变量
  osg::ref_ptr<osg::Group> m_tempGroup;  // 存放测量线和点的组
  osg::ref_ptr<osg::Vec3Array> m_lineCoords;
  osg::ref_ptr<osg::Geometry> m_lineGeom;

  // 裁剪相关的变量
  osg::ref_ptr<osg::Vec3Array> m_clipCoords;  // 存储多边形顶点
  osg::ref_ptr<osg::Geometry> m_clipGeom;      // 绘制线框的几何体
  osg::ref_ptr<osg::Geode> m_clipGeode;        // 容器
  bool m_isClipFinished = false;               // 标记是否闭合完成

  osg::ref_ptr<osgText::Text> m_distanceText;
  osg::ref_ptr<osg::Geode> m_textGeode;

  // 记录当前鼠标实时拾取的终点
  osg::Vec3d m_currentMousePoint;
};