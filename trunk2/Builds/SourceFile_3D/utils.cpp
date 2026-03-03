#include "utils.h"

#include <future>
#include <qpainter.h>

// 记录单个瓦片处理后的结果
struct TileTaskResult {
  int index;
  std::string fileName;
  osg::BoundingBox bb;
};

std::string _Chinesepath;

// 批量将 Vec3Array 从 EPSG:2383 转换为 osgEarth 世界坐标(ECEF)
static void ConvertPointCloudToECEF(osg::ref_ptr<osg::Vec3Array>& vertices,
                                    const osgEarth::SpatialReference* geoSRS) {
  if (!vertices.valid() || vertices->empty() || !geoSRS) return;

  // 创建坐标系转换器（只创建一次）
  OGRSpatialReference srcSRS, dstSRS;

  if (srcSRS.importFromEPSG(32650) != OGRERR_NONE) {
    std::cout << "srcSRS init failed!" << std::endl;
    return;
  }

  if (dstSRS.importFromEPSG(4326) != OGRERR_NONE) {
    std::cout << "dstSRS init failed!" << std::endl;
    return;
  }
  // 使用 std::unique_ptr 配合自定义析构器防止内存泄漏
  OGRCoordinateTransformation* poCT =
      OGRCreateCoordinateTransformation(&srcSRS, &dstSRS);

  if (!poCT) {
    return;
  }

  for (size_t i = 0; i < vertices->size(); ++i) {
    double x = (*vertices)[i].x();
    double y = (*vertices)[i].y();
    double z = (*vertices)[i].z();

    if (poCT->Transform(1, &x, &y, &z)) {
      osgEarth::GeoPoint geo(geoSRS, x, y, z, osgEarth::ALTMODE_ABSOLUTE);
      osg::Vec3d ecef;
      geo.toWorld(ecef);
      (*vertices)[i].set(ecef.x(), ecef.y(), ecef.z());
    }
  }

  OCTDestroyCoordinateTransformation(poCT);
}

static osg::Geode* createSparseGeode(osg::ref_ptr<osg::Vec3Array> vertices,
                                     osg::ref_ptr<osg::Vec4Array> colors,
                                     osg::ref_ptr<osg::Vec4Array> True_colors,
                                     float point_size, int sampleStep) {
  osg::ref_ptr<osg::Vec3Array> sparseVerts = new osg::Vec3Array();
  osg::ref_ptr<osg::Vec4Array> sparseCols = new osg::Vec4Array();
  osg::ref_ptr<osg::Vec4Array> sparseTrueCols = new osg::Vec4Array();
  for (size_t i = 0; i < vertices->size(); i += sampleStep) {
    sparseVerts->push_back((*vertices)[i]);
    sparseCols->push_back((*colors)[i]);
    sparseTrueCols->push_back((*True_colors)[i]);
  }
  osg::ref_ptr<osg::Geometry> geom = new osg::Geometry();
  osg::ref_ptr<osg::Geode> geode = new osg::Geode();
  osg::Material* mat = new osg::Material();

  mat->setColorMode(osg::Material::ColorMode::DIFFUSE);
  osg::Point* point = new osg::Point(point_size);

  geom->setVertexArray(sparseVerts.get());
  geom->setColorArray(sparseTrueCols.get(), osg::Array::BIND_PER_VERTEX);

  // 存放类别颜色
  geom->setVertexAttribArray(10, sparseCols, osg::Array::BIND_PER_VERTEX);

  geom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, sparseVerts->size()));
  geom->getOrCreateStateSet()->setAttribute(mat);
  geom->getOrCreateStateSet()->setAttribute(point);

  geode->addDrawable(geom.get());
  return geode.release();
}

static osg::BoundingBox saveTileToDisk(
    const std::string& filePath, const osgEarth::SpatialReference* geoSRS,
    osg::ref_ptr<osg::Vec3Array> vertices, osg::ref_ptr<osg::Vec4Array> colors,
    osg::ref_ptr<osg::Vec4Array> True_colors) {
  // 1. 坐标转换到 ECEF
  ConvertPointCloudToECEF(vertices, geoSRS);

  // 2. 计算该块的包围盒
  osg::BoundingBox bb;
  for (const auto& v : *vertices) bb.expandBy(v);

  // 3. 创建该瓦片的内部 LOD 结构（全量层 + 抽稀层）
  osg::ref_ptr<osg::LOD> internalLOD = new osg::LOD();

  // 距离相机 0-5000米，显示全量
  internalLOD->addChild(
      createSparseGeode(vertices, colors, True_colors, 2.0f, 1), 0.0f, 5000.0f);

  // 距离相机 5000-1000000米，显示抽稀量（解决拉远时的卡顿）
  internalLOD->addChild(
      createSparseGeode(vertices, colors, True_colors, 4.0f, 10), 5000.0f,
      1000000.0f);

  // 4. 将该节点序列化到硬盘文件
  osgDB::writeNodeFile(*internalLOD, filePath,
                       new osgDB::Options("WriteImageHint=IncludeData"));
  return bb;  // 返回包围盒，用于主程序设置 PagedLOD 的中心和半径
}

osg::Group* convertLasToPagedTiles(const QString& lasFilePath,
                                   const QString& tilesBaseDir,
                                   const osgEarth::SpatialReference* geoSRS) {
  // 计时
  auto start_time = std::chrono::high_resolution_clock::now();
  static std::once_flag gdalInitFlag;

  // 初始化 GDAL 数据路径（只执行一次）
  std::call_once(gdalInitFlag, [&]() {
    QString gdal_data = QApplication::applicationDirPath() + "\\data";
    CPLSetConfigOption("GDAL_DATA", gdal_data.toLocal8Bit().constData());
  });

  // 初始化 LAS 读取器
  LASreadOpener lasreadopener;
  lasreadopener.set_file_name(lasFilePath.toLocal8Bit().constData());
  LASreader* lasreader = lasreadopener.open();
  if (!lasreader) return nullptr;

  // 定义相关变量
  osg::ref_ptr<osg::Group> rootGroup = new osg::Group();
  std::vector<TileTaskResult> results;
  std::vector<std::thread> workers;
  std::mutex resultMutex;
  osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array();
  osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array();  // 类别颜色
  osg::ref_ptr<osg::Vec4Array> trueColors = new osg::Vec4Array();  // 真彩色
  int tileIndex = 0;

  // 定义一个内部 Lambda 用于分发任务，减少重复代码
  auto dispatchTask =
      [&](int idx, std::string path, osg::ref_ptr<osg::Vec3Array> v,
          osg::ref_ptr<osg::Vec4Array> c, osg::ref_ptr<osg::Vec4Array> true_c) {
        workers.emplace_back([&results, idx, path, geoSRS, v, c, true_c]() {
          try {
            // 修改 saveTileToDisk 使其内部调用新的 ConvertPointCloudToECEF(v,
            // geoSRS)
            osg::BoundingBox bb = saveTileToDisk(path, geoSRS, v, c, true_c);

            results.push_back({idx, path, bb});
          } catch (...) {
            std::cerr << "Error processing tile " << idx << std::endl;
          }
        });
      };

  // 读取las，多线程构建瓦片
  while (lasreader->read_point()) {
    double x = lasreader->point.get_x();
    double y = lasreader->point.get_y();
    double z = lasreader->point.get_z();

    unsigned short rawR = lasreader->point.rgb[0];
    unsigned short rawG = lasreader->point.rgb[1];
    unsigned short rawB = lasreader->point.rgb[2];

    float r, g, b;

    // 自动探测：如果任一分量大于 255，判定为 16位数据
    // 如果全部小于等于 255，且不全为 0，通常判定为 8位数据
    if (rawR > 255 || rawG > 255 || rawB > 255) {
      // 16-bit 模式
      r = rawR / 65535.0f;
      g = rawG / 65535.0f;
      b = rawB / 65535.0f;
    } else {
      // 8-bit 模式 (或者虽然是16位但颜色非常暗，此时按8位处理视觉效果更好)
      r = rawR / 255.0f;
      g = rawG / 255.0f;
      b = rawB / 255.0f;
    }

    // 进一步保护：防止某些异常值超过 1.0
    r = std::max(0.0f, std::min(1.0f, r));
    g = std::max(0.0f, std::min(1.0f, g));
    b = std::max(0.0f, std::min(1.0f, b));

    trueColors->push_back(osg::Vec4(r, g, b, 1.0f));

    unsigned classfy = lasreader->point.get_classification();
    osg::Vec4 col =
        colorMap.count(classfy) ? colorMap.at(classfy) : osg::Vec4(1, 1, 1, 1);

    vertices->push_back(osg::Vec3d(x, y, z));
    colors->push_back(col);

    if (vertices->size() >= PerTileSize) {
      _Chinesepath = tilesBaseDir.toLocal8Bit().constData();
      std::string fileName =
          _Chinesepath + "tile_" + std::to_string(tileIndex++) + ".osgb";

      dispatchTask(tileIndex, fileName, vertices, colors, trueColors);
      vertices = new osg::Vec3Array();
      colors = new osg::Vec4Array();
      trueColors = new osg::Vec4Array();

      // 限制并发：每 4 个瓦片阻塞等待一次，防止内存和 I/O 撑爆
      if (workers.size() >= 4) {
        for (auto& t : workers)
          if (t.joinable()) t.join();
        workers.clear();
      }
    }
  }

  // 处理最后一个瓦片
  if (!vertices->empty()) {
    std::string fileName = _Chinesepath + "tile_last.osgb";
    dispatchTask(-1, fileName, vertices, colors, trueColors);
  }

  // 清理线程和LAS读取器
  for (auto& t : workers) {
    if (t.joinable()) t.join();
  }
  workers.clear();
  lasreader->close();
  delete lasreader;

  // 构建 PagedLOD 树
  osg::BoundingBox totalBB;
  osg::ref_ptr<osgDB::Options> localOpt = new osgDB::Options();
  localOpt->setDatabasePath(_Chinesepath);
  for (const auto& res : results) {
    if (!res.bb.valid()) continue;

    totalBB.expandBy(res.bb);

    osg::ref_ptr<osg::PagedLOD> plod = new osg::PagedLOD();

    plod->setDatabaseOptions(localOpt.get());
    plod->setCenter(res.bb.center());
    plod->setRadius(res.bb.radius());
    QFileInfo fi(QString::fromStdString(res.fileName));
    plod->setFileName(0, fi.fileName().toStdString());
    plod->setRange(0, 0.0f, 2000000.0f);

    rootGroup->addChild(plod.get());
  }

  auto end_time = std::chrono::high_resolution_clock::now();
  auto total_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end_time - start_time);
  std::cout << "--- Conversion Finished ---" << std::endl;
  std::cout << "Total Elapsed Time: " << total_duration.count() / 1000.0
            << " seconds" << std::endl;

  // 简单着色器，支持真彩色和类别色两种模式
  static const char* vertSource =
      "#version 330 compatibility\n"  // 使用兼容模式
      "layout(location = 0) in vec4 osg_Vertex;\n"
      "layout(location = 3) in vec4 osg_Color;\n"
      "layout(location = 10) in vec4 osg_SecondaryColor;\n"
      "uniform int u_colorMode;\n"
      "out vec4 v_color;\n"
      "void main() {\n"
      "    gl_Position = gl_ModelViewProjectionMatrix * osg_Vertex;\n"
      "    if(u_colorMode == 0) v_color = osg_Color;\n"
      "    else v_color = osg_SecondaryColor;\n"
      "}\n";

  // 片元着色器
  static const char* fragSource =
      "#version 330 compatibility\n"
      "in vec4 v_color;\n"
      "out vec4 out_FragColor;\n"
      "void main() {\n"
      "    out_FragColor = v_color;\n"
      "}\n";

  osg::StateSet* ss = rootGroup->getOrCreateStateSet();
  osg::ref_ptr<osg::Program> program = new osg::Program();
  program->addShader(new osg::Shader(osg::Shader::VERTEX, vertSource));
  program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragSource));
  ss->setAttributeAndModes(program, osg::StateAttribute::ON);
  osg::ref_ptr<osg::Uniform> colorModeUniform =
      new osg::Uniform("u_colorMode", 0);
  ss->addUniform(colorModeUniform.get());

  return rootGroup.release();
}
void updateCameraSensitivity(osgEarth::Util::EarthManipulator* manip) {
  if (!manip) return;

  osgEarth::Viewpoint vp = manip->getViewpoint();
  double range = vp.range().get().getValue();

  // 1. 计算倍率系数 (0.2 到 1.0 之间)
  double factor = (range - 200.0) / (50000.0 - 200.0);
  factor = std::max(0.0, std::min(1.0, factor));

  double panScale = 0.2 + factor * 0.8;
  double rotScale = 0.3 + factor * 0.7;

  osgEarth::Util::EarthManipulator::Settings* settings = manip->getSettings();

  // 2. 滚轮灵敏度 (直接设置)
  settings->setScrollSensitivity(0.1 + factor * 0.9);

  // 3. 配置平移 (Pan) 选项
  osgEarth::Util::EarthManipulator::ActionOptions panOptions;
  // 分别添加水平和垂直方向的灵敏度
  panOptions.add(osgEarth::Util::EarthManipulator::OPTION_SCALE_X, panScale);
  panOptions.add(osgEarth::Util::EarthManipulator::OPTION_SCALE_Y, panScale);

  settings->bindMouse(osgEarth::Util::EarthManipulator::ACTION_PAN,
                      osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON, 0, panOptions);

  // 4. 配置旋转 (Rotate) 选项
  osgEarth::Util::EarthManipulator::ActionOptions rotOptions;
  rotOptions.add(osgEarth::Util::EarthManipulator::OPTION_SCALE_X, rotScale);
  rotOptions.add(osgEarth::Util::EarthManipulator::OPTION_SCALE_Y, rotScale);

  settings->bindMouse(osgEarth::Util::EarthManipulator::ACTION_ROTATE,
                      osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON, 0,
                      rotOptions);
}

void adjustUI(Ui::OSGEarthAppClass* ui, int w, int h) {
  // 设置左侧工具栏固定宽度
  int toolbarFrame_width = 140, stackedWidget_width = 320;
  ui->toolbarFrame->setFixedWidth(toolbarFrame_width);
  ui->stackedWidget->setFixedWidth(stackedWidget_width);
  ui->widgetOSG->setFixedWidth(w - toolbarFrame_width - stackedWidget_width -
                               50);

  // 10. 右上角关闭按钮 (不变)
  ui->btn_Close->raise();
  ui->btn_Close->setStyleSheet(
      QString("QPushButton{border-image:url(%1/Resource/common/close.png)};")
          .arg(QApplication::applicationDirPath()));
}