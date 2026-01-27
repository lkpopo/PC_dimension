#pragma once
#include <osg/Group>
#include <osgManipulator/CommandManager>
#include <osgManipulator/Selection>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/TranslateAxisDragger>

class ManipulatorHelper {
 public:
  ManipulatorHelper(osg::Group* root);

  // 将拖拽器附加到目标节点（即你的 GeoTransform 下的那个控制节点）
  void attach(osg::MatrixTransform* target);

  // 移除并清理
  void detach();

 private:
  osg::ref_ptr<osg::Group> _root;
  osg::ref_ptr<osgManipulator::Selection> _selection;
  osg::ref_ptr<osgManipulator::CompositeDragger> _mainDragger;
  osg::ref_ptr<osgManipulator::CommandManager> _manager;
  osg::ref_ptr<osg::Group> _activeDraggerGroup;
};