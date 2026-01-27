#include "Controller.h"

Controller::Controller(osg::Node* model) {
  _fallMT = new osg::MatrixTransform();
  _adjustMT = new osg::MatrixTransform();

  // 1. 计算模型大小，确定底部中心
  osg::ComputeBoundsVisitor cbv;
  model->accept(cbv);
  _bbox = cbv.getBoundingBox();

  // 2. 校准模型：让模型底座中心位于 (0,0,0)
  float centerX = (_bbox.xMax() + _bbox.xMin()) / 2.0;
  float centerY = (_bbox.yMax() + _bbox.yMin()) / 2.0;
  float bottomZ = _bbox.zMin();
  _adjustMT->setMatrix(osg::Matrix::translate(-centerX, -centerY, -bottomZ));

  // 3. 组装层级
  _fallMT->addChild(_adjustMT.get());
  _adjustMT->addChild(model);
}

void Controller::updateFall(Direction dir, float angleDeg) {
  float angleRad = osg::DegreesToRadians(angleDeg);

  // 计算旋转轴心（Pivot）
  // 假设底座半宽为 w，深度为 d
  float w = (_bbox.xMax() - _bbox.xMin()) / 2.0;
  float d = (_bbox.yMax() - _bbox.yMin()) / 2.0;

  osg::Vec3 pivot, axis;
  switch (dir) {
    case RIGHT:
      pivot.set(w, 0, 0);
      axis.set(0, 1, 0);
      break;
    case LEFT:
      pivot.set(-w, 0, 0);
      axis.set(0, 1, 0);
      break;
    case FRONT:
      pivot.set(0, d, 0);
      axis.set(1, 0, 0);
      break;
    case BACK:
      pivot.set(0, -d, 0);
      axis.set(1, 0, 0);
      break;
  }

  // 物理逻辑：绕边缘旋转的矩阵公式
  // M = T(pivot) * R(angle) * T(-pivot)
  osg::Matrix m = osg::Matrix::translate(-pivot) *
                  osg::Matrix::rotate(angleRad, axis) *
                  osg::Matrix::translate(pivot);

  _fallMT->setMatrix(m);
}