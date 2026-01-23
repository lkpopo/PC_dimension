#pragma once
#include <QButtonGroup>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFuture>
#include <QLineEdit>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlQuery>
#include <QTimer>
#include <QWidget>
#include <QtConcurrent>
#include <QtWidgets/QWidget>
#include <osg/Camera>
#include <osg/Group>
#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgEarth/ImageLayer>
#include <osgEarth/ModelLayer>
#include <osgEarth/MapNode>
#include <osgEarth/ShaderUtils>
#include <osgEarth/VirtualProgram>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgQt/GraphicsWindowQt>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <vector>

#include "ui_OSGEarthApp.h"

using namespace std;

struct ProjectMemoryData {
  QString projectName;
  QStringList allFiles;  // 所有的 las, tif, shp 都混在一起

  void clear() {
    projectName.clear();
    allFiles.clear();
  }
};

class OSGEarthApp : public QWidget {
  Q_OBJECT

 public:
  explicit OSGEarthApp(QWidget* parent = nullptr);
  ~OSGEarthApp();

  void applyFullLayout(int w, int h);
  void onReShow();
  void resetScene();
  void setEditMode(bool edit, QString pName);
  bool getEarthInitStatus() const { return m_earth_init; }
  void addFileToTree(QString filePath);
  void registerLoadedObject(const QString& filePath, osg::Object* obj);

 signals:
  void projectSavedSuccess();

 public slots:
  osg::Object* onSlotLoadLas(const QString& filePath);
  void onSlotLasEle();
  osg::Object* onSlotTif(const QString& filePath);
  osg::Object* onSlotElevation(const QString& filePath);
  osg::Object* onSlotShp(const QString& filePath);
  osg::Object* onSlotLoadObj(const QString& filePath);
  void onSlotClose();
  void onSavePro();
  void onLoadScene();

 private:
  void initEnvironment();    // 路径与环境配置
  void initOSGViewer();      // 相机、渲染窗口与操作器
  void initUIConnections();  // 信号与槽绑定
  void startAsyncLoad();     // 异步读取地球模型
  void flyToNode(osg::Node* node);
  void flyToImageCenter(const QString& filePath);
  void initSceneManagerTree();
  void onSlotLocateFile(QString path);
  void onSlotDeleteFile(QString path);
  void resetSceneUI();

 private:
  Ui::OSGEarthAppClass ui;
  QTimer* m_frameTimer = nullptr;
  QButtonGroup* m_cameraGroup = nullptr;
  QWidget* m_glWidget;
  QString m_lastOpenPath;
  QMap<QString, QTreeWidgetItem*> m_categoryNodes;
  QMap<QString, osg::ref_ptr<osg::Object>> m_pathNodeMap;
  QMap<QString, QTreeWidgetItem*> m_pathItemMap;

  // 业务数据
  ProjectMemoryData m_currentData;
  std::string m_tifPath;
  std::string m_elePath;
  double m_lon = 0.0;
  double m_lat = 0.0;
  bool m_isEditMode = false;
  bool m_earth_init = false;

  // OSG
  osg::ref_ptr<osgViewer::Viewer> m_viewer;
  osg::ref_ptr<osgEarth::Util::EarthManipulator> m_earthManipulator;
  osg::ref_ptr<osg::Group> m_root;
  osg::ref_ptr<osg::Node> m_earthNode;
  osg::ref_ptr<osgEarth::MapNode> m_mapNode;
  osg::ref_ptr<const osgEarth::SpatialReference> m_geoSRS;
  osg::ref_ptr<osgEarth::ImageLayer> m_layer;
  osg::ref_ptr<osgEarth::ElevationLayer> m_eleLayer;
  osg::ref_ptr<osg::Camera> m_camera;
  osg::AnimationPathCallback* m_aniCallback = nullptr;
  osg::ref_ptr<osg::Group> m_dataGroup;
};