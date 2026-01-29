#pragma once
#include <gdal_priv.h>

#include <QButtonGroup>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFuture>
#include <QInputDialog>
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
#include <osgEarth/MapNode>
#include <osgEarth/ModelLayer>
#include <osgEarth/ShaderUtils>
#include <osgEarth/VirtualProgram>
#include <osgEarthAnnotation/ModelNode>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthUtil/ExampleResources>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TrackballDragger>
#include <osgQt/GraphicsWindowQt>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <vector>

#include "Controller.h"
#include "CoordinateHandler.h"
#include "NoticeToast.h"
#include "ui_OSGEarthApp.h"
#include "ManipulatorHelper.h"
#include "AssetLoader.h"
#include "ProjectDAO.h"
#include "InteractionManager.h"
#include "utils.h"

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

  enum class WorkMode {
    NewMode,    // 新建模式：可写项目名，可增删，有保存
    EditMode,   // 编辑模式：只读项目名，可增删，有保存
    BrowseMode  // 浏览模式：只读项目名，不可增删，无保存
  };

  void applyFullLayout(int w, int h);
  void onReShow();
  void resetScene();
  bool getEarthInitStatus() const { return m_earth_init; }
  void addFileToTree(QString filePath);
  void registerLoadedObject(const QString& filePath, osg::Object* obj);
  void setWorkMode(WorkMode mode, QString pName = "");
  void updateUIByWorkMode();

 signals:
  void projectSavedSuccess();

 public slots:

  void onSlotClose();
  void onSavePro();
  void onLoadScene(QString filePath = QString(), double lon=0.0, double lat=0.0,
                   double alt=0.0);
  void onMeasure(bool checked);

 private:
  void initEnvironment();    // 路径与环境配置
  void initOSGViewer();      // 相机、渲染窗口与操作器
  void initUIConnections();  // 信号与槽绑定
  void startAsyncLoad();     // 异步读取地球模型
  void flyToNode(osg::Node* node);
  void flyToLayer(osgEarth::Layer* layer);
  void initSceneManagerTree();
  void onSlotLocateFile(QString path);
  void onSlotDeleteFile(QString path);
  //void enterTowerEditMode(const QString& path);
  //void onCoordinateChanged(double lon, double lat, double alt, bool isOut);
  void initMembers();
  
  //void startCollapseAnimation();

 private:
  Ui::OSGEarthAppClass ui;
  QTimer* m_frameTimer = nullptr;
  QButtonGroup* m_cameraGroup = nullptr;
  QWidget* m_glWidget;
  QString m_lastOpenPath;
  double m_lon, m_lat, m_alt = 0.0;

  // TreeWidget 的分类根节点映射（如 "影像", "点云"）
  QMap<QString, QTreeWidgetItem*> m_categoryNodes;

  // 文件路径与 OSG 对象的映射（核心资源管理器）
  QMap<QString, osg::ref_ptr<osg::Object>> m_pathNodeMap;

  // 文件路径与 TreeWidget 列表项的映射（用于同步 UI 删除）
  QMap<QString, QTreeWidgetItem*> m_pathItemMap;

  // 文件后缀名与加载函数指针的映射表（优雅的分流器）
  //std::map<QString, SceneLoaderFunc> m_sceneFuncMap;

  // 当前项目内存数据（记录已加载的文件列表等信息）
  ProjectMemoryData m_currentData;

  // 地球环境是否初始化完成的标志
  bool m_earth_init = false;

  // 当前工作模式（编辑、浏览、新建）
  WorkMode m_currentMode;

  // 自定义的浮动提示通知框（Toast 效果）
  NoticeToast* m_noticeToast;

  Controller* m_currentTower;
  ManipulatorHelper* m_towerManip;
  AssetLoader* m_assetLoader;
  InteractionManager* m_interManager;

  // OSG
  osg::ref_ptr<osgViewer::Viewer> m_viewer;
  osg::ref_ptr<osgEarth::Util::EarthManipulator> m_earthManipulator;
  osg::ref_ptr<osg::Group> m_root;
  osg::ref_ptr<osg::Node> m_earthNode;
  osg::ref_ptr<osgEarth::MapNode> m_mapNode;
  osg::ref_ptr<const osgEarth::SpatialReference> m_geoSRS;
  osg::ref_ptr<osg::Camera> m_camera;
  osg::AnimationPathCallback* m_aniCallback = nullptr;
  osg::ref_ptr<CoordinateHandler> m_coordHandler;

  // 数据节点分组，专门用于挂载 OBJ、LAS 等非图层类模型
  osg::ref_ptr<osg::Group> m_dataGroup;
};