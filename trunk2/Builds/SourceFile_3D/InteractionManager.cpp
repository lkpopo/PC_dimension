#include "InteractionManager.h"

InteractionManager::InteractionManager(osgViewer::Viewer* viewer,
                                       osg::Group* dataGroup,
                                       osg::Group* interactionGroup,
                                       QObject* parent)
    : QObject(parent), m_viewer(viewer) {
  m_tempGroup = new osg::Group();
  if (interactionGroup) {
    interactionGroup->addChild(m_tempGroup);
  }
  initMeasureGeometry();
  initClipGeometry();
  setupClipShader(dataGroup);
}

void InteractionManager::setMode(InterMode mode) {
  m_currentMode = mode;
  clearAll();
  if (mode == InterMode::VIEW) {
    m_tempGroup->setNodeMask(0);  // 关键：0 表示彻底不渲染，不论坐标在哪
  } else {
    m_tempGroup->setNodeMask(0xffffffff);  // 恢复渲染
  }
}

void InteractionManager::clearAll() {
  if (m_distanceText) m_distanceText->setText("");

  if (!m_lineCoords->empty()) {
    m_lineCoords->clear();
    refreshGeometry(m_lineGeom.get());
  }
  m_isFirstClick = true;

  if (!m_clipCoords->empty()) {
    m_clipCoords->clear();
    refreshGeometry(m_clipGeom.get());
  }
  m_isClipFinished = false;
}

bool InteractionManager::handle(const osgGA::GUIEventAdapter& ea,
                                osgGA::GUIActionAdapter& aa) {

  if (m_currentMode == InterMode::VIEW) {
    if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE &&
        ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
      osg::Vec3d worldPos;
      if (pick(ea.getX(), ea.getY(), worldPos)) {
        emit requestContextMenu(worldPos);
        return true;
      }
    }
    return false;
  }

  // ESC按键处理
  if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
    if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Escape) {
      clearAll();
      return true;  // 事件已处理，不再向下传递
    }
  }

  // 锁定相机
  if (ea.getEventType() == osgGA::GUIEventAdapter::DRAG ||
      ea.getEventType() == osgGA::GUIEventAdapter::SCROLL) {
    return true;
  }

  switch (m_currentMode) {
    case InterMode::MEASURE:
      return handleMeasure(ea, aa);
    case InterMode::CLIP:
      return handleClip(ea, aa);
    default:
      return false;
  }
}

bool InteractionManager::pick(float x, float y, osg::Vec3d& out_pos) {
  double delta = 5.0;
  osg::ref_ptr<osgUtil::PolytopeIntersector> intersector =
      new osgUtil::PolytopeIntersector(osgUtil::Intersector::WINDOW, x - delta,
                                       y - delta, x + delta, y + delta);

  intersector->setIntersectionLimit(osgUtil::Intersector::LIMIT_NEAREST);

  osgUtil::IntersectionVisitor iv(intersector.get());
  if (m_currentMode == InterMode::CLIP)
    iv.setTraversalMask(MASK_TERRAIN);
  else
    iv.setTraversalMask(0xffffffff);
  m_viewer->getCamera()->accept(iv);

  if (intersector->containsIntersections()) {
    auto& intersections = intersector->getIntersections();

    for (auto& intersection : intersections) {
      osg::Vec3 localPoint = intersection.localIntersectionPoint;
      osg::Matrix localToWorld =
          osg::computeLocalToWorld(intersection.nodePath);
      out_pos = localPoint * localToWorld;
      return true;
    }
  }
  return false;
}

void InteractionManager::updateMeasureLine(const osg::Vec3d& start,
                                           const osg::Vec3d& end) {
  if (m_lineCoords->size() < 2) {
    m_lineCoords->resize(2);
  }
  (*m_lineCoords)[0] = start;
  (*m_lineCoords)[1] = end;

  refreshGeometry(m_lineGeom.get());
}

void InteractionManager::updateText(const osg::Vec3d& pos, double distance) {
  if (!m_distanceText || !m_textAT.valid()) return;

  QString text = QString("%1 m").arg(distance, 0, 'f', 2);

  m_distanceText->setText(text.toUtf8().constData(),
                          osgText::String::ENCODING_UTF8);
  m_textAT->setPosition(pos);
}

void InteractionManager::initClipGeometry() {
  m_clipGeom = new osg::Geometry();
  m_clipCoords = new osg::Vec3Array();
  m_clipGeom->setVertexArray(m_clipCoords);
  m_clipGeom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, 0));

  m_clipGeode = new osg::Geode();
  m_clipGeode->addDrawable(m_clipGeom);

  osg::StateSet* ss = m_clipGeode->getOrCreateStateSet();
  ss->setMode(GL_DEPTH_TEST,
              osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
  ss->setAttributeAndModes(new osg::LineWidth(2.5f), osg::StateAttribute::ON);
  ss->setRenderBinDetails(10000, "RenderBin");

  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
  m_clipGeom->setColorArray(colors, osg::Array::BIND_OVERALL);

  m_tempGroup->addChild(m_clipGeode);
}

void InteractionManager::initMeasureGeometry() {
  osg::Vec4Array* colors = new osg::Vec4Array();
  colors->push_back(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));

  // 初始化线几何体
  m_lineGeom = new osg::Geometry();
  m_lineCoords = new osg::Vec3Array();
  m_lineGeom->setVertexArray(m_lineCoords);
  m_lineGeom->addPrimitiveSet(
      new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, 0));
  m_lineGeom->setColorArray(colors, osg::Array::BIND_OVERALL);  // 设置线条颜色

  osg::Geode* geode = new osg::Geode();
  geode->setName("MeasureLineGeode");
  osg::StateSet* ss = geode->getOrCreateStateSet();

  ss->setAttributeAndModes(new osg::LineWidth(2.0f));  // 设置线条宽度
  osg::ref_ptr<osg::PolygonOffset> polyoffset =
      new osg::PolygonOffset(-1.0f, -1.0f);
  ss->setAttributeAndModes(polyoffset, osg::StateAttribute::ON);
  ss->setRenderBinToInherit();
  geode->addDrawable(m_lineGeom);
  m_tempGroup->addChild(geode);

  m_distanceText = new osgText::Text();
  m_distanceText->setColor(
      osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));  // 文字颜色：黄色

  // 屏幕像素模式，确保大小合适
  m_distanceText->setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
  m_distanceText->setCharacterSize(50.0);  // 世界尺度下调
  m_distanceText->setAxisAlignment(osgText::Text::XY_PLANE);

  // 关键：去掉可能会产生大色块的背景设置
  m_distanceText->setBackdropType(osgText::Text::NONE);

  m_textGeode = new osg::Geode();
  m_textGeode->addDrawable(m_distanceText);

  ss = m_textGeode->getOrCreateStateSet();

  // 2. 彻底关闭光照和深度测试
  ss->setMode(GL_DEPTH_TEST,
              osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);

  // 3. 隔离Shader
  ss->setAttributeAndModes(new osg::Program(),
                           osg::StateAttribute::OFF |
                               osg::StateAttribute::OVERRIDE |
                               osg::StateAttribute::PROTECTED);

  ss->setRenderBinDetails(1000, "RenderBin");

  // 添加AutoTransform节点 实现文字铁近点云时 扭曲的问题
  m_textAT = new osg::AutoTransform;
  m_textAT->setAutoRotateMode(osg::AutoTransform::ROTATE_TO_SCREEN);
  m_textAT->setAutoScaleToScreen(true);
  m_textAT->setDataVariance(osg::Object::DYNAMIC);

  m_textAT->addChild(m_textGeode);
  m_tempGroup->addChild(m_textAT);
}

bool InteractionManager::handleMeasure(const osgGA::GUIEventAdapter& ea,
                                       osgGA::GUIActionAdapter& aa) {
  if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE) {
    if (!m_isFirstClick) {
      osg::Vec3d hitPos;
      if (pick(ea.getX(), ea.getY(), hitPos)) {
        m_currentMousePoint = hitPos;
        updateMeasureLine(m_startPoint, m_currentMousePoint);
        double dist = (m_currentMousePoint - m_startPoint).length();
        updateText(m_currentMousePoint, dist);
      }
    }
  }

  // 仅响应鼠标左键按下
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
    osg::Vec3d hitPos;
    if (pick(ea.getX(), ea.getY(), hitPos)) {
      if (m_isFirstClick) {
        clearAll();
        m_startPoint = hitPos;
        m_isFirstClick = false;
      } else {
        updateMeasureLine(m_startPoint, hitPos);
        m_isFirstClick = true;  // 测完一次重置
      }
      return true;
    }
  }
}

bool InteractionManager::handleClip(const osgGA::GUIEventAdapter& ea,
                                    osgGA::GUIActionAdapter& aa) {
  float x = ea.getX();
  float y = ea.getY();

  // --- 1. 鼠标移动：更新橡皮筋 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE) {
    if (m_clipCoords->size() >= 2 && !m_isClipFinished) {
      osg::Vec3d hitPos;
      if (pick(x, y, hitPos)) {
        m_clipCoords->back() = hitPos;
        refreshGeometry(m_clipGeom.get());
      }
    }
    return false;
  }

  // --- 2. 鼠标左键：固定顶点 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) {
    osg::Vec3d hitPos;
    if (pick(x, y, hitPos)) {
      if (m_isClipFinished) {
        m_clipCoords->clear();
        m_isClipFinished = false;
      }

      if (m_clipCoords->empty()) {
        // 第一次点击：放两个点（一个固定起点，一个跟随鼠标的终点）
        m_clipCoords->push_back(hitPos);
        m_clipCoords->push_back(hitPos);
      } else {
        m_clipCoords->push_back(hitPos);
      }
      refreshGeometry(m_clipGeom.get());
      return true;
    }
  }

  // --- 3. 鼠标右键：闭合区域 ---
  if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH &&
      ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) {
    if (m_clipCoords->size() > 2 && !m_isClipFinished) {
      m_clipCoords->pop_back();  // 移除最后那个跟随鼠标的临时点
      m_isClipFinished = true;
      refreshGeometry(m_clipGeom.get());
    }
    return true;
  }

  return false;
}

void InteractionManager::refreshGeometry(osg::Geometry* geom) {
  if (!geom) return;

  osg::Array* vertices = geom->getVertexArray();
  if (!vertices) return;

  osg::DrawArrays* da =
      dynamic_cast<osg::DrawArrays*>(geom->getPrimitiveSet(0));
  if (da) {
    da->setCount(vertices->getNumElements());
  }

  // 3. 标记数据脏区
  vertices->dirty();         // 告诉 OSG 顶点数据变了
  geom->dirtyBound();        // 重新计算包围盒
  geom->dirtyDisplayList();  // 刷新显存缓冲区
}

void InteractionManager::setupClipShader(osg::Group* dataGroup) {
  osg::StateSet* ss = dataGroup->getOrCreateStateSet();

  osg::ref_ptr<osg::Program> program = new osg::Program;

  // 顶点着色器：计算点在“裁剪相机”视角下的屏幕投影位置
  const char* vertSource =
      "#version 120\n"
      "varying vec4 v_clipPos;\n"
      "uniform mat4 u_clipMVP;\n"
      "void main() {\n"
      "    // 计算当前顶点在裁剪那一刻的投影坐标\n"
      "    v_clipPos = u_clipMVP * gl_Vertex;\n"
      "    \n"
      "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
      "    gl_FrontColor = gl_Color;\n"
      "    gl_PointSize = 2.0;\n"
      "}\n";

  // 片元着色器：基于屏幕多边形进行丢弃判定
  const char* fragSource =
      "#version 120\n"
      "varying vec4 v_clipPos;\n"
      "uniform float u_polyX[64];\n"
      "uniform float u_polyY[64];\n"
      "uniform int u_polySize;\n"
      "uniform int u_clipMode;\n"  // 0:正常, 1:裁掉内部, 2:裁掉外部
      "void main() {\n"
      "    if (u_clipMode == 0 || u_polySize < 3) {\n"
      "        gl_FragColor = gl_Color;\n"
      "        return;\n"
      "    }\n"
      "    \n"
      "    // 1. 透视除法：转为 NDC 坐标 (-1 到 1)\n"
      "    vec3 ndc = v_clipPos.xyz / v_clipPos.w;\n"
      "    \n"
      "    // 2. 映射到屏幕归一化坐标 (0 到 1)，对应 C++ 传进来的 polyX/Y\n"
      "    vec2 uv = ndc.xy * 0.5 + 0.5;\n"
      "    \n"
      "    // 3. 如果点在相机背面（w < 0），通常不进行视口内裁剪判定\n"
      "    if (v_clipPos.w <= 0.0) {\n"
      "        gl_FragColor = gl_Color;\n"
      "        return;\n"
      "    }\n"
      "    \n"
      "    // 4. PNPOLY 算法判定\n"
      "    bool inside = false;\n"
      "    for (int i = 0, j = u_polySize - 1; i < u_polySize; j = i++) {\n"
      "        if (((u_polyY[i] > uv.y) != (u_polyY[j] > uv.y)) &&\n"
      "            (uv.x < (u_polyX[j] - u_polyX[i]) * (uv.y - u_polyY[i]) / "
      "(u_polyY[j] - u_polyY[i]) + u_polyX[i])) {\n"
      "            inside = !inside;\n"
      "        }\n"
      "    }\n"
      "    \n"
      "    if (u_clipMode == 1 && inside) discard;\n"
      "    if (u_clipMode == 2 && !inside) discard;\n"
      "    \n"
      "    gl_FragColor = gl_Color;\n"
      "}\n";

  program->addShader(new osg::Shader(osg::Shader::VERTEX, vertSource));
  program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragSource));
  ss->setAttributeAndModes(program, osg::StateAttribute::ON);

  // 初始化默认 Uniform
  ss->getOrCreateUniform("u_clipMode", osg::Uniform::INT)->set(0);
  ss->getOrCreateUniform("u_polySize", osg::Uniform::INT)->set(0);
}

void InteractionManager::updateClipUniforms(osg::Group* dataGroup, int mode) {
  if (m_clipCoords->empty()) return;

  osg::Camera* cam = m_viewer->getCamera();
  osg::Viewport* vp = cam->getViewport();

  // 1. 冻结当前的 MVP 矩阵
  osg::Matrixf viewMat = cam->getViewMatrix();
  osg::Matrixf projMat = cam->getProjectionMatrix();
  osg::Matrixf clipMVP = viewMat * projMat;

  // 2. 转换多边形顶点为屏幕 0-1 坐标

  osg::Matrixf mvpw = clipMVP * vp->computeWindowMatrix();
  int size = std::min((int)m_clipCoords->size(), 64);

  const int MAX_POLY_POINTS = 64;
  osg::ref_ptr<osg::FloatArray> polyX = new osg::FloatArray(MAX_POLY_POINTS);
  osg::ref_ptr<osg::FloatArray> polyY = new osg::FloatArray(MAX_POLY_POINTS);

  for (int i = 0; i < MAX_POLY_POINTS; ++i) {
    if (i < size) {
      osg::Vec3 screenPos = m_clipCoords->at(i) * mvpw;
      (*polyX)[i] = screenPos.x() / (float)vp->width();
      (*polyY)[i] = screenPos.y() / (float)vp->height();
    } else {
      (*polyX)[i] = 0.0f;  // 填充默认值
      (*polyY)[i] = 0.0f;
    }
  }

  // 3. 提交到 StateSet
  osg::StateSet* ss = dataGroup->getOrCreateStateSet();
  ss->getOrCreateUniform("u_clipMVP", osg::Uniform::FLOAT_MAT4)->set(clipMVP);

  ss->getOrCreateUniform("u_polyX", osg::Uniform::FLOAT, MAX_POLY_POINTS)
      ->setArray(polyX.get());
  ss->getOrCreateUniform("u_polyY", osg::Uniform::FLOAT, MAX_POLY_POINTS)
      ->setArray(polyY.get());

  ss->getOrCreateUniform("u_polySize", osg::Uniform::INT)->set(size);
  ss->getOrCreateUniform("u_clipMode", osg::Uniform::INT)->set(mode);
}
