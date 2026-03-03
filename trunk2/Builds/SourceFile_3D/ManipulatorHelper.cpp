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
  // 1. 彻底移除旧的 XYZ 轴 (这是消失的关键)
  if (_activeDraggerGroup.valid()) {
    // 遍历所有父节点并移除自己
    osg::Node::ParentList parents = _activeDraggerGroup->getParents();
    for (osg::Group* parent : parents) {
      parent->removeChild(_activeDraggerGroup.get());
    }
    _activeDraggerGroup = nullptr;  // 释放引用
  }

  // 2. 还原模型层级 (防止模型留在 Selection 节点里)
  if (_selection.valid() && _selection->getNumChildren() > 0) {
    osg::ref_ptr<osg::Node> target = _selection->getChild(0);
    osg::Group* selectionParent = _selection->getParent(0);

    if (selectionParent && target) {
      // 获取 Selection 的当前矩阵，补偿给模型 (防止跳回原位)
      osg::MatrixTransform* mt =
          dynamic_cast<osg::MatrixTransform*>(target.get());
      if (mt) {
        mt->setMatrix(mt->getMatrix() * _selection->getMatrix());
      }

      // 物理还原子节点
      selectionParent->replaceChild(_selection.get(), target.get());
    }
    _selection->removeChild(target.get());
  }

  // 3. 彻底重置管理器 (断开信号槽连接)
  _manager = new osgManipulator::CommandManager();
  _selection = new osgManipulator::Selection();
}