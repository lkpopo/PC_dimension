#pragma once

#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <functional>

// OSG & osgEarth 核心头文件
#include <osg/Node>
#include <osg/ref_ptr>
#include <osgDB/ReadFile>
#include <osg/ComputeBoundsVisitor>
#include <osgDB/WriteFile>
#include <osgEarth/ElevationLayer>
#include <osgEarth/GeoTransform>
#include <osgEarth/ImageLayer>
#include <osgEarth/MapNode>
#include <osgEarth/ModelLayer>

// 针对 GDAL 的处理
#include <gdal_priv.h>
#include "utils.h"

    /**
     * @brief AssetLoader 类负责将具体的加载逻辑从主 App 中抽离
     * 它是无状态的工具类，或者由主 App 持有生命周期
     */
    class AssetLoader : public QObject {
  Q_OBJECT

 public:
  explicit AssetLoader(osg::ref_ptr<osgEarth::MapNode> mapNode,
                       QObject* parent = nullptr);
  virtual ~AssetLoader() = default;


  osg::ref_ptr<osg::Object> load(const QString& filePath, double lon,
                                 double lat, double alt);

  // --- 专用加载接口 (从原 OSGEarthApp 迁移) ---
  osg::ref_ptr<osg::Object> loadLas(const QString& filePath);
  osg::ref_ptr<osg::Object> loadTif(const QString& filePath);
  osg::ref_ptr<osg::Object> loadElevation(const QString& filePath);
  osg::ref_ptr<osg::Object> loadShp(const QString& filePath);
  osg::ref_ptr<osg::Object> loadObj(const QString& filePath, double lon,
                                    double lat, double alt = 0.0);

  // --- 工具函数 ---
  enum class TifType { IMAGE, ELEVATION };
  TifType checkTifType(const QString& filePath);

 private:
  // 持有 MapNode 引用，用于添加图层和获取 SRS
  osg::ref_ptr<osgEarth::MapNode> m_mapNode;

  // 缓存地理空间参考
  osg::ref_ptr<const osgEarth::SpatialReference> m_geoSRS;

  // 内部处理 LAS 转换逻辑 (封装原有的 convertLasToPagedTiles)
  //osg::ref_ptr<osg::Group> convertLasToPagedTiles(
  //    const QString& filePath, const QString& outputDir,
  //    const osgEarth::SpatialReference* srs);
};