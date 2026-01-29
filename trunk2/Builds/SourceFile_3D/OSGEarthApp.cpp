#include "OSGEarthApp.h"

#include "SceneItemWidget.h"

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
  initSceneManagerTree();  // 5.初始化场景管理树
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

  m_noticeToast = new NoticeToast(this);
}

void OSGEarthApp::startAsyncLoad() {
  // 使用 QPointer 监控 this 的生命周期
  QPointer<OSGEarthApp> weakThis = this;

  QtConcurrent::run([weakThis]() {
    // 1. 线程开始，先尝试加载
    osg::ref_ptr<osg::Node> loadedNode = nullptr;
    try {
      loadedNode = osgDB::readNodeFile("./earth/mymap0.earth");
    } catch (...) {
      return;
    }
    if (weakThis.isNull() || !loadedNode.valid()) return;

    // 3. 回到主线程执行挂载
    QMetaObject::invokeMethod(
        weakThis.data(),
        [weakThis, loadedNode]() {
          OSGEarthApp* self = weakThis.data();
          self->m_earthNode = loadedNode;
          self->initMembers();
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
  m_camera->setNearFarRatio(0.00001);
  m_camera->setProjectionMatrixAsPerspective(
      30.0f,
      static_cast<double>(traits->width) / static_cast<double>(traits->height),
      0.1f, 10000.0f);
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
          [this]() { this->onLoadScene(); });
  connect(ui.btn_Close, &QPushButton::clicked, this, &OSGEarthApp::onSlotClose);
  connect(ui.btnSavePro, &QPushButton::clicked, this, &OSGEarthApp::onSavePro);
  connect(ui.btnMeasure, &QPushButton::clicked, this, &OSGEarthApp::onMeasure);
  
  connect(ui.btnClipMode, &QPushButton::toggled, this, [this](bool checked) {
    ui.clipSubToolbar->setVisible(checked);

    // 2. 切换模式
    if (checked) {
      m_interManager->setMode(InterMode::CLIP);
    } else {
      m_interManager->setMode(InterMode::VIEW);
    }
  });

  // 渲染定时器
  m_frameTimer = new QTimer(this);
  connect(m_frameTimer, &QTimer::timeout, this, [this]() {
    if (m_viewer.valid()) {
      updateCameraSensitivity(m_earthManipulator.get());
      m_viewer->frame();
    }
  });
  m_frameTimer->start(5);
}

/*-------------------加载场景-------------------*/

void OSGEarthApp::onLoadScene(QString filePath, double lon, double lat,
                              double alt) {
  // 如果是添加新文件，则弹出选择对话框
  if (filePath.isEmpty()) {
    QString filter = "All Supported Files (*.las *.tif *.img *.shp *.obj);;...";
    filePath = QFileDialog::getOpenFileName(
        this, QStringLiteral("选择要加载的文件"), m_lastOpenPath, filter);
    if (filePath.isEmpty()) return;
  }

  // 防止重复加载
  if (m_pathNodeMap.contains(filePath)) {
    ui.lblText->setText(QStringLiteral("该文件已加载"));
    return;
  }

  // 更新最后一次打开的路径
  m_lastOpenPath = QFileInfo(filePath).absolutePath();
  osg::ref_ptr<osg::Object> loadedNode =
      m_assetLoader->load(filePath, lon, lat, alt);
  if (loadedNode.valid()) {
    osg::Node* node = dynamic_cast<osg::Node*>(loadedNode.get());
    if (node) {
      m_dataGroup->addChild(node);
      this->flyToNode(node);  // 定位
    } else if (osgEarth::Layer* layer =
                   dynamic_cast<osgEarth::Layer*>(loadedNode.get())) {
      this->flyToLayer(layer);
    }
  }

  registerLoadedObject(filePath, loadedNode.get());
}

void OSGEarthApp::onSavePro() {
  // 1. 获取名称和时间
  QString pName = ui.leProjectName->text().trimmed();
  // QString currentTime =
  //     QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

  if (pName.isEmpty()) {
    pName = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    ui.leProjectName->setText(pName);
  }

  QSqlDatabase db = QSqlDatabase::database();
  QSqlQuery query(db);

  // 判断是不是更新现有的项目
  bool isUpdate = (m_currentMode == WorkMode::EditMode &&
                   pName == m_currentData.projectName);

  // 2. 只有“新建”或者“改名”时才查重
  if (!isUpdate && ProjectDataManager::isProjectExists(pName)) {
    m_noticeToast->popup(this, u8"项目名称已存在，请更换后再试。", 2000);
    return;
  }

  QList<AssetRecord> assetList;
  QMapIterator<QString, osg::ref_ptr<osg::Object>> it(m_pathNodeMap);
  while (it.hasNext()) {
    it.next();
    AssetRecord rec;
    rec.filePath = it.key();
    if (auto xform = dynamic_cast<osgEarth::GeoTransform*>(it.value().get())) {
      osgEarth::GeoPoint pos = xform->getPosition();
      rec.lon = pos.x();
      rec.lat = pos.y();
      rec.alt = pos.z();
    }
    assetList.append(rec);
  }

  // 3. 一行调用保存
  if (ProjectDataManager::saveProject(pName, assetList, isUpdate)) {
    m_currentData.projectName = pName;
    emit projectSavedSuccess();
    m_noticeToast->popup(this, u8"项目保存成功", 2000);
  }
}

void OSGEarthApp::onSlotClose() {
  this->hide();

  resetScene();

  if (m_frameTimer) {
    m_frameTimer->stop();
  }
}

void OSGEarthApp::onMeasure(bool checked) {
  if (!m_interManager) return;

  if (checked) {
    // --- 进入测量模式 ---
    m_interManager->clearAll();
    m_interManager->setMode(InterMode::MEASURE);
    ui.btnMeasure->setProperty("active", true);
  } else {
    // --- 退出测量模式 ---
    m_interManager->setMode(InterMode::VIEW);
    m_interManager->clearAll();
    ui.btnMeasure->setProperty("active", false);
  }

  ui.btnMeasure->style()->unpolish(ui.btnMeasure);
  ui.btnMeasure->style()->polish(ui.btnMeasure);
}

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

    // 5. 使用操作器执行平滑飞行（第二个参数是动画持续时间，单位：秒）
    m_earthManipulator->setViewpoint(vp, 2.0);
  }
}

void OSGEarthApp::flyToLayer(osgEarth::Layer* layer) {
  if (!layer || !m_earthManipulator) return;

  // 1. 获取图层的空间范围
  osgEarth::DataExtent extent = layer->getExtent();

  if (extent.isValid()) {
    // 2. 手动计算中心点 (经纬度)
    double centerLon = (extent.xMin() + extent.xMax()) / 2.0;
    double centerLat = (extent.yMin() + extent.yMax()) / 2.0;

    // 3. 计算跨度，用于自动调整相机高度 (Range)
    double width = extent.width();    // 经度跨度
    double height = extent.height();  // 纬度跨度
    double dimension = std::max(width, height);

    // 粗略估算相机高度系数
    // 1度跨度大约对应 100km-150km 的观测高度比较合适
    double range = dimension * 120000.0 * 1.5;

    if (range < 2000.0) range = 2000.0;

    // 4. 执行定位
    osgEarth::Viewpoint vp(u8"影像layer", centerLon, centerLat,
                           0.0,         // 海拔偏移
                           0.0, -90.0,  // 航向角, 俯仰角 (垂直向下看)
                           range        // 距离地表的距离
    );

    m_earthManipulator->setViewpoint(vp, 2.0);  // 2秒平滑飞行
  } else {
    // 如果图层范围还没准备好，可以回退调用 flyToImageCenter(path)
    return;
  }
}

void OSGEarthApp::onReShow() {
  this->show();
  if (m_frameTimer) {
    m_frameTimer->start(5);  // 重新开始渲染
  }
}

void OSGEarthApp::resetScene() {
  // 清空成员变量
  m_currentData.clear();
  m_pathNodeMap.clear();
  m_pathItemMap.clear();

  if (ui.leProjectName) ui.leProjectName->clear();

  // 清楚数据节点
  if (m_dataGroup.valid()) {
    m_dataGroup->removeChildren(0, m_dataGroup->getNumChildren());
  }

  // 清除layer
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

  // 清楚场景管理树控件种的item
  for (int i = 0; i < ui.treeSceneManager->topLevelItemCount(); ++i) {
    QTreeWidgetItem* root = ui.treeSceneManager->topLevelItem(i);
    qDeleteAll(root->takeChildren());
  }
}

void OSGEarthApp::applyFullLayout(int w, int h) {
  // 1. 调整主窗口几何尺寸
  this->setGeometry(0, 0, w, h);

  adjustUI(&ui, w, h);
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

  // 连接信号
  connect(widget, &SceneItemWidget::signalLocate, this,
          &OSGEarthApp::onSlotLocateFile);
  connect(widget, &SceneItemWidget::signalDelete, this,
          &OSGEarthApp::onSlotDeleteFile);
  // connect(widget, &SceneItemWidget::signalSimulate, this,
  //         &OSGEarthApp::enterTowerEditMode);

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

  // 1. 影像或高程图层
  if (auto* layer = dynamic_cast<osgEarth::Layer*>(obj)) {
    flyToLayer(layer);
  }
  // 3. 模型/点云
  else if (auto* node = dynamic_cast<osg::Node*>(obj)) {
    flyToNode(node);
  }
}

void OSGEarthApp::onSlotDeleteFile(QString path) {
  // 1. 检查文件是否在映射表中
  if (!m_pathNodeMap.contains(path)) return;

  // 获取对应的 OSG 对象
  osg::ref_ptr<osg::Object> obj = m_pathNodeMap[path];

  // 情况 A/B/D: osgEarth 图层 (影像 Image, 矢量 Model, 高程 Elevation)
  // 这些都继承自 osgEarth::Layer
  if (auto* layer = dynamic_cast<osgEarth::Layer*>(obj.get())) {
    if (m_mapNode.valid() && m_mapNode->getMap()) {
      // 现代 API：removeLayer 会自动判断它是哪种层并安全移除
      m_mapNode->getMap()->removeLayer(layer);
    }
  }
  // 情况 C: 普通模型节点 (OBJ/LAS)
  else if (auto* node = dynamic_cast<osg::Node*>(obj.get())) {
    if (m_dataGroup.valid()) {
      m_dataGroup->removeChild(node);
    }
  }

  // 3. 从映射表中删除
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

void OSGEarthApp::setWorkMode(WorkMode mode, QString pName) {
  m_currentMode = mode;
  m_currentData.projectName = pName;
}

void OSGEarthApp::updateUIByWorkMode() {
  // 1. 处理UI
  ui.leProjectName->setText(m_currentData.projectName);
  ui.leProjectName->setReadOnly(m_currentMode == WorkMode::BrowseMode);
  ui.btnSavePro->setVisible(m_currentMode != WorkMode::BrowseMode);
  ui.btnLoadScene->setVisible(m_currentMode != WorkMode::BrowseMode);

  // 2. 刷新 Tree 列表里的所有删除按钮
  bool canDelete = (m_currentMode != WorkMode::BrowseMode);

  // 直接遍历我们维护的 Map
  QMapIterator<QString, QTreeWidgetItem*> it(m_pathItemMap);
  while (it.hasNext()) {
    it.next();
    QTreeWidgetItem* item = it.value();
    if (item) {
      // 获取对应的自定义 Widget
      SceneItemWidget* w = qobject_cast<SceneItemWidget*>(
          ui.treeSceneManager->itemWidget(item, 0));
      if (w) {
        w->setDeleteButtonVisible(canDelete);
      }
    }
  }

  // 3. 处理特殊的样式提示
  switch (m_currentMode) {
    case WorkMode::NewMode:
      ui.lblText->setText(QStringLiteral("模式：新建项目"));
      ui.leProjectName->setPlaceholderText(
          QStringLiteral("请输入新项目名称..."));
      ui.leProjectName->setStyleSheet(
          "QLineEdit { border: 2px solid #3498db; }");
      break;
    case WorkMode::EditMode:
      ui.lblText->setText(
          QStringLiteral("模式：编辑项目 [%1]").arg(m_currentData.projectName));
      ui.leProjectName->setStyleSheet("QLineEdit { background: #f0f0f0; }");
      break;
    case WorkMode::BrowseMode:
      ui.lblText->setText(QStringLiteral("模式：浏览项目 [%1] (受保护)")
                              .arg(m_currentData.projectName));
      ui.leProjectName->setStyleSheet(
          "QLineEdit { background: #eee; color: #888; }");
      break;
  }
}

void OSGEarthApp::initMembers() {
  // 由于是异步加载，所以此函数不可以直接在构造函数调用
  m_earthNode->setName("earth");
  m_earthNode->setNodeMask(MASK_TERRAIN);
  m_root->addChild(m_earthNode.get());

  m_mapNode = osgEarth::MapNode::findMapNode(m_earthNode.get());
  m_assetLoader = new AssetLoader(m_mapNode, this);
  m_geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();

  m_interManager = new InteractionManager(m_viewer.get(), m_dataGroup, this);
  m_viewer->addEventHandler(m_interManager);

  m_dataGroup->getOrCreateStateSet()->setMode(
      GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  m_earth_init = true;
}

/*
 void OSGEarthApp::startCollapseAnimation() {
   QVariantAnimation* anim = new QVariantAnimation(this);
   anim->setDuration(10000);  // 10秒倒塌
   anim->setStartValue(0.0f);
   anim->setEndValue(90.0f);

   connect(anim, &QVariantAnimation::valueChanged,
           [this](const QVariant& value) {
             if (m_currentTower) {
               // 假设用户选择了 RIGHT 方向
               m_currentTower->updateFall(Controller::RIGHT, value.toFloat());
             }
           });
   anim->start();
 }

 */

/*
 void OSGEarthApp::enterTowerEditMode(const QString& path) {
  startCollapseAnimation();
   1. 获取对应的物体
   osg::Object* obj = m_pathNodeMap[path].get();

// 注意：我们要操作的是 GeoTransform 里面的那个动画/校准节点
// 这样不会破坏地理坐标，只是在局部做微调
 osgEarth::GeoTransform* xform = dynamic_cast<osgEarth::GeoTransform*>(obj);
 if (!xform || xform->getNumChildren() == 0) return;

// 假设第一个孩子是我们的 TowerController 里的 fallMT
 osg::MatrixTransform* towerRoot =
     dynamic_cast<osg::MatrixTransform*>(xform->getChild(0));

 if (towerRoot) {
   if (!m_towerManip) {
     m_towerManip = new ManipulatorHelper(m_root.get());
   }
   m_towerManip->attach(towerRoot);
 }

// 2. UI 切换
//ui.simulateControlPanel->show();
 qDebug() << "Tower Edit Mode Active for: " << path;
}
*/

/*
 void OSGEarthApp::onCoordinateChanged(double lon, double lat, double alt,
                                       bool isOut) {
   // 容错处理
   if (!ui.lblCoordinate) return;

   if (isOut) {
     ui.lblCoordinate->setText(u8"位置: 太空");
   } else {
     // 格式化经纬度信息
     QString info = QString(u8"经度: %1 | 纬度: %2 | 海拔: %3 m")
                        .arg(lon, 0, 'f', 2)
                        .arg(lat, 0, 'f', 2)
                        .arg(alt, 0, 'f', 2);

     ui.lblCoordinate->setText(info);
   }
 }
 */