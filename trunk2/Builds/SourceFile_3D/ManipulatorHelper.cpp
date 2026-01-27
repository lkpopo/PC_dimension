#include "ManipulatorHelper.h"

ManipulatorHelper::ManipulatorHelper(osg::Group* root) : _root(root) {
  _manager = new osgManipulator::CommandManager();
  _selection = new osgManipulator::Selection();
}

void ManipulatorHelper::attach(osg::MatrixTransform* target) {
  detach();

  // 1. 创建一个普通的 Group 作为拖拽器的容器
  // 不要用 CompositeDragger，直接用 Group 即可
  osg::ref_ptr<osg::Group> draggerGroup = new osg::Group();

  // 2. 创建平移拖拽器
  osg::ref_ptr<osgManipulator::TranslateAxisDragger> transDragger =
      new osgManipulator::TranslateAxisDragger();
  transDragger->setupDefaultGeometry();
  transDragger->setHandleEvents(true);

  // 3. 创建旋转拖拽器
  osg::ref_ptr<osgManipulator::TrackballDragger> rotateDragger =
      new osgManipulator::TrackballDragger();
  rotateDragger->setupDefaultGeometry();
  rotateDragger->setHandleEvents(true);

  // 4. 将它们加入容器
  draggerGroup->addChild(transDragger.get());
  draggerGroup->addChild(rotateDragger.get());

  // 5. 核心：分别将两个拖拽器连接到同一个 Selection
  // CommandManager 允许一个 Selection 被多个 Dragger 控制
  _manager->connect(*transDragger, *_selection);
  _manager->connect(*rotateDragger, *_selection);

  // 6. 统一设置拖拽器的初始大小和位置
  float radius = target->getBound().radius();
  osg::Matrix scaleMat =
      osg::Matrix::scale(radius * 1.5, radius * 1.5, radius * 1.5);

  // 注意：拖拽器本身也需要平移到模型的位置，否则它会在原点运行
  osg::Matrix modelMat = target->getMatrix();
  transDragger->setMatrix(scaleMat * modelMat);
  rotateDragger->setMatrix(scaleMat * modelMat);

  // 7. 建立层级
  osg::Group* parent = target->getParent(0);
  if (parent) {
    // 让 _selection 替换掉 target 的位置
    parent->replaceChild(target, _selection.get());
    if (_selection->getNumChildren() == 0) {
      _selection->addChild(target);
    }
    // 将拖拽器容器加入场景
    parent->addChild(draggerGroup.get());
  }

  // 记录引用以便以后清理
  _activeDraggerGroup = draggerGroup;
}

void ManipulatorHelper::detach() {
  if (_mainDragger.valid() && _mainDragger->getParent(0)) {
    _mainDragger->getParent(0)->removeChild(_mainDragger.get());
  }
  // 注意：实际项目中这里需要处理 Selection 的节点还原逻辑，即把 target
  // 重新挂回原父节点
}