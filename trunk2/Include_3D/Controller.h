#pragma once
#include <osg/MatrixTransform>
#include <osgEarth/GeoData>
#include <osg/ComputeBoundsVisitor>

class Controller {
 public:
  enum Direction { FRONT, BACK, LEFT, RIGHT };

  Controller(osg::Node* model);

  // 获取构建好的层级根节点（即那个 GeoTransform）
  osg::MatrixTransform* getAnimationNode() { return _fallMT.get(); }

  // 执行倒塌：传入方向和当前角度 (0-90)
  void updateFall(Direction dir, float angleDeg);

 private:
  osg::ref_ptr<osg::MatrixTransform> _fallMT;    // 动画层
  osg::ref_ptr<osg::MatrixTransform> _adjustMT;  // 校准层
  osg::BoundingBox _bbox;  // 模型的包围盒，用于计算底部边缘
};