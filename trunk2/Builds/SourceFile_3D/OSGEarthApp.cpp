#include "OSGEarthApp.h"

#include "SceneItemWidget.h"
#include "utils.h"

OSGEarthApp::OSGEarthApp(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
  setAttribute(Qt::WA_QuitOnClose, true);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("osgEarth Eigen APP");

  initEnvironment();       // 1. 环境准备
  initOSGViewer();         // 2. 配置相机和渲染窗口
  initUIConnections();     // 3. 绑定按钮逻辑
  startAsyncLoad();        // 4. 异步加载地球模型
  initSceneManagerTree();  // 初始化场景管理树
}

OSGEarthApp::~OSGEarthApp() {
  auto gw = dynamic_cast<osgQt::GraphicsWindowQt*>(
      m_viewer->getCamera()->getGraphicsContext());
  if (gw) {
    QWidget* glw = gw->getGLWidget();
    if (glw) {
      // 关键：解除父子关系
      // 因为 glWidget 是由 OSG 内部管理的，如果不解除 Parent，
      // 当 OSGEarthApp 析构时，Qt 会尝试 delete glw，导致与 OSG 冲突崩溃。
      glw->setParent(nullptr);
    }
  }

  if (m_viewer.valid()) {
    m_viewer.release();
  }
}

void OSGEarthApp::initEnvironment() {
  // 设置当前工作目录
  QString exeDir = QCoreApplication::applicationDirPath();
  QDir::setCurrent(exeDir);

  m_root = new osg::Group;
  m_root->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,
                                         osg::StateAttribute::ON);
  m_dataGroup = new osg::Group();
  m_root->addChild(m_dataGroup.get());
}

void OSGEarthApp::startAsyncLoad() {
  QtConcurrent::run([this]() {
    osg::ref_ptr<osg::Node> loadedNode =
        osgDB::readNodeFile("./earth/mymap0.earth");

    // 2. 加载完成后，不能直接操作 m_root，必须回到主线程
    // 使用 invokeMethod 安全地在主线程执行挂载
    QMetaObject::invokeMethod(
        this,
        [this, loadedNode]() {
          if (loadedNode.valid()) {
            m_earthNode = loadedNode;
            m_earthNode->setName("earth");
            m_root->addChild(m_earthNode.get());
            m_mapNode = osgEarth::MapNode::findMapNode(m_earthNode.get());

            if (m_mapNode.valid()) {
              m_geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();
            }
            m_earth_init = true;
          }
        },
        Qt::QueuedConnection);
  });
}

void OSGEarthApp::initOSGViewer() {
  // 1. 创建 Traits
  osg::ref_ptr<osg::GraphicsContext::Traits> traits =
      new osg::GraphicsContext::Traits;
  traits->windowDecoration = false;
  traits->width = ui.widgetOSG->width() > 0 ? ui.widgetOSG->width() : 800;
  traits->height = ui.widgetOSG->height() > 0 ? ui.widgetOSG->height() : 600;
  traits->doubleBuffer = true;

  // 2. 创建 GraphicsWindowQt
  osgQt::GraphicsWindowQt* gw = new osgQt::GraphicsWindowQt(traits.get());
  m_glWidget = gw->getGLWidget();  // 建议将 m_glWidget 设为类成员变量

  // 3. 直接加入布局
  if (ui.widgetOSG->layout()) {
    ui.widgetOSG->layout()->setContentsMargins(0, 0, 0, 0);
    ui.widgetOSG->layout()->setSpacing(0);
    ui.widgetOSG->layout()->addWidget(m_glWidget);
  }

  // 3. 创建相机并绑定上下文
  m_camera = new osg::Camera;
  m_camera->setGraphicsContext(gw);
  m_camera->setViewport(new osg::Viewport(0, 0, traits->width, traits->height));
  m_camera->setClearColor(osg::Vec4(0.93, 0.93, 0.93, 1.0));
  m_camera->setProjectionMatrixAsPerspective(
      30.0f,
      static_cast<double>(traits->width) / static_cast<double>(traits->height),
      1.0f, 10000.0f);

  // 4. 配置 Viewer
  m_viewer = new osgViewer::Viewer;
  m_viewer->setCamera(m_camera);
  m_viewer->setSceneData(m_root.get());
  m_viewer->setThreadingModel(osgViewer::Viewer::SingleThreaded);

  // 5. 配置操作器 (Manipulator)
  m_earthManipulator = new osgEarth::Util::EarthManipulator;
  m_viewer->setCameraManipulator(m_earthManipulator.get());

  // 6. 添加事件处理
  m_viewer->addEventHandler(
      new osgGA::StateSetManipulator(m_camera->getOrCreateStateSet()));
  m_viewer->addEventHandler(new osgViewer::StatsHandler);

  if (m_mapNode.valid()) {
    m_camera->addCullCallback(
        new osgEarth::Util::AutoClipPlaneCullCallback(m_mapNode.get()));
  }

  // 7. 绑定缩放事件
  m_earthManipulator->getSettings()->bindScroll(
      osgEarth::Util::EarthManipulator::ACTION_ZOOM_IN,
      osgGA::GUIEventAdapter::SCROLL_UP);
  m_earthManipulator->getSettings()->bindScroll(
      osgEarth::Util::EarthManipulator::ACTION_ZOOM_OUT,
      osgGA::GUIEventAdapter::SCROLL_DOWN);

  // 8. 实例化
  m_viewer->realize();
}

void OSGEarthApp::initUIConnections() {
  connect(ui.btnLoadScene, &QPushButton::clicked, this,
          &OSGEarthApp::onLoadScene);
  connect(ui.btn_Close, &QPushButton::clicked, this, &OSGEarthApp::onSlotClose);
  connect(ui.btnSavePro, &QPushButton::clicked, this, &OSGEarthApp::onSavePro);

  // 渲染定时器
  m_frameTimer = new QTimer(this);
  connect(m_frameTimer, &QTimer::timeout, this, [this]() {
    if (m_viewer.valid()) {
      m_viewer->frame();
    }
  });
  m_frameTimer->start(5);
}

/*-------------------加载场景-------------------*/

osg::Object* OSGEarthApp::onSlotLoadLas(const QString& filePath) {
  QFileInfo lasInfo(filePath);
  QString tilesBaseDir =
      QApplication::applicationDirPath() + "/tiles/" + lasInfo.baseName() + "/";
  QString indexFile = tilesBaseDir + "index.osgb";

  // 检查索引树是否存在 ---
  if (QFileInfo::exists(indexFile)) {
    qDebug() << "Found existing index file, loading...";
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
    options->setDatabasePath(tilesBaseDir.toStdString());

    // 使用 options 加载
    osg::ref_ptr<osg::Node> loadedIndex =
        osgDB::readNodeFile(indexFile.toStdString(), options.get());
    if (loadedIndex.valid()) {
      m_dataGroup->addChild(loadedIndex.get());
      this->flyToNode(loadedIndex.get());
      return loadedIndex.get();
    }
  }

  // 构建瓦片
  QDir().mkpath(tilesBaseDir);
  osg::Group* resultNode =
      convertLasToPagedTiles(filePath, tilesBaseDir, m_geoSRS.get());
  if (resultNode) {
    osgDB::writeNodeFile(*resultNode, indexFile.toStdString());
    m_dataGroup->addChild(resultNode);
    this->flyToNode(resultNode);
  }
  return resultNode;
}

void OSGEarthApp::onSlotLasEle() {
  if (!m_root) {
    osg::notify(osg::FATAL) << "[onSlotLasEle] Root node is null!" << std::endl;
    return;
  }

  osg::notify(osg::ALWAYS)
      << "[onSlotLasEle] Start processing point cloud for elevation coloring..."
      << std::endl;

  auto getColorByRatio = [](float t) -> osg::Vec4 {
    // 多段渐变：蓝->青->绿->黄->红
    if (t < 0.25f) {  // 蓝 -> 青
      float f = t / 0.25f;
      return osg::Vec4(0.0f, f, 1.0f, 1.0f);
    } else if (t < 0.5f) {  // 青 -> 绿
      float f = (t - 0.25f) / 0.25f;
      return osg::Vec4(0.0f, 1.0f, 1.0f - f, 1.0f);
    } else if (t < 0.75f) {  // 绿 -> 黄
      float f = (t - 0.5f) / 0.25f;
      return osg::Vec4(f, 1.0f, 0.0f, 1.0f);
    } else {  // 黄 -> 红
      float f = (t - 0.75f) / 0.25f;
      return osg::Vec4(1.0f, 1.0f - f, 0.0f, 1.0f);
    }
  };

  // 遍历 m_root 的所有子节点
  for (unsigned int i = 0; i < m_root->getNumChildren(); ++i) {
    osg::PagedLOD* plod = dynamic_cast<osg::PagedLOD*>(m_root->getChild(i));
    if (!plod) {
      osg::notify(osg::INFO) << "[onSlotLasEle] Child " << i
                             << " is not a Geode, skip." << std::endl;
      continue;
    }

    for (unsigned int k = 0; k < plod->getNumChildren(); ++k)

    {
      osg::Geode* geode = dynamic_cast<osg::Geode*>(plod->getChild(k));

      if (!geode) {
        osg::notify(osg::INFO) << "[onSlotLasEle] Child " << k
                               << " is not a Geode, skip." << std::endl;
        continue;
      }

      osg::notify(osg::INFO)
          << "[onSlotLasEle] Processing Geode " << k
          << ", drawables: " << geode->getNumDrawables() << std::endl;

      for (unsigned int j = 0; j < geode->getNumDrawables(); ++j) {
        osg::Geometry* geom =
            dynamic_cast<osg::Geometry*>(geode->getDrawable(j));
        if (!geom) {
          osg::notify(osg::INFO) << "[onSlotLasEle] Drawable " << j
                                 << " is not Geometry, skip." << std::endl;
          continue;
        }

        osg::Vec3Array* vertices =
            dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
        osg::Vec4Array* colors =
            dynamic_cast<osg::Vec4Array*>(geom->getColorArray());

        if (!vertices) {
          osg::notify(osg::INFO) << "[onSlotLasEle] Geometry " << j
                                 << " has no vertices, skip." << std::endl;
          continue;
        }

        if (!colors) {
          colors = new osg::Vec4Array();
          colors->resize(vertices->size());
          geom->setColorArray(colors);
          geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
          osg::notify(osg::INFO)
              << "[onSlotLasEle] Created new color array for Geometry " << j
              << std::endl;
        }

        // 找到 Z 的最小值和最大值
        float zMin = std::numeric_limits<float>::max();
        float zMax = -std::numeric_limits<float>::max();
        for (auto& v : *vertices) {
          if (v.z() < zMin) zMin = v.z();
          if (v.z() > zMax) zMax = v.z();
        }

        osg::notify(osg::ALWAYS)
            << "[onSlotLasEle] Geometry " << j << " zMin=" << zMin
            << ", zMax=" << zMax << ", points=" << vertices->size()
            << std::endl;

        // 按高度映射颜色：蓝->红
        for (size_t k = 0; k < vertices->size(); ++k) {
          float t = (vertices->at(k).z() - zMin) / (zMax - zMin);
          (*colors)[k] = getColorByRatio(t);
        }

        geom->dirtyDisplayList();
        geom->dirtyBound();

        osg::notify(osg::ALWAYS)
            << "[onSlotLasEle] Geometry " << j
            << " color updated based on elevation." << std::endl;
      }
    }
  }

  // 更新场景
  m_viewer->setSceneData(m_root);
  osg::notify(osg::ALWAYS)
      << "[onSlotLasEle] All point clouds processed for elevation."
      << std::endl;
}

osg::Object* OSGEarthApp::onSlotTif(const QString& filePath) {
  // 1. 构建 GDAL 图层配置
  std::string tifName =
      QFileInfo(filePath).fileName().toLocal8Bit().constData();
  std::string tifUrl = filePath.toLocal8Bit().constData();

  // 2. 扁平化构建 Config (参考你成功的案例)
  osgEarth::Config layerConf;
  layerConf.key() = "image";
  layerConf.add("name", tifName);
  layerConf.add("driver", "gdal");
  layerConf.add("url", tifUrl);
  layerConf.add("fadeInDuration", "0");
  layerConf.add("min_level", "0");
  layerConf.add("max_data_level", "25");

  // 2. 创建 ImageLayer
  osgEarth::ImageLayerOptions layerOpt(layerConf);
  osg::ref_ptr<osgEarth::ImageLayer> layer = new osgEarth::ImageLayer(layerOpt);

  if (!layer.valid()) return nullptr;

  m_mapNode->getMap()->addImageLayer(layer);

  flyToImageCenter(filePath);
  return layer;
}

osg::Object* OSGEarthApp::onSlotElevation(const QString& filePath) {
  QByteArray by = filePath.toLocal8Bit();
  m_elePath = by.constData();

  QFileInfo fileinfo = QFileInfo(filePath);

  QString file_name = fileinfo.fileName();
  QByteArray by2 = file_name.toLocal8Bit();
std:
  string tif_name = by2.constData();

  osgEarth::Config terrainConfig;
  // 初始化terrainConfig
  terrainConfig.key() = "elevation";
  terrainConfig.add("name", tif_name);
  terrainConfig.add("driver", "gdal");
  terrainConfig.add("url", m_elePath);
  terrainConfig.add("visible", true);

  // 初始化TileSourceOptions
  osgEarth::TileSourceOptions tileSourceOpt;
  tileSourceOpt.merge(terrainConfig);
  tileSourceOpt.setDriver("gdal");
  osg::ref_ptr<osgEarth::TileSource> pTileSource =
      osgEarth::TileSourceFactory::create(tileSourceOpt);

  osgEarth::ElevationLayerOptions layerOpt(terrainConfig);

  if (m_eleLayer != NULL) {
    m_mapNode->getMap()->removeElevationLayer(m_eleLayer);
  }

  m_eleLayer = new osgEarth::ElevationLayer(layerOpt, pTileSource);
  osg::ref_ptr<osgEarth::Map> map = m_mapNode->getMap();
  map->addLayer(m_eleLayer);

  flyToImageCenter(filePath);
  return m_eleLayer;
}

osg::Object* OSGEarthApp::onSlotShp(const QString& filePath) {
  osgEarth::Drivers::OGRFeatureOptions featureData;
  featureData.url() = filePath.toStdString();

  ifstream infile("earth/China.prj");
  string line;
  getline(infile, line);
  featureData.profile()->srsString() = line;

  osgEarth::Features::FeatureSourceLayerOptions ogrLayer;
  ogrLayer.name() = filePath.toStdString() + "_source";
  ogrLayer.featureSource() = featureData;
  osgEarth::Features::FeatureSourceLayer* featureSourceLayer =
      new osgEarth::Features::FeatureSourceLayer(ogrLayer);
  osg::ref_ptr<osgEarth::Map> map = m_mapNode->getMap();
  map->addLayer(featureSourceLayer);
  osgEarth::Features::FeatureSource* features =
      featureSourceLayer->getFeatureSource();
  if (!features) {
    printf("无法打开该矢量文件！");
    return nullptr;
  }

  // 设置样式
  osgEarth::Symbology::Style style;

  // 可见性
  osgEarth::Symbology::RenderSymbol* rs =
      style.getOrCreate<osgEarth::Symbology::RenderSymbol>();
  rs->depthTest() = false;

  // 贴地设置
  // osgEarth::Symbology::AltitudeSymbol* alt =
  // style.getOrCreate<osgEarth::Symbology::AltitudeSymbol>(); alt->clamping() =
  // alt->CLAMP_TO_TERRAIN; alt->technique() = alt->TECHNIQUE_DRAPE;

  // 设置矢量面样式（包括边界线）
  osgEarth::Symbology::LineSymbol* ls =
      style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
  ls->stroke()->color() = osgEarth::Symbology::Color("#FA8072");
  ls->stroke()->width() = 1.0;
  ls->tessellationSize()->set(100, osgEarth::Units::KILOMETERS);

  osgEarth::Symbology::PolygonSymbol* polygonSymbol =
      style.getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>();
  polygonSymbol->fill()->color() = osgEarth::Symbology::Color(
      152.0f / 255, 251.0f / 255, 152.0f / 255, 0.8f);  // 238 230 133
  polygonSymbol->outline() = true;

  //
  osgEarth::Features::FeatureModelLayerOptions fmlOpt;
  fmlOpt.name() = filePath.toStdString();
  fmlOpt.featureSourceLayer() = filePath.toStdString() + "_source";
  fmlOpt.enableLighting() = false;
  fmlOpt.styles() = new osgEarth::Symbology::StyleSheet();
  fmlOpt.styles()->addStyle(style);

  osg::ref_ptr<osgEarth::Features::FeatureModelLayer> fml =
      new osgEarth::Features::FeatureModelLayer(fmlOpt);
  map->addLayer(fml);

  return m_mapNode->getLayerNode(fml);
}

osg::Object* OSGEarthApp::onSlotLoadObj(const QString& filePath) {
  return nullptr;
}

void OSGEarthApp::onLoadScene() {
  // 1. 弹出文件对话框 (保持原样)
  QString filter = "All Supported Files (*.las *.tif *.img *.shp *.obj);;...";
  QString filePath = QFileDialog::getOpenFileName(
      this, QStringLiteral("选择要加载的文件"), m_lastOpenPath, filter);
  if (filePath.isEmpty()) return;

  // 防止重复加载
  if (m_pathNodeMap.contains(filePath)) {
    ui.lblText->setText(QStringLiteral("该文件已加载"));
    return;
  }

  // 更新最后一次打开的路径
  m_lastOpenPath = QFileInfo(filePath).absolutePath();
  QFileInfo fileInfo(filePath);
  QString suffix = fileInfo.suffix().toLower();

  // 2. 分流处理，并接收返回的 Node
  osg::ref_ptr<osg::Object> loadedNode = nullptr;

  if (suffix == "las") {
    loadedNode = onSlotLoadLas(filePath);  // 修改这些函数，让它们返回 node
  } else if (suffix == "tif") {
    loadedNode = onSlotTif(filePath);
  } else if (suffix == "shp") {
    loadedNode = onSlotShp(filePath);
  } else if (suffix == "obj") {
    loadedNode = onSlotLoadObj(filePath);
  }

  registerLoadedObject(filePath, loadedNode.get());
}

/*
void OSGEarthApp::onSlotShpPos() {}

 void OSGEarthApp::onSlotElePos() {
   // 定位
   // tif文件读取
   // GDALAllRegister();  //注册所有的驱动
   CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");  //
   以防中文名不能正常读取 GDALDataset* poDataset =
       (GDALDataset*)GDALOpen(m_elePath.c_str(), GA_ReadOnly);  // GDAL数据集
   if (poDataset == NULL) return;

   // 获取图像的尺寸
   int nImgSizeX = poDataset->GetRasterXSize();
   int nImgSizeY = poDataset->GetRasterYSize();

   // 获取坐标变换系数
   double trans[6];
   CPLErr err = poDataset->GetGeoTransform(trans);
   delete poDataset;

   // 中心点
   double centlon = trans[0] + trans[1] * nImgSizeX / 2.0;
   double centlat = trans[3] + trans[5] * nImgSizeY / 2.0;

   m_earthManipulator->setViewpoint(
       osgEarth::Viewpoint(u8"高程", centlon, centlat, 3000, 0.0, -90.0, 0.0),
       1.0);
 }

 void OSGEarthApp::onSlotTifPos() {
   //定位
   //tif文件读取
    GDALAllRegister(); //注册所有的驱动
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
   以防中文名不能正常读取
    GDALDataset* poDataset = (GDALDataset*)GDALOpen(m_tifPath.c_str(),
    GA_ReadOnly); //GDAL数据集 if (poDataset == NULL)
        return;

   //获取图像的尺寸
    int nImgSizeX = poDataset->GetRasterXSize();
    int nImgSizeY = poDataset->GetRasterYSize();

    double trans[6];
    CPLErr err = poDataset->GetGeoTransform(trans);

    const char* PrjRef = poDataset->GetProjectionRef(); //获取影像投影信息;
    OGRSpatialReference projSRS;

    delete poDataset;

   //中心点
    double centlon = trans[0] + trans[1] * nImgSizeX / 2.0;
    double centlat = trans[3] + trans[5] * nImgSizeY / 2.0;

    m_earthManipulator->setViewpoint(osgEarth::Viewpoint(u8"影像", centlon,
    centlat, 3000, 0.0, -90.0, 0.0), 1.0);
 }

 void OSGEarthApp::onSlotLasPos() {
   m_earthManipulator->setViewpoint(
       osgEarth::Viewpoint("las Model", m_lon, m_lat, 2000, 0.0, -90.0, 0.0),
       1.5);
 }
 */

/*-------------------辅助函数-------------------*/
void OSGEarthApp::flyToNode(osg::Node* node) {
  if (!node || !m_mapNode || !m_earthManipulator) return;

  // 1. 获取节点的包围球（包含中心点 center 和半径 radius）
  osg::BoundingSphere bs = node->getBound();

  // 如果包围球无效（比如节点还没完全加载），可以尝试强制更新
  if (!bs.valid()) {
    node->dirtyBound();
    bs = node->getBound();
  }

  if (bs.valid()) {
    // 2. 将世界坐标 (ECEF) 转换为地理坐标 (经纬度)
    osgEarth::GeoPoint geo;
    geo.fromWorld(m_geoSRS, bs.center());

    // 3. 计算观察范围 (Range)
    // 通常设为半径的 2 到 3 倍，这样能看清全貌又不会太远
    double range = bs.radius() * 2.5;
    if (range < 500.0) range = 500.0;  // 防止半径太小时镜头贴太近

    // 4. 创建视角对象
    // 参数依次为：名称、经度、纬度、高度(相对海拔)、航向角(0为北)、俯仰角(-90为垂直向下)、距离
    osgEarth::Viewpoint vp("Point Cloud Target", geo.x(), geo.y(), 2000, 0.0,
                           -90.0, range);
    m_lon = geo.x();
    m_lat = geo.y();

    // 5. 使用操作器执行平滑飞行（第二个参数是动画持续时间，单位：秒）
    m_earthManipulator->setViewpoint(vp, 2.0);
  }
}

void OSGEarthApp::onReShow() {
  this->show();
  if (m_frameTimer) {
    m_frameTimer->start(5);  // 重新开始渲染
  }
}

void OSGEarthApp::resetScene() {
  if (ui.leProjectName) ui.leProjectName->clear();

  if (m_dataGroup.valid()) {
    m_dataGroup->removeChildren(0, m_dataGroup->getNumChildren());
  }

  if (m_mapNode.valid()) {
    osgEarth::Map* map = m_mapNode->getMap();
    osgEarth::LayerVector layers;
    map->getLayers(layers);

    for (auto& layer : layers) {
      if (layer->getName() == "readymap_elevation" ||
          layer->getName() == "GlobeImage")
        continue;

      map->removeLayer(layer);
    }
  }
}

void OSGEarthApp::applyFullLayout(int w, int h) {
  // 1. 调整主窗口几何尺寸
  this->setGeometry(0, 0, w, h);

  // 2. 调整全局背景
  if (ui.lblBackground) {
    ui.lblBackground->setGeometry(0, 0, w, h);
    ui.lblBackground->lower();
  }

  // --- 核心布局参数 ---
  int rightPanelWidth = 320;  // 右侧面板总宽
  int btnW = 280;    // 内部组件宽度（稍微加宽点，更美观）
  int btnH = 35;     // 按钮高度
  int spacing = 15;  // 组件间的垂直间距
  int sideMargin = (rightPanelWidth - btnW) / 2;
  int panelX = w - rightPanelWidth;
  int compX = panelX + sideMargin;  // 组件起始 X
  int currentY = 50;                // 起始顶部间距

  // 3. 渲染区域 (左侧 OSG Widget)
  if (ui.widgetOSG) {
    int osgW = w - rightPanelWidth - 30;  // 留 30px 间隙
    int osgH = h - 40;
    ui.widgetOSG->setGeometry(15, 20, osgW, osgH);

    // 同步更新相机
    if (osgH > 0 && m_camera.valid()) {
      double aspect = static_cast<double>(osgW) / static_cast<double>(osgH);
      m_camera->setProjectionMatrixAsPerspective(45.0f, aspect, 1.0f, 10000.0f);
      m_camera->setViewport(0, 0, osgW, osgH);
      auto gw = dynamic_cast<osgQt::GraphicsWindowQt*>(
          m_camera->getGraphicsContext());
      if (gw) gw->getEventQueue()->windowResize(0, 0, osgW, osgH);
    }
  }

  // 4. 右侧面板背景
  if (ui.lblBtnbackground) {
    ui.lblBtnbackground->setGeometry(panelX, 0, rightPanelWidth, h);
  }

  // 5. [顶部] 项目名称输入框
  if (ui.leProjectName) {
    ui.leProjectName->setGeometry(compX, currentY, btnW, btnH);
    currentY += (btnH + spacing);
  }

  // 6. [顶部] 加载场景按钮
  if (ui.btnLoadScene) {
    ui.btnLoadScene->setGeometry(compX, currentY, btnW, btnH);
    currentY += (btnH + spacing);
  }

  // --- 计算底部区域高度，以便给 Tree 留出空间 ---
  int bottomAreaHeight = 120;  // 给 lblText 和 btnSavePro 预留的总高度
  int treeY = currentY;
  int treeH = h - treeY - bottomAreaHeight - 20;  // 20 为底部留白

  // 7. [中间核心] 文件树控件 (treeSceneManager)
  if (ui.treeSceneManager) {
    ui.treeSceneManager->setGeometry(compX, treeY, btnW, treeH);
  }

  // 8. [底部] 提示文本 (lblText)
  int lblTextY = treeY + treeH + 10;
  if (ui.lblText) {
    ui.lblText->setGeometry(compX, lblTextY, btnW, 40);
    ui.lblText->setAlignment(Qt::AlignCenter);
  }

  // 9. [底部] 保存按钮 (btnSavePro)
  if (ui.btnSavePro) {
    ui.btnSavePro->setGeometry(compX, h - 30 - btnH, btnW, btnH);
  }

  // 10. 右上角关闭按钮 (不变)
  if (ui.btn_Close) {
    ui.btn_Close->setGeometry(w - 45, 10, 35, 35);
    ui.btn_Close->raise();
  }
}

void OSGEarthApp::onSavePro() {
  // 1. 获取名称和时间
  QString pName = ui.leProjectName->text().trimmed();
  QString currentTime =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

  if (pName.isEmpty()) {
    pName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    ui.leProjectName->setText(pName);
  }

  QSqlDatabase db = QSqlDatabase::database();
  QSqlQuery query(db);

  // --- 核心逻辑：区分模式 ---
  bool isNewProject = true;

  // 如果当前已经是编辑模式，且输入框名字没改，说明是覆盖保存
  if (m_isEditMode && pName == m_currentData.projectName) {
    isNewProject = false;
  }

  // 2. 只有“新建”或者“改名”时才查重
  if (isNewProject) {
    query.prepare("SELECT p_name FROM t_project WHERE p_name = ?");
    query.addBindValue(pName);
    if (query.exec() && query.next()) {
      QMessageBox::warning(
          this, QStringLiteral("名称冲突"),
          QStringLiteral("项目 [%1] 已存在，请修改项目名称后再保存。")
              .arg(pName));
      return;
    }
  }

  // 3. 开始数据库事务
  if (!db.transaction()) return;

  if (isNewProject) {
    // A1. 新建项目：插入主表
    query.prepare(
        "INSERT INTO t_project (p_name, create_time, modify_time) VALUES (?, "
        "?, ?)");
    query.addBindValue(pName);
    query.addBindValue(currentTime);
    query.addBindValue(currentTime);
    if (!query.exec()) {
      cout << "Insert Main Error: " << query.lastError().text().toStdString()
           << endl;
      db.rollback();
      return;
    }
  } else {
    // A2. 编辑项目：仅更新主表的修改时间
    query.prepare("UPDATE t_project SET modify_time = ? WHERE p_name = ?");
    query.addBindValue(currentTime);
    query.addBindValue(pName);
    if (!query.exec()) {
      db.rollback();
      return;
    }

    // B1. 编辑模式下，先清空旧资产记录 (关键步骤)
    query.prepare("DELETE FROM t_project_assets WHERE p_name = ?");
    query.addBindValue(pName);
    if (!query.exec()) {
      db.rollback();
      return;
    }
  }

  // B2. 统一插入资源路径 (从映射表 m_pathNodeMap 获取最新的文件列表)
  query.prepare(
      "INSERT INTO t_project_assets (p_name, file_path) VALUES (?, ?)");

  // 注意：这里建议使用 m_pathNodeMap.keys()，因为这是当前场景中真实存在的文件
  QMapIterator<QString, osg::ref_ptr<osg::Object>> it(m_pathNodeMap);
  while (it.hasNext()) {
    it.next();
    query.addBindValue(pName);
    query.addBindValue(it.key());  // 文件绝对路径
    if (!query.exec()) {
      cout << "Insert Assets Error: " << query.lastError().text().toStdString()
           << endl;
      db.rollback();
      return;
    }
  }

  // 4. 提交
  if (db.commit()) {
    m_currentProjectName = pName;
    m_isEditMode = true;  // 保存后进入编辑状态
    emit projectSavedSuccess();
    QMessageBox::information(this, QStringLiteral("成功"),
                             QStringLiteral("项目保存成功！"));
  } else {
    db.rollback();
  }
}

void OSGEarthApp::onSlotClose() {
  this->hide();

  // 清空场景管理器资源
  resetSceneUI();

  // 清空渲染的模型
  resetScene();

  // 停止 OSG 的定时器，节省不显示的性能开销
  if (m_frameTimer) {
    m_frameTimer->stop();
  }
}

void OSGEarthApp::setEditMode(bool edit, QString pName) {
  m_isEditMode = edit;
  ui.leProjectName->setText(pName);

  if (m_isEditMode) {
    // 编辑模式：锁定输入框，不允许改名
    ui.leProjectName->setReadOnly(true);
    ui.lblText->setText(
        QStringLiteral("当前状态：正在编辑项目 [%1]").arg(pName));
  } else {
    // 新建模式：恢复可写
    ui.leProjectName->setReadOnly(false);
    ui.lblText->setText(QStringLiteral("支持模型格式：las,tif,obj,dem,shp"));
    ui.leProjectName->clear();
  }
}

void OSGEarthApp::initSceneManagerTree() {
  QStringList categories = {
      QStringLiteral("点云数据 (LAS)"), QStringLiteral("三维模型 (OBJ)"),
      QStringLiteral("影像/地形 (TIF)"), QStringLiteral("矢量数据 (SHP)")};

  ui.treeSceneManager->header()->setStretchLastSection(true);
  ui.treeSceneManager->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  ui.treeSceneManager->setIndentation(5);

  for (const QString& cat : categories) {
    QTreeWidgetItem* root = new QTreeWidgetItem(ui.treeSceneManager);
    root->setText(0, cat);
    root->setSizeHint(0, QSize(280, 35));
    root->setExpanded(true);  // 默认展开

    QFont font = root->font(0);
    font.setBold(true);
    root->setFont(0, font);

    // 用 key 记录方便查找
    if (cat.contains("LAS"))
      m_categoryNodes["las"] = root;
    else if (cat.contains("OBJ"))
      m_categoryNodes["obj"] = root;
    else if (cat.contains("TIF"))
      m_categoryNodes["tif"] = root;
    else if (cat.contains("SHP"))
      m_categoryNodes["shp"] = root;
  }
}

void OSGEarthApp::addFileToTree(QString filePath) {
  QFileInfo info(filePath);
  QString suffix = info.suffix().toLower();
  QString fileName = info.fileName();

  // 1. 找到对应的根节点 (假设你已在 initTree 中初始化了 m_categoryNodes)
  QTreeWidgetItem* parentNode = nullptr;
  if (suffix == "las")
    parentNode = m_categoryNodes["las"];
  else if (suffix == "obj")
    parentNode = m_categoryNodes["obj"];
  else if (suffix == "tif" || suffix == "img")
    parentNode = m_categoryNodes["tif"];
  else if (suffix == "shp")
    parentNode = m_categoryNodes["shp"];

  if (!parentNode) return;

  // 2. 创建树子节点
  QTreeWidgetItem* item = new QTreeWidgetItem(parentNode);

  // --- 关键步骤：设置尺寸 ---
  int itemW = ui.treeSceneManager->viewport()->width();
  int itemH = 40;
  item->setSizeHint(0, QSize(itemW, itemH));
  item->setData(0, Qt::UserRole, filePath);

  // 3. 创建自定义 Widget
  SceneItemWidget* widget =
      new SceneItemWidget(fileName, filePath, ui.treeSceneManager);

  connect(widget, &SceneItemWidget::signalLocate, this,
          &OSGEarthApp::onSlotLocateFile);

  // 连接删除信号 (需要 Lambda 捕获当前 item 指针)
  connect(widget, &SceneItemWidget::signalDelete, this,
          &OSGEarthApp::onSlotDeleteFile);

  // 设置 Widget 的固定高度，防止被 Tree 压缩
  widget->setFixedHeight(itemH);
  widget->setFixedWidth(itemW);

  // 4. 将 Widget 插入树节点
  ui.treeSceneManager->setItemWidget(item, 0, widget);
  m_pathItemMap[filePath] = item;

  // 5. 确保父节点展开，否则看不见子项
  parentNode->setExpanded(true);
}

void OSGEarthApp::onSlotLocateFile(QString path) {
  if (!m_pathNodeMap.contains(path)) return;

  osg::Object* obj = m_pathNodeMap[path].get();

  // ---------- 情况 1：影像图层 ----------
  if (auto* layer = dynamic_cast<osgEarth::ImageLayer*>(obj)) {
    flyToImageCenter(path);
  }
  // ---------- 情况 2：模型 / 点云 ----------
  auto* node = dynamic_cast<osg::Node*>(obj);
  flyToNode(node);
}

void OSGEarthApp::flyToImageCenter(const QString& filePath) {
  // 1. 编码转换：处理中文路径的关键
  QByteArray localPath = filePath.toLocal8Bit();
  const char* c_path = localPath.constData();

  // 2. 确保驱动已注册
  GDALAllRegister();
  CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

  // 3. 打开数据集
  GDALDataset* poDataset = (GDALDataset*)GDALOpen(c_path, GA_ReadOnly);
  if (!poDataset) {
    cout << "Failed to open image for flight: " << c_path << endl;
    return;
  }

  // 4. 读取元数据并计算中心点
  int nImgSizeX = poDataset->GetRasterXSize();
  int nImgSizeY = poDataset->GetRasterYSize();
  double trans[6];

  if (poDataset->GetGeoTransform(trans) == CE_None) {
    // 计算影像中心坐标 (地理坐标或投影坐标)
    double centlon = trans[0] + trans[1] * nImgSizeX / 2.0;
    double centlat = trans[3] + trans[5] * nImgSizeY / 2.0;

    //(名称, 经度, 纬度, 海拔, 偏航, 俯仰, 距离)
    m_earthManipulator->setViewpoint(
        osgEarth::Viewpoint(u8"影像定位", centlon, centlat, 0.0, 0.0, -90.0,
                            1000.0),
        1.0);
  }

  // 6. 关闭数据集
  GDALClose(poDataset);
}

void OSGEarthApp::registerLoadedObject(const QString& filePath,
                                       osg::Object* obj) {
  if (!obj) {
    ui.lblText->setText(QStringLiteral("加载失败，请检查文件格式"));
    return;
  }

  // 1. 建立映射
  m_pathNodeMap[filePath] = obj;

  // 2. 添加到 UI 树 (创建 SceneItemWidget)
  addFileToTree(filePath);

  // 3. 记录到当前项目的文件列表（防止重复）
  if (!m_currentData.allFiles.contains(filePath)) {
    m_currentData.allFiles.append(filePath);
  }

  ui.lblText->setText(QStringLiteral("加载成功: %1")
                          .arg(QFileInfo(filePath).suffix().toLower()));
}

void OSGEarthApp::onSlotDeleteFile(QString path) {
  // 1. 检查文件是否在映射表中
  if (!m_pathNodeMap.contains(path)) return;

  // 获取对应的 OSG 对象
  osg::ref_ptr<osg::Object> obj = m_pathNodeMap[path];

  // 情况 A: 影像图层 (TIF/IMG)
  if (auto* layer = dynamic_cast<osgEarth::ImageLayer*>(obj.get())) {
    if (m_mapNode.valid() && m_mapNode->getMap()) {
      m_mapNode->getMap()->removeImageLayer(layer);
    }
  }
  // 情况 B: 矢量图层 (SHP)
  else if (auto* modelLayer = dynamic_cast<osgEarth::ModelLayer*>(obj.get())) {
    if (m_mapNode.valid() && m_mapNode->getMap()) {
      m_mapNode->getMap()->removeModelLayer(modelLayer);
    }
  }
  // 情况 C: 普通模型节点 (OBJ/LAS)
  else if (auto* node = dynamic_cast<osg::Node*>(obj.get())) {
    if (m_root.valid()) {
      m_dataGroup->removeChild(node);
    }
  }

  // 3. 从映射表中删除（这一步会减少 osg::ref_ptr 的引用计数，真正释放内存）
  m_pathNodeMap.remove(path);

  // 4. 从 QTreeWidget 中删除对应的项
  if (m_pathItemMap.contains(path)) {
    QTreeWidgetItem* item = m_pathItemMap.take(path);
    if (item && item->parent()) {
      item->parent()->removeChild(item);
      delete item;
    }
  }

  // 5. 维护项目变量 (如果有记录当前项目所有文件的列表)
  m_currentData.allFiles.removeAll(path);

  // 6. 状态提示
  ui.lblText->setText(
      QStringLiteral("已移除资源：%1").arg(QFileInfo(path).fileName()));

  // 7. 强制刷新一次界面（可选）
  // m_viewer->frame();
}

void OSGEarthApp::resetSceneUI() {
  m_currentData.clear();
  m_pathNodeMap.clear();
  m_pathItemMap.clear();

  for (int i = 0; i < ui.treeSceneManager->topLevelItemCount(); ++i) {
    QTreeWidgetItem* root = ui.treeSceneManager->topLevelItem(i);
    qDeleteAll(root->takeChildren());
  }
}