#include "OSGEarthApp.h"

#include "utils.h"

OSGEarthApp::OSGEarthApp(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
  setAttribute(Qt::WA_QuitOnClose, true);
  setAttribute(Qt::WA_DeleteOnClose);
  setWindowTitle("osgEarth Eigen APP");

  initEnvironment();    // 1. 环境准备
  initOSGViewer();      // 2. 配置相机和渲染窗口
  initUIConnections();  // 3. 绑定按钮逻辑
  startAsyncLoad();     // 4. 异步加载地球模型
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
            qDebug() << "Earth Node Loaded in Main Thread";
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

/*-------------------加载场景-------------------*/

void OSGEarthApp::onSlotLoadLas(const QString& filePath) {
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
      return;
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

void OSGEarthApp::onSlotTif(const QString& filePath) {
  QByteArray by = filePath.toLocal8Bit();
  m_tifPath = by.constData();
  QFileInfo fileinfo = QFileInfo(filePath);
  // 文件名
  QString file_name = fileinfo.fileName();
  QByteArray by2 = file_name.toLocal8Bit();
std:
  string tif_name = by2.constData();

  osgEarth::Config terrainConfig;
  // 初始化terrainConfig
  terrainConfig.key() = "image";
  terrainConfig.add("name", tif_name);
  terrainConfig.add("driver", "gdal");
  terrainConfig.add("url", m_tifPath);
  terrainConfig.add("hastransparency", true);
  terrainConfig.add("transparent", true);
  terrainConfig.add("visible", true);
  terrainConfig.add("format", "png");
  terrainConfig.add("transparent_color", "0 0 0 0");
  // 初始化TileSourceOptions
  osgEarth::TileSourceOptions tileSourceOpt;
  tileSourceOpt.merge(terrainConfig);
  tileSourceOpt.setDriver("gdal");
  osg::ref_ptr<osgEarth::TileSource> pTileSource =
      osgEarth::TileSourceFactory::create(tileSourceOpt);

  osgEarth::ImageLayerOptions layerOpt(terrainConfig);

  if (m_layer != NULL) {
    m_mapNode->getMap()->removeImageLayer(m_layer);
  }
  m_layer = new osgEarth::ImageLayer(layerOpt, pTileSource);
  osg::ref_ptr<osgEarth::Map> map = m_mapNode->getMap();
  map->addImageLayer(m_layer);

  // 定位
  // tif文件读取
  GDALAllRegister();                                  // 注册所有的驱动
  CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");  // 以防中文名不能正常读取
  GDALDataset* poDataset =
      (GDALDataset*)GDALOpen(m_tifPath.c_str(), GA_ReadOnly);  // GDAL数据集
  if (poDataset == NULL) return;
  // 获取图像的尺寸
  int nImgSizeX = poDataset->GetRasterXSize();
  int nImgSizeY = poDataset->GetRasterYSize();
  double trans[6];
  CPLErr err = poDataset->GetGeoTransform(trans);
  delete poDataset;
  // 中心点
  double centlon = trans[0] + trans[1] * nImgSizeX / 2.0;
  double centlat = trans[3] + trans[5] * nImgSizeY / 2.0;

  m_earthManipulator->setViewpoint(
      osgEarth::Viewpoint(u8"影像", centlon, centlat, 3000, 0.0, -90.0, 0.0),
      1.0);
}

void OSGEarthApp::onSlotElevation(const QString& filePath) {
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

  // 定位
  // 获取坐标变换系数
  // tif文件读取
  GDALAllRegister();                                  // 注册所有的驱动
  CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");  // 以防中文名不能正常读取
  GDALDataset* poDataset =
      (GDALDataset*)GDALOpen(m_elePath.c_str(), GA_ReadOnly);  // GDAL数据集
  if (poDataset == NULL) return;
  // 获取图像的尺寸
  int nImgSizeX = poDataset->GetRasterXSize();
  int nImgSizeY = poDataset->GetRasterYSize();
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

void OSGEarthApp::onSlotShp(const QString& filePath) {
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
    return;
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
}

void OSGEarthApp::onSlotLoadObj(const QString& filePath) {}

void OSGEarthApp::onLoadScene() {
  // 1. 弹出文件对话框，支持多种格式过滤
  QString filter =
      "All Supported Files (*.las *.tif *.img *.shp *.obj);;"
      "Point Cloud (*.las);;"
      "Imagery/Elevation (*.tif);;"
      "Vector (*.shp);;"
      "Model (*.obj)";

  QString filePath = QFileDialog::getOpenFileName(
      this, QStringLiteral("选择要加载的文件"),
      m_lastOpenPath,  // 记录上次打开的路径，提升体验
      filter);

  if (filePath.isEmpty()) return;

  // 更新最后一次打开的路径
  m_lastOpenPath = QFileInfo(filePath).absolutePath();

  // 2. 获取后缀名并判断类型
  QFileInfo fileInfo(filePath);
  QString suffix = fileInfo.suffix().toLower();

  // 3. 分流处理
  if (suffix == "las") {
    onSlotLoadLas(filePath);
  } else if (suffix == "tif") {
    onSlotTif(filePath);
    //} else {
    onSlotElevation(filePath);
    //}
  } else if (suffix == "shp") {
    onSlotShp(filePath);
  } else if (suffix == "obj") {
    onSlotLoadObj(filePath);
  }

  // 4. 重要：将加载成功的文件路径加入待保存列表
  if (!m_currentData.allFiles.contains(filePath)) {
    m_currentData.allFiles.append(filePath);
    cout << "Added to project list: " << filePath.toLocal8Bit().constData()
         << endl;
  }
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

void OSGEarthApp::onReShow() {
  this->show();
  if (m_frameTimer) {
    m_frameTimer->start(5);  // 重新开始渲染
  }
}

void OSGEarthApp::resetForNewProject() {
  m_currentData.clear();
  if (ui.leProjectName) ui.leProjectName->clear();

  if (m_dataGroup.valid()) {
    m_dataGroup->removeChildren(0, m_dataGroup->getNumChildren());
  }

  if (m_mapNode.valid()) {
    osgEarth::Map* map = m_mapNode->getMap();
    osgEarth::LayerVector layers;
    map->getLayers(layers);

    for (auto& layer : layers) {
      // cout << layer->getName() << endl;
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

  // 2. 调整全局背景（如有）
  if (ui.lblBackground) {
    ui.lblBackground->setGeometry(0, 0, w, h);
    ui.lblBackground->lower();
  }

  // --- 核心参数定义 ---
  int rightPanelWidth = 320;  // 右侧面板宽度（略微加宽以适应输入框）
  int btnW = 260;             // 按钮和输入框宽度
  int btnH = 35;              // 按钮和输入框高度
  int sideMargin = (rightPanelWidth - btnW) / 2;  // 侧向居中间距
  int btnX = w - rightPanelWidth + sideMargin;    // 右侧组件的 X 坐标
  int currentY = 100;                             // 起始 Y 坐标
  int spacing = 15;                               // 组件垂直间距

  // 3. 渲染区域 (左侧 OSG Widget)
  // 预留右侧空间给面板，四周留 20px 边距
  if (ui.widgetOSG) {
    int osgW = w - rightPanelWidth - 40;
    int osgH = h - 40;
    ui.widgetOSG->setGeometry(20, 20, osgW, osgH);

    // 同步更新 OSG 相机参数（防止拉伸）
    if (osgH > 0 && m_camera.valid()) {
      double aspect = static_cast<double>(osgW) / static_cast<double>(osgH);
      m_camera->setProjectionMatrixAsPerspective(45.0f, aspect, 1.0f, 10000.0f);
      m_camera->setViewport(0, 0, osgW, osgH);

      auto gw = dynamic_cast<osgQt::GraphicsWindowQt*>(
          m_camera->getGraphicsContext());
      if (gw) gw->getEventQueue()->windowResize(0, 0, osgW, osgH);
    }
  }

  // 4. 右侧面板背景 Label
  if (ui.lblBtnbackground) {
    ui.lblBtnbackground->setGeometry(w - rightPanelWidth, 0, rightPanelWidth,
                                     h);
  }

  // 5. 布局：项目名称输入框 (leProjectName)
  if (ui.leProjectName) {
    ui.leProjectName->setGeometry(btnX, currentY, btnW, btnH);
    currentY += (btnH + spacing);
  }

  // 6. 布局：加载场景按钮 (btnLoadScene)
  if (ui.btnLoadScene) {
    ui.btnLoadScene->setGeometry(btnX, currentY, btnW, btnH);
    // 这里可以给 btnLoadScene 设置样式
  }

  // 7. 布局：提示文本 (lblText) - 位于中间或特定间距
  currentY += 60;  // 额外空出一段距离
  if (ui.lblText) {
    ui.lblText->setGeometry(btnX, currentY, btnW,
                            60);  // 高度可以略大，支持多行
  }

  // 8. 布局：保存按钮 (btnSavePro) - 靠底部
  if (ui.btnSavePro) {
    // 固定在距离底部 60px 的位置
    ui.btnSavePro->setGeometry(btnX, h - 60 - btnH, btnW, btnH);
  }

  // 9. 右上角关闭按钮 (btn_Close)
  if (ui.btn_Close) {
    ui.btn_Close->setGeometry(w - 40 - 10, 10, 40, 40);
    ui.btn_Close->raise();  // 确保在最顶层
    QString iconPath = QString("%1/Resource/common/close.png")
                           .arg(QApplication::applicationDirPath());
    ui.btn_Close->setStyleSheet(
        QString("QPushButton{border-image:url(%1); border:none;}")
            .arg(iconPath));
  }
}

void OSGEarthApp::onSavePro() {
  // 1. 获取名称
  QString pName = ui.leProjectName->text().trimmed();
  QString currentTime =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

  // 如果用户没输入，以时间作为名称
  if (pName.isEmpty()) {
    pName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    ui.leProjectName->setText(pName);
  }

  QSqlDatabase db = QSqlDatabase::database();
  QSqlQuery query(db);

  // 2. 查重逻辑
  query.prepare("SELECT p_name FROM t_project WHERE p_name = ?");
  query.addBindValue(pName);

  if (query.exec() && query.next()) {
    // 发现同名，打印到终端并弹窗
    cout << "Save Failed: Project name [" << pName.toLocal8Bit().constData()
         << "] already exists." << endl;
    QMessageBox::warning(
        this, QStringLiteral("名称冲突"),
        QStringLiteral("项目 [%1] 已存在，请修改项目名称后再保存。")
            .arg(pName));
    return;
  }

  // 3. 开始保存
  if (!db.transaction()) {
    cout << "Database Transaction Error!" << endl;
    return;
  }

  // A. 插入项目主表
  query.prepare(
      "INSERT INTO t_project (p_name, create_time, modify_time) VALUES (?, ?, "
      "?)");
  query.addBindValue(pName);
  query.addBindValue(currentTime);
  query.addBindValue(currentTime);

  if (!query.exec()) {
    // 使用 lastError().text() 打印具体错误到 cout
    cout << "Insert Project Error: "
         << query.lastError().text().toLocal8Bit().constData() << endl;
    db.rollback();
    return;
  }

  // B. 插入资源路径表
  query.prepare(
      "INSERT INTO t_project_assets (p_name, file_path) VALUES (?, ?)");
  for (const QString& path : m_currentData.allFiles) {
    query.addBindValue(pName);
    query.addBindValue(path);
    if (!query.exec()) {
      cout << "Insert Assets Error: "
           << query.lastError().text().toLocal8Bit().constData() << endl;
      db.rollback();
      return;
    }
  }

  // 4. 提交
  if (db.commit()) {
    m_currentData.projectName = pName;
    emit projectSavedSuccess();
    QMessageBox::information(this, QStringLiteral("成功"),
                             QStringLiteral("项目保存成功！"));
  } else {
    cout << "Commit failed!" << endl;
    db.rollback();
  }
}

void OSGEarthApp::onSlotClose() {
  this->hide();

  // 建议同时停止 OSG 的定时器，节省不显示的性能开销
  if (m_frameTimer) {
    m_frameTimer->stop();
  }
}