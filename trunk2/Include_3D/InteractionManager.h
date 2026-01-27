#pragma once
#include <QObject>
#include <osg/Geode>
#include <osg/Group>
#include <osgEarth/MapNode>
#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>

// 定义交互模式
enum class InterMode {
  VIEW,       // 普通查看模式
  MEASURE,    // 测距模式
  PICK_POINT  // 拾取点模式（用于后续的连线）
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
  void signalDistanceMeasured(double distance);
  void signalPointPicked(osg::Vec3d worldPos);

 private:
  // 执行拾取逻辑
  bool pick(float x, float y, osg::Vec3d& out_pos);

  // 绘制测量视觉效果
  void addMeasurePoint(const osg::Vec3d& pos);
  void updateMeasureLine(const osg::Vec3d& start, const osg::Vec3d& end);

 private:
  osg::observer_ptr<osgViewer::Viewer> m_viewer;
  osg::observer_ptr<osg::Group> m_dataGroup;

  InterMode m_currentMode = InterMode::VIEW;

  // 测距相关的临时变量
  bool m_isFirstClick = true;
  osg::Vec3d m_startPoint;

  // 视觉节点
  osg::ref_ptr<osg::Group> m_tempGroup;  // 存放测量线和点的组
  osg::ref_ptr<osg::Vec3Array> m_lineCoords;
  osg::ref_ptr<osg::Geometry> m_lineGeom;
};