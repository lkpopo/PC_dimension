#include "AssetLoader.h"

AssetLoader::AssetLoader(osg::ref_ptr<osgEarth::MapNode> mapNode,
                         QObject* parent)
    : QObject(parent), m_mapNode(mapNode) {
  if (m_mapNode.valid()) {
    m_geoSRS = m_mapNode->getMapSRS()->getGeographicSRS();
  }
}

osg::ref_ptr<osg::Object> AssetLoader::loadLas(const QString& filePath) {
  QFileInfo lasInfo(filePath);
  QString tilesBaseDir =
      QApplication::applicationDirPath() + "/tiles/" + lasInfo.baseName() + "/";
  QString indexFile = tilesBaseDir + "index.osgb";

  // 检查索引树是否存在 ---
  if (QFileInfo::exists(indexFile)) {
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
    options->setDatabasePath(tilesBaseDir.toLocal8Bit().constData());

    // 使用 options 加载
    osg::ref_ptr<osg::Node> loadedIndex =
        osgDB::readNodeFile(indexFile.toLocal8Bit().constData(), options.get());
    if (loadedIndex.valid()) {
      return loadedIndex.get();
    }
  }

  // 构建瓦片
  QDir().mkpath(tilesBaseDir);
  osg::Group* resultNode =
      convertLasToPagedTiles(filePath, tilesBaseDir, m_geoSRS.get());
  if (resultNode) {
    osgDB::writeNodeFile(*resultNode, indexFile.toLocal8Bit().constData());
  }
  return resultNode;
}

osg::ref_ptr<osg::Object> AssetLoader::loadTif(const QString& filePath) {
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
  return layer;
}

osg::ref_ptr<osg::Object> AssetLoader::loadElevation(const QString& filePath) {
  // 1. 转换路径和名称
  std::string pathStr = filePath.toLocal8Bit().constData();
  std::string layerName =
      QFileInfo(filePath).baseName().toLocal8Bit().constData();

  // 2. 使用通用的 Config 构建配置 (万能方案)
  osgEarth::Config layerConfig;
  layerConfig.add("name", layerName);
  layerConfig.add("driver", "gdal");
  layerConfig.add("url", pathStr);

  // 3. 通过 Config 创建图层选项
  osgEarth::ElevationLayerOptions layerOpt(layerConfig);

  // 4. 创建并添加高程图层
  osg::ref_ptr<osgEarth::ElevationLayer> layer =
      new osgEarth::ElevationLayer(layerOpt);
  m_mapNode->getMap()->addLayer(layer.get());

  return layer.get();
}

osg::ref_ptr<osg::Object> AssetLoader::loadShp(const QString& filePath) {
  std::string shpPath = filePath.toLocal8Bit().constData();

  // 1. 配置数据源
  osgEarth::Drivers::OGRFeatureOptions featureData;
  featureData.url() = shpPath;

  // 2. 创建源图层
  osgEarth::Features::FeatureSourceLayerOptions ogrLayerOpt;
  std::string sourceName = shpPath + "_source";
  ogrLayerOpt.name() = sourceName;
  ogrLayerOpt.featureSource() = featureData;
  osg::ref_ptr<osgEarth::Features::FeatureSourceLayer> sourceLayer =
      new osgEarth::Features::FeatureSourceLayer(ogrLayerOpt);
  m_mapNode->getMap()->addLayer(sourceLayer.get());

  // 3. 样式配置
  osgEarth::Symbology::Style style;
  osgEarth::Symbology::RenderSymbol* rs =
      style.getOrCreate<osgEarth::Symbology::RenderSymbol>();
  rs->depthTest() = false;

  // 设置矢量面样式（包括边界线）
  osgEarth::Symbology::LineSymbol* ls =
      style.getOrCreateSymbol<osgEarth::Symbology::LineSymbol>();
  ls->stroke()->color() = osgEarth::Symbology::Color("#FA8072");
  ls->stroke()->width() = 1.0;
  ls->tessellationSize()->set(100, osgEarth::Units::KILOMETERS);

  // osgEarth::Symbology::PolygonSymbol* polygonSymbol =
  //     style.getOrCreateSymbol<osgEarth::Symbology::PolygonSymbol>();
  // polygonSymbol->fill()->color() = osgEarth::Symbology::Color(
  //     152.0f / 255, 251.0f / 255, 152.0f / 255, 0.8f);  // 238 230 133
  // polygonSymbol->outline() = true;

  // 4. 创建模型图层
  osgEarth::Features::FeatureModelLayerOptions fmlOpt;
  fmlOpt.name() = shpPath;
  fmlOpt.featureSourceLayer() = sourceName;
  fmlOpt.enableLighting() = false;
  fmlOpt.styles() = new osgEarth::Symbology::StyleSheet();
  fmlOpt.styles()->addStyle(style);

  osg::ref_ptr<osgEarth::Features::FeatureModelLayer> fml =
      new osgEarth::Features::FeatureModelLayer(fmlOpt);
  m_mapNode->getMap()->addLayer(fml.get());

  return fml.get();
}

osg::ref_ptr<osg::Object> AssetLoader::loadObj(const QString& filePath,
                                               double lon, double lat,
                                               double alt) {
  std::string pathStr = filePath.toLocal8Bit().constData();
  osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(pathStr);
  if (!model.valid()) {
    return nullptr;
  }
  // 计算包围盒
  osg::ComputeBoundsVisitor cbv;
  model->accept(cbv);
  osg::BoundingBox bb = cbv.getBoundingBox();

  // 计算底部偏移：如果原点在中心，bb.zMin() 通常是一个负数
  // 我们需要把模型向上移动 -bb.zMin() 的距离
  float offsetZ = -bb.zMin();

  // 创建地理变换节点
  osgEarth::GeoTransform* xform = new osgEarth::GeoTransform();
  osgEarth::GeoPoint pos(m_mapNode->getMapSRS(), lon, lat, alt + offsetZ,
                         osgEarth::ALTMODE_RELATIVE);
  xform->setPosition(pos);
  xform->addChild(model.get());
  return xform;
}

osg::ref_ptr<osg::Object> AssetLoader::load(const QString& filePath, double lon,
                                            double lat, double alt) {
  QFileInfo fileInfo(filePath);
  QString suffix = fileInfo.suffix().toLower();

  if (suffix == "las") return loadLas(filePath);
  if (suffix == "shp") return loadShp(filePath);
  if (suffix == "obj") return loadObj(filePath, lon, lat, alt);
  if (suffix == "tif") {
    return (checkTifType(filePath) == TifType::ELEVATION)
               ? loadElevation(filePath)
               : loadTif(filePath);
  }
  return nullptr;
}

AssetLoader::TifType AssetLoader::checkTifType(const QString& filePath) {
  // 强制注册 GDAL 驱动
  GDALAllRegister();

  GDALDataset* poDataset =
      (GDALDataset*)GDALOpen(filePath.toLocal8Bit().constData(), GA_ReadOnly);
  if (!poDataset) return TifType::IMAGE;  // 打不开默认当影像

  TifType result = TifType::IMAGE;

  if (poDataset->GetRasterCount() > 0) {
    // 获取第一个波段的数据类型
    GDALRasterBand* poBand = poDataset->GetRasterBand(1);
    GDALDataType dataType = poBand->GetRasterDataType();

    // 如果是浮点型 (Float32/Float64) 或者 16位整数，通常是高程
    if (dataType == GDT_Float32 || dataType == GDT_Float64 ||
        dataType == GDT_Int16) {
      result = TifType::ELEVATION;
    }
  }

  GDALClose(poDataset);
  return result;
}
