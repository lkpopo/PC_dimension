#pragma once
#include <cpl_error.h>
#include <cpl_port.h>
#include <gdal.h>
#include <gdal_frmts.h>
#include <gdal_priv.h>
#include <gdal_version.h>
#include <osgPlugins/ive/EllipsoidModel.h>

#include <QApplication>
#include <QFileDialog>
#include <cmath>
#include <map>
#include <lasreader.hpp>
#include <osg/Array>
#include <osg/Point>
#include <osg/Vec3d>
#include <osg/ref_ptr>
#include <osgDB/WriteFile>
#include <osgEarth/GeoData>
#include <osgEarth/Map>
#include <osgEarth/MapNode>
#include <osgEarth/SpatialReference>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthFeatures/FeatureModelLayer>
#include <osgEarthSymbology/Style>
#include <osgEarthUtil/AutoClipPlaneHandler>
#include <osgViewer/Viewer>

#include "ogr_geometry.h"

const std::map<int, osg::Vec4> colorMap = {
    {0, osg::Vec4(0.52, 0.52, 0.52, 1.0)},
    {1, osg::Vec4(0.52, 0.52, 0.52, 1.0)},
    {2, osg::Vec4(0.1, 0.7, 0.1, 1.0)},  // 低植被
    {3, osg::Vec4(0.0, 1.0, 0.25, 1.0)},
    {4, osg::Vec4(0.1, 1.0, 0.1, 1.0)},  // 高植被
    {5, osg::Vec4(0.0, 0.0, 1.0, 1.0)},
    {6, osg::Vec4(1.0, 0.43, 0.7, 1.0)},      // 建筑物
    {11, osg::Vec4(0.74, 0.74, 0.38, 1.0)},   // 地面
    {16, osg::Vec4(0.96, 0.027, 0.98, 1.0)},  // 低导线
    {17, osg::Vec4(0.76, 0.76, 0.76, 1.0)},   // 杆塔
    {20, osg::Vec4(1.0, 0.73, 0.45, 1.0)},    // 高导线
    {27, osg::Vec4(1.0, 0.5, 0.75, 1.0)},     // 绝缘子
    {28, osg::Vec4(1.0, 0, 0, 1.0)},
    {-1, osg::Vec4(1.0, 0, 0, 1.0)}  // 默认黑色
};

constexpr auto PerTileSize = 200000;
constexpr auto VisualTileRange = 10000.0f;




osg::Group* convertLasToPagedTiles(const QString& lasFilePath,
                                   const QString& tilesBaseDir,
                                   const osgEarth::SpatialReference* geoSRS);