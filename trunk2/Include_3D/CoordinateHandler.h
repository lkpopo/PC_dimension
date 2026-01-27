#pragma once
#include <QObject>
#include <osgEarth/GeoData>
#include <osgEarth/MapNode>
#include <osgGA/GUIEventHandler>
#include <osgEarth/Terrain>
#include <osgViewer/View>

// 继承 QObject 以使用信号槽
class CoordinateHandler : public QObject, public osgGA::GUIEventHandler {
  Q_OBJECT

 public:
  CoordinateHandler(osgEarth::MapNode* mapNode);

  // 重写事件处理函数
  virtual bool handle(const osgGA::GUIEventAdapter& ea,
                      osgGA::GUIActionAdapter& aa) override;

 signals:
  // 定义信号，将坐标信息发送给 UI 界面
  void signalMouseCoords(double lon, double lat, double alt, bool isOut);

 private:
  osg::observer_ptr<osgEarth::MapNode> m_mapNode;
};