#include "PanoramaWidget.h"
#include <QDebug>
#include <cmath>
#include <QtCore/qmath.h>
#include <QtGlobal>
#include <algorithm>
#include <QVector>
#include <QVector2D>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QDebug>
#include <cmath>
#include <QPainter>
#include <windows.h>
#include <iostream>
#include <QApplication>
#include <QTransform>
#include <QMutex>
#include "PanoramaWidget.h"
#include <QPoint>
#include <cmath>
#include <QVector3D>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMouseEvent>
#include <QDebug>
#include "CommonTool.h"

PanoramaWidget::PanoramaWidget(QString picPath, bool autoRotate, bool blCompassShow, QWidget* parent)
    : QOpenGLWidget(parent)
    , m_picPath(picPath)//场景图片地址
    , m_autoRotate(autoRotate)//是否自动旋转
    , m_blCompassShow(blCompassShow)//是否显示指北针
    , m_fov(60.0)//初始视角范围
    , m_rotation(QVector3D(0, 0, 0))//初始位置
    , m_zoom(1.0f)//初始缩放
    , m_zoom_min(0.5f)//最小缩放倍数
    , m_zoom_max(3.0f)//最大缩放倍数
    , m_H_min(-180.0f)//左侧水平视角，负值
    , m_H_max(180.0f)//右侧水平视角，正值
    , m_V_min(-90.0f)//下方垂直视角，负值
    , m_V_max(90.0f)//上方垂直视角，正值
    , m_compassN(0)//正北方向
    , m_compassN_picpath("")//指北针图片地址
    , m_compassN_location("")//指北针显示位置
    , m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_isDragging(false)
    , m_isMovingHotspot(false)
    , m_selectedHotspot(-1)
    , m_sphereStacks(50)
    , m_sphereSlices(100) 
    , m_rotationSpeed(0.05f)
    , m_inactivityDelay(3000) 
    , m_hotspotMovingEnabled(true) 
    , m_transitionEnabled(true)
    , m_isTransitioning(false)
    , m_transitionProgress(0.0f)
    , m_animation(nullptr)  
    , m_program(nullptr)
    , m_texture(nullptr)
{
    // 设置OpenGL版本和格式
    QSurfaceFormat format;
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);

    //初始化Hotspot
    InitHotspot();

    // 创建指北针按钮
    m_compassLabel = new RotatableLabel(this);
    m_compassLabel->setFixedSize(100, 100);

    // 初始化定时器
    m_animationTimer = new QTimer(this);
    m_inactivityTimer = new QTimer(this);
    if (m_autoRotate == true)
    {
        connect(m_animationTimer, &QTimer::timeout, this, &PanoramaWidget::onAnimationTimer);
        connect(m_inactivityTimer, &QTimer::timeout, this, &PanoramaWidget::startAutoRotation);
    }
  
    // 创建过渡动画
    m_animation = new QPropertyAnimation(this, "transitionProgress");
    m_animation->setDuration(1000); // 1秒过渡
    m_animation->setStartValue(0.0f);
    m_animation->setEndValue(1.0f);
    m_animation->setEasingCurve(QEasingCurve::InOutQuad);

    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);


}

PanoramaWidget::~PanoramaWidget()
{
    makeCurrent();
    if (m_texture) delete m_texture;
    if (m_program) delete m_program;

    if (m_animationTimer) delete m_animationTimer;
    if (m_inactivityTimer) delete m_inactivityTimer;

    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_ebo) glDeleteBuffers(1, &m_ebo);

    doneCurrent();
}

void PanoramaWidget::createSphereGeometry()
{
    QVector<float> vertices;
    QVector<unsigned int> indices;
    const float radius = 1.0f;
    for (int i = 0; i <= m_sphereStacks; ++i) {
        float phi = M_PI * i / m_sphereStacks;

        for (int j = 0; j <= m_sphereSlices; ++j) {
            float theta = 2 * M_PI * j / m_sphereSlices;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * cos(phi);
            float z = radius * sin(phi) * sin(theta);

            float u = /*1.0f -*/ static_cast<float>(j) / m_sphereSlices;
            float v = 1.0f - static_cast<float>(i) / m_sphereStacks;

            vertices.append(x);
            vertices.append(y);
            vertices.append(z);
            vertices.append(u);
            vertices.append(v);
        }
    }

    for (int i = 0; i < m_sphereStacks; ++i) {
        for (int j = 0; j < m_sphereSlices; ++j) {
            int first = i * (m_sphereSlices + 1) + j;
            int second = first + m_sphereSlices + 1;

            indices.append(first);
            indices.append(second);
            indices.append(first + 1);

            indices.append(first + 1);
            indices.append(second);
            indices.append(second + 1);
        }
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
        vertices.constData(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
        indices.constData(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
        (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void PanoramaWidget::initHotspotGeometry()
{
    // 热点四边形顶点数据
    float vertices[] = {
        // 位置             // 纹理坐标
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, 0.0f,  0.0f, 0.0f,
         0.5f,  0.5f, 0.0f,  1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f,  0.0f, 1.0f
    };

    m_hotspotVAO.create();
    m_hotspotVAO.bind();

    m_hotspotVBO.create();
    m_hotspotVBO.bind();
    m_hotspotVBO.allocate(vertices, sizeof(vertices));

    m_hotspotProgram->enableAttributeArray(0);
    m_hotspotProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(float));
    m_hotspotProgram->enableAttributeArray(1);
    m_hotspotProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(float), 2, 5 * sizeof(float));

    m_hotspotVAO.release();
}

void PanoramaWidget::changePic(QString picPath, QString id, bool isTransitioning)
{
    if (picPath == m_picPath)  return;
    //重置热点对应的屏幕位置
    m_mapAllHpScreenPos.clear();
    //
    // 更新当前图片路径
    m_picID = id;
    m_picPath = picPath;

    //初始化窗口参数
    initWindow(picPath);

    if (m_isTransitioning)
    {
        qWarning() << "Transition in progress, ignoring new image change request";
        return;
    }
    if (!isTransitioning)
    {
        // 直接切换模式
        loadPanorama(picPath);
        return;
    }
    // 淡入淡出过渡模式
    m_isTransitioning = isTransitioning;
    m_transitionProgress = 0.0f;

    // 加载下一张纹理
    makeCurrent();
    QImage image(m_picPath);
    if (image.isNull())
    {
        qDebug() << "Failed to load image:" << m_picPath;
        return;
    }
    if (m_nextTexture)
    {
        delete m_nextTexture;
        m_nextTexture = nullptr;
    }
    m_nextTexture = new QOpenGLTexture(image.mirrored());
    m_nextTexture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    m_nextTexture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_nextTexture->setWrapMode(QOpenGLTexture::Repeat);
    doneCurrent();

    // 启动过渡动画
    m_animation->start();
}

void PanoramaWidget::SetAllPicAngle(std::map<QString, WindowParameter>& mapPicAngle)
{
    m_mapPicAngle = mapPicAngle;
}

void PanoramaWidget::SetOnePicAngle(QString pic, WindowParameter wPt)
{
    m_mapPicAngle.insert(std::make_pair(pic, wPt));
}

void PanoramaWidget::SetAllPicCompass(std::map<QString, CompassParameter>& mapPicCompass)
{
    m_mapPicCompass = mapPicCompass;
}

void PanoramaWidget::SetOnePicCompass(QString pic, CompassParameter wPt)
{
    m_mapPicCompass.insert(std::make_pair(pic, wPt));
}

void PanoramaWidget::initWindow(QString& picPath)
{  
    //视角
    WindowParameter pam = m_mapPicAngle[picPath];
    m_fov = pam.fov.toFloat();
    QStringList angleList = pam.start_angle.split(";");
    if (angleList.size() == 3)
        m_rotation = QVector3D(angleList[0].toFloat(), angleList[1].toFloat(), angleList[2].toFloat());

    m_zoom = pam.zoom.toFloat();
    QStringList zoomList = pam.zoom_range.split(";");
    if (zoomList.size() == 2)
    {
        m_zoom_min = zoomList[0].toFloat();
        m_zoom_max = zoomList[1].toFloat();
    }

    QStringList HList = pam.h_range.split(";");
    if (HList.size() == 2)
    {
        m_H_min = HList[0].toFloat();
        m_H_max = HList[1].toFloat();
    }
    QStringList VList = pam.v_range.split(";");
    if (VList.size() == 2)
    {
        m_V_min = VList[0].toFloat();
        m_V_max = VList[1].toFloat();
    } 
    //指北针
    CompassParameter comp = m_mapPicCompass[picPath];
    m_compassN = comp.compassAngle;
    m_compassN_picpath = comp.picPath;
    m_compassN_location = comp.location;   
}

void PanoramaWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LEQUAL);

    // 初始化着色器
    setupShaders();

    // 初始化全景图几何体
    createSphereGeometry();

    // 初始化热点几何体
    initHotspotGeometry();

    // 加载全景图纹理
    if (m_texture) {
        delete m_texture;
        m_texture = nullptr;
    }
    m_texture = new QOpenGLTexture(QImage(m_picPath).mirrored());

    // 启动空闲检测定时器
    resetInactivityTimer();
}

void PanoramaWidget::randerHotspotList()
{
    //渲染热点
    for (const auto& hotspot : m_mapPicHotspot[m_picPath])
    {
        if (hotspot.iconID == m_cutHotPoint.iconID) continue;
        if (!hotspot.texture || !hotspot.icon_visible) continue;

        //分组显示
        if (m_GroupStatus == u8"查看" && 
            (m_blGroupShow == true) && (isIconShowGroup(hotspot.iconID) == false)) continue;

        //层次显示      
        if ((m_blLevelShow == true) && (isIconShowLevel(hotspot.iconID) == false)) continue;
        
        m_hotspotProgram->bind();
        m_hotspotVAO.bind();

        //
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

        m_hotspotProgram->setUniformValue("view", m_view);
        m_hotspotProgram->setUniformValue("projection", m_projection);
        m_hotspotProgram->setUniformValue("hotspotTexture", 0);

        QMatrix4x4 tmpModel;
        // ========== 步骤1：仅应用全景旋转（纯旋转，无缩放） ==========
        tmpModel.rotate(m_rotation.x(), 1, 0, 0);
        tmpModel.rotate(m_rotation.y(), 0, 1, 0);
        tmpModel.rotate(m_rotation.z(), 0, 0, 1);
        // ========== 步骤2：平移（带缩放，但仅改位置，不改朝向） ==========
        QVector3D originalPos = hotspot.position.normalized();
        QVector3D translatePos = originalPos;
        tmpModel.translate(translatePos);
        // ========== 步骤3：强制朝向摄像机（仅绕Y轴旋转，禁用Z轴） ==========
        QVector3D cameraPos(0.0f, 0.0f, 1.0f);
        QMatrix4x4 billboardMatrix;
        billboardMatrix.setToIdentity();
        // 核心：提取全景旋转后的摄像机方向
        QVector3D viewDir = (tmpModel.inverted() * cameraPos).normalized();
        QVector3D upDir = (tmpModel.inverted() * QVector3D(0, 1, 0)).normalized();
        // 构建纯朝向矩阵（仅保留Y轴旋转）
        QVector3D forward = viewDir;
        // 修复1：强制forward的Y轴为0，仅保留X/Z平面（禁用Z轴旋转的核心）
        forward.setY(0);
        forward.normalize();
        // 修复2：重新计算right向量，仅基于Y轴对齐的forward
        QVector3D right = QVector3D::crossProduct(QVector3D(0, 1, 0), forward).normalized();
        upDir = QVector3D::crossProduct(forward, right).normalized();
        // 手动构建朝向矩阵（仅保留Y轴旋转分量）
        billboardMatrix.setRow(0, QVector4D(right.x(), upDir.x(), forward.x(), 0));
        billboardMatrix.setRow(1, QVector4D(right.y(), upDir.y(), forward.y(), 0));
        billboardMatrix.setRow(2, QVector4D(right.z(), upDir.z(), forward.z(), 0));
        billboardMatrix.setRow(3, QVector4D(0, 0, 0, 1));
        // 应用朝向
        tmpModel *= billboardMatrix;
        // ========== 步骤4：缩放 ==========
        tmpModel.scale(hotspot.scale_x , hotspot.scale_y);
        m_hotspotProgram->setUniformValue("model", tmpModel);

        glActiveTexture(GL_TEXTURE0);
        hotspot.texture->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        hotspot.texture->release();

        m_hotspotVAO.release();
        m_hotspotProgram->release();

        // 绘制文本     
        if (hotspot.title_visible)
        {
            QPainter painter;
            // 绑定OpenGL上下文与QPainter
            painter.begin(this);
            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            // 计算屏幕坐标
            QVector3D sphericalCoo = QVector3D(tmpModel(0, 3), tmpModel(1, 3), tmpModel(2, 3));

            //if (sphericalCoo.z() - m_cutHotPoint.position.z() > 1.0)
            //{
            //    painter.end();
            //    return;
            //}
            m_cutHpScreenPos = sphereToScreen(sphericalCoo, m_view, m_projection);
            //
            m_mapIconIDUP.insert(std::make_pair(hotspot.iconID, hotspot));

            m_mapAllHpScreenPos[hotspot.iconID] = m_cutHpScreenPos;

            // 设置字体大小
            QFont font("微软雅黑", hotspot.title_text_size);
            painter.setFont(font);
            // 获取文本尺寸
            QString text = hotspot.name;
            QRect textRect = painter.fontMetrics().boundingRect(text);
            // 动态计算文本框高度
            int dynamicHeight = textRect.height() + 10; // 增加边距
            // 调整文本框位置（居中对齐）
            QRect adjustedRect(
                m_cutHpScreenPos.x() - textRect.width() / 2 - 5,  // 水平居中
                m_cutHpScreenPos.y() - 60,     // 垂直居中
                textRect.width() + 10,                             // 动态宽度
                dynamicHeight                                // 动态高度
            );

            painter.setFont(font);
            painter.save();
            painter.setPen(Qt::NoPen);
            QColor bgColor = QColor(hotspot.title_bg_color.red(), hotspot.title_bg_color.green(), hotspot.title_bg_color.blue(), 160);
            painter.setBrush(bgColor);
            painter.drawRoundedRect(adjustedRect, 4, 4);
            painter.setPen(hotspot.title_text_color);
            painter.drawText(adjustedRect, Qt::AlignCenter, hotspot.name);
            painter.restore();
            painter.end();
        }
    }
}

void PanoramaWidget::randerHotspot_tmp()
{
    //渲染临时热点
    auto& hotspot = m_cutHotPoint;
    {
        if (!hotspot.texture || !hotspot.icon_visible) return;

        m_hotspotProgram->bind();
        m_hotspotVAO.bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        m_hotspotProgram->setUniformValue("view", m_view);
        m_hotspotProgram->setUniformValue("projection", m_projection);
        m_hotspotProgram->setUniformValue("hotspotTexture", 0);

        QMatrix4x4 tmpModel;
        // ========== 步骤1：仅应用全景旋转（纯旋转，无缩放） ==========
        tmpModel.rotate(m_rotation.x(), 1, 0, 0);
        tmpModel.rotate(m_rotation.y(), 0, 1, 0);
        tmpModel.rotate(m_rotation.z(), 0, 0, 1);
        // ========== 步骤2：平移（带缩放，但仅改位置，不改朝向） ==========
        QVector3D originalPos = hotspot.position.normalized();
        QVector3D translatePos = originalPos /** m_zoom*/;
        tmpModel.translate(translatePos);
        // ========== 步骤3：强制朝向摄像机（仅绕Y轴旋转，禁用Z轴） ==========
        QVector3D cameraPos(0.0f, 0.0f, 1.0f);
        QMatrix4x4 billboardMatrix;
        billboardMatrix.setToIdentity();
        // 核心：提取全景旋转后的摄像机方向
        QVector3D viewDir = (tmpModel.inverted() * cameraPos).normalized();
        QVector3D upDir = (tmpModel.inverted() * QVector3D(0, 1, 0)).normalized();
        // 构建纯朝向矩阵（仅保留Y轴旋转）
        QVector3D forward = viewDir;
        // 修复1：强制forward的Y轴为0，仅保留X/Z平面（禁用Z轴旋转的核心）
        forward.setY(0);
        forward.normalize();
        // 修复2：重新计算right向量，仅基于Y轴对齐的forward
        QVector3D right = QVector3D::crossProduct(QVector3D(0, 1, 0), forward).normalized();
        upDir = QVector3D::crossProduct(forward, right).normalized();
        // 手动构建朝向矩阵（仅保留Y轴旋转分量）
        billboardMatrix.setRow(0, QVector4D(right.x(), upDir.x(), forward.x(), 0));
        billboardMatrix.setRow(1, QVector4D(right.y(), upDir.y(), forward.y(), 0));
        billboardMatrix.setRow(2, QVector4D(right.z(), upDir.z(), forward.z(), 0));
        billboardMatrix.setRow(3, QVector4D(0, 0, 0, 1));
        // 应用朝向
        tmpModel *= billboardMatrix;
        // ========== 步骤4：缩放 ==========
        tmpModel.scale(hotspot.scale_x , hotspot.scale_y);
        m_hotspotProgram->setUniformValue("model", tmpModel);

        glActiveTexture(GL_TEXTURE0);
        hotspot.texture->bind();
        glDrawArrays(GL_TRIANGLES, 0, 6);
        hotspot.texture->release();

        m_hotspotVAO.release();
        m_hotspotProgram->release();

        // 绘制文本     
        if (hotspot.title_visible)
        {
            QPainter painter;
            // 绑定OpenGL上下文与QPainter
            painter.begin(this);
            painter.setRenderHint(QPainter::HighQualityAntialiasing);
            painter.setRenderHint(QPainter::TextAntialiasing);
            // 计算屏幕坐标
            QVector3D sphericalCoo = QVector3D(tmpModel(0, 3), tmpModel(1, 3), tmpModel(2, 3));  
            if (sphericalCoo.z() - m_cutHotPoint.position.z() > 1.0)
            {
                painter.end();
                return;
            }
            m_cutHpScreenPos = sphereToScreen(sphericalCoo, m_view, m_projection);
            // 设置字体大小
            QFont font("微软雅黑", hotspot.title_text_size);
            painter.setFont(font);
            // 获取文本尺寸
            QString text = hotspot.name;
            QRect textRect = painter.fontMetrics().boundingRect(text);
            // 动态计算文本框高度
            int dynamicHeight = textRect.height() + 10; // 增加边距
            // 调整文本框位置（居中对齐）
            QRect adjustedRect(
                m_cutHpScreenPos.x() - textRect.width() / 2 - 5,  // 水平居中
                m_cutHpScreenPos.y() - 60 ,     // 垂直居中
                textRect.width() + 10,                             // 动态宽度
                dynamicHeight                                // 动态高度
            );

            painter.setFont(font);
            painter.save();
            painter.setPen(Qt::NoPen);
            QColor bgColor = QColor(hotspot.title_bg_color.red(), hotspot.title_bg_color.green(), hotspot.title_bg_color.blue(), 160);
            painter.setBrush(bgColor);
            painter.drawRoundedRect(adjustedRect, 4, 4);
            painter.setPen(hotspot.title_text_color);
            painter.drawText(adjustedRect, Qt::AlignCenter, hotspot.name);
            painter.restore();
            painter.end();
        }
    }
}

QVector3D PanoramaWidget::getRotatedPosition()
{
    // 原始向量（单位球后方向量）
    QVector3D originalVec(0.0f, 0.0f, -1.0f);

    // 构建旋转矩阵（旋转顺序：x→y→z，角度转换为弧度）
    QMatrix4x4 rotMatrix;
    rotMatrix.rotate(m_rotation.x(), 1, 0, 0); // 绕x轴旋转（角度制）
    rotMatrix.rotate(m_rotation.y(), 0, 1, 0); // 绕y轴旋转（角度制）
    rotMatrix.rotate(m_rotation.z(), 0, 0, 1); // 绕z轴旋转（角度制）

    // 将向量与旋转矩阵相乘（矩阵左乘向量）
    QVector4D vec4(originalVec, 1.0f); // 齐次坐标
    QVector4D rotatedVec4 = rotMatrix * vec4;
    QVector3D rotatedVec(rotatedVec4.x(), rotatedVec4.y(), rotatedVec4.z());

    // 归一化（若旋转矩阵正交，可省略，此处用于精度修正）
    rotatedVec.normalize();

    QVector3D rotatedVec_tmp(-rotatedVec.x(), -rotatedVec.y(), rotatedVec.z());
    return rotatedVec_tmp;
}

void PanoramaWidget::loadPanorama(const QString& imagePath)
{
    makeCurrent();
    m_picPath = imagePath;
    QImage image(imagePath);
    if (image.isNull()) 
    {
        qDebug() << "Failed to load image:" << imagePath;
        return;
    }
    if (m_texture) 
    {
        delete m_texture;
        m_texture = nullptr;
    }
    m_texture = new QOpenGLTexture(image.mirrored());
    m_texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
    m_texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_texture->setWrapMode(QOpenGLTexture::Repeat);

    doneCurrent();
    update();
}

void PanoramaWidget::startAutoRotation()
{
    if (!m_autoRotate) {
        m_autoRotate = true;
        emit sigShowElement(false);
        m_animationTimer->start(8); // ~60 FPS
    }
}

void PanoramaWidget::stopAutoRotation()
{
    m_autoRotate = false;
    m_animationTimer->stop();
}

void PanoramaWidget::setAutoRotationSpeed(float speed)
{
    m_rotationSpeed = qMax(0.1f, qMin(speed, 2.0f));
}

void PanoramaWidget::onAnimationTimer()
{
    if (m_autoRotate) 
    {
        float tmp = m_rotation.y() + m_rotationSpeed;
        if (tmp > 360)
               tmp = 0;
        m_rotation.setY(tmp);
        update();
    }
}

void PanoramaWidget::resetInactivityTimer()
{
    stopAutoRotation();
    m_inactivityTimer->start(m_inactivityDelay);
}

// 渲染函数
void PanoramaWidget::paintGL()
{  
    if (!m_program || !m_texture) return;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // --------渲染全景图片--------
    m_program->bind();

    QMatrix4x4 model;
    model.rotate(m_rotation.x(), 1, 0, 0);
    model.rotate(m_rotation.y(), 0, 1, 0);
    model.rotate(m_rotation.z(), 0, 0, 1);
    m_program->setUniformValue("model", model);
    //
    m_view.setToIdentity();
    m_view.lookAt(
        QVector3D(0.0f, 0.0f, 1.0f),  // 摄像机位置
        QVector3D(0.0f, 0.0f, 0.0f),  // 观察目标
        QVector3D(0.0f, 1.0f, 0.0f)   // 上方向

    );
    m_program->setUniformValue("view", m_view);
    //
    m_projection.setToIdentity();
    m_projection.perspective(60.0/m_zoom, static_cast<float>(width()) / height(), 0.1f, 100.0f);
    m_program->setUniformValue("projection", m_projection);

    // 传递你的原有矩形裁剪参数（保留，不影响圆形裁剪）
    m_program->setUniformValue("regionValid", m_MapRegion_LLClip[m_picPath].Region_valid);
    m_program->setUniformValue("thetaMin", m_MapRegion_LLClip[m_picPath].Region_thetaMin);
    m_program->setUniformValue("thetaMax", m_MapRegion_LLClip[m_picPath].Region_thetaMax);
    m_program->setUniformValue("phiMin", m_MapRegion_LLClip[m_picPath].Region_phiMin);
    m_program->setUniformValue("phiMax", m_MapRegion_LLClip[m_picPath].Region_phiMax);
    //

    if (m_isTransitioning && m_nextTexture)
    {
        // 过渡渲染：混合当前纹理和下一张纹理
        m_program->setUniformValue("transitionProgress", m_transitionProgress);
        // 绑定当前纹理到纹理单元0
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        m_program->setUniformValue("texture1", 0);
        // 绑定下一张纹理到纹理单元1
        glActiveTexture(GL_TEXTURE1);
        m_nextTexture->bind();
        m_program->setUniformValue("texture2", 1);
    }
    else
    {
        // 正常渲染
        m_program->setUniformValue("transitionProgress", 0.0f);
        glActiveTexture(GL_TEXTURE0);
        m_texture->bind();
        m_program->setUniformValue("texture1", 0);
    }
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_sphereStacks * m_sphereSlices * 6, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
    m_program->release();

    // --------渲染热点 --------
    if (m_blHotspotShow == true)
    {
        randerHotspotList();
        randerHotspot_tmp();
    } 
    // --------渲染标尺 --------
    {
 
    }  
    // --------指北针-----------
    if (m_blCompassShow == true)
    {
        //指北针位置
        if (m_compassN_location == "LeftTop")
            m_compassLabel->move(20, 20);
        else if(m_compassN_location == "BottomCenter")
			m_compassLabel->move((width() - m_compassLabel->width()) / 2, height() - m_compassLabel->height() - 20);
        else if (m_compassN_location == "RightTop") {
            int x = width() - m_compassLabel->width() - 20;
            int y = 20;
            m_compassLabel->move(x, y);
        } 

        //默认指北针图片
        if (m_compassN_picpath == "") 
            m_compassN_picpath = QApplication::applicationDirPath() + "/Resource/compass/compass1.png";
        float angle = m_compassN - m_rotation.y();  //计算指北针旋转角度：指北针的显示角度 = 指北针原始朝向（m_compassN） - 全景相机水平旋转（m_rotation.y()）
        QPixmap originalPixmap = QPixmap(m_compassN_picpath);
        m_compassLabel->setPixmap(originalPixmap);
		m_compassLabel->setRotationAngle(angle);  // 设置旋转角度
    }
}

// 过渡进度属性设置器
void PanoramaWidget::setTransitionProgress(float progress)
{
    m_transitionProgress = progress;

    if (m_transitionProgress >= 1.0f) {
        // 过渡完成，切换到新纹理
        m_isTransitioning = false;
        if (m_texture) {
            delete m_texture;
        }
        m_texture = m_nextTexture;
        m_nextTexture = nullptr;
    }
    update();
    emit sig_transEnd();    
}

float PanoramaWidget::getTransitionProgress() const
{
    return m_transitionProgress;
}


void PanoramaWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void PanoramaWidget::InsertHPToDB()
{
    //热点写入数据库
    QString position = QString::number(m_cutHotPoint.position.x(), 10, 4) + \
                "," + QString::number(m_cutHotPoint.position.y(), 10, 4) + \
                "," + QString::number(m_cutHotPoint.position.z(), 10, 4);
    bool isLock = m_cutHotPoint.lock;

    QString title_bg_color = QString::number(m_cutHotPoint.title_bg_color.red()) + ","
        + QString::number(m_cutHotPoint.title_bg_color.green()) + ","
        + QString::number(m_cutHotPoint.title_bg_color.blue());
    QString title_text_color = QString::number(m_cutHotPoint.title_text_color.red()) + ","
        + QString::number(m_cutHotPoint.title_text_color.green()) + ","
        + QString::number(m_cutHotPoint.title_text_color.blue());
    QString title_text_size = QString::number(m_cutHotPoint.title_text_size);
    //
    QString sql = QString("INSERT INTO hotpoint( \
				id, \
				hotpoint_name, \
				picture_id, \
				hotpoint_path, \
				style,\
                position,\
				w_zoom,\
				h_zoom,\
				isLock,\
				isIconShow,\
				isTitleShow,\
				isFollowSysZoom,\
                project_id,\
                title_bg_color,\
                title_text_color,\
                title_text_size\
			) VALUES (\
				'%1',\
				'%2', \
				'%3', \
				'%4', \
				'%5', \
				'%6', \
				'%7', \
				'%8', \
				'%9', \
				'%10',\
                '%11',\
                '%12',\
				'%13', \
                '%14', \
                '%15', \
                '%16' \
			); ")
        .arg(m_cutHotPoint.iconID)
        .arg(m_cutHotPoint.name)
        .arg(m_picID)
        .arg(m_cutHotPoint.iconPath)
        .arg(m_cutHotPoint.style)
        .arg(position)
        .arg(m_cutHotPoint.scale_x)
        .arg(m_cutHotPoint.scale_y)
        .arg(isLock)
        .arg(m_cutHotPoint.icon_visible)
        .arg(m_cutHotPoint.title_visible)
        .arg(m_cutHotPoint.isFollowSysZoom)
        .arg(ProjectInfo::Ins().m_projUUID)
        .arg(title_bg_color)
        .arg(title_text_color)
        .arg(title_text_size);
    //执行语句
    QSqlQuery query;
    if (!query.exec(sql))
    {
        qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
    }
}

void PanoramaWidget::UpdateHPToDB()
{
    //热点更新数据库
    QString position = QString::number(m_cutHotPoint.position.x(), 10, 4) + \
        "," + QString::number(m_cutHotPoint.position.y(), 10, 4) + \
        "," + QString::number(m_cutHotPoint.position.z(), 10, 4);
    bool isLock = m_cutHotPoint.lock;

    QString title_bg_color = QString::number(m_cutHotPoint.title_bg_color.red()) + ","
        + QString::number(m_cutHotPoint.title_bg_color.green()) + ","
        + QString::number(m_cutHotPoint.title_bg_color.blue());
    QString title_text_color = QString::number(m_cutHotPoint.title_text_color.red()) + ","
        + QString::number(m_cutHotPoint.title_text_color.green()) + ","
        + QString::number(m_cutHotPoint.title_text_color.blue());
    QString title_text_size = QString::number(m_cutHotPoint.title_text_size);
    //
    QString sql = QString("UPDATE hotpoint set \
				        hotpoint_name = '%1', \
				        hotpoint_path = '%2', \
				        style = '%3',\
                        position = '%4',\
				        w_zoom = '%5',\
				        h_zoom = '%6',\
				        isLock = '%7',\
				        isIconShow = '%8',\
				        isTitleShow = '%9',\
				        isFollowSysZoom = '%10',\
                        title_bg_color = '%11',\
                        title_text_color = '%12',\
                        title_text_size = '%13' \
                        where id = '%14'")
                        .arg(m_cutHotPoint.name)
                        .arg(m_cutHotPoint.iconPath)
                        .arg(m_cutHotPoint.style)
                        .arg(position)
                        .arg(m_cutHotPoint.scale_x)
                        .arg(m_cutHotPoint.scale_y)
                        .arg(isLock)
                        .arg(m_cutHotPoint.icon_visible)
                        .arg(m_cutHotPoint.title_visible)
                        .arg(m_cutHotPoint.isFollowSysZoom)
                        .arg(title_bg_color)
                        .arg(title_text_color)
                        .arg(title_text_size)
                        .arg(m_cutHotPoint.iconID);
    //执行语句
    QSqlQuery query;
    if (!query.exec(sql))
    {
        qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
    }
    //更新修改时间
    {
        QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString updata = QString("update project set update_time = '%1' where id = (SELECT project_id FROM hotpoint WHERE id = '%2')").arg(update_time).arg(m_cutHotPoint.iconID);
        QSqlQuery query;
        query.exec(updata);
    }
}

static bool isMatrixInvertible(const QMatrix4x4& mat) {
    const float epsilon = 1e-8f; // 浮点精度容错
    return qAbs(mat.determinant()) > epsilon;
}


QVector3D PanoramaWidget::screenToSphere(const QPoint& mousePos)
{
        // ========== 1. 基础参数（物理坐标） ==========
     int viewportWidth = width();
     int viewportHeight = height();
     float mouseX = static_cast<float>(mousePos.x());
     float mouseY = static_cast<float>(mousePos.y());
 
     // ========== 2. 屏幕→NDC ==========
     float ndcX = (2.0f * mouseX / viewportWidth) - 1.0f;
     float ndcY = (2.0f * mouseY / viewportHeight) - 1.0f;
     QVector4D ndcPos(ndcX, ndcY, -1.0f, 1.0f);
 
     // ========== 3. NDC→视图空间射线==========
     QMatrix4x4 invProjection = m_projection.inverted();
     if (!isMatrixInvertible(invProjection)) {
         qWarning() << "Projection matrix is not invertible!";
         return QVector3D(0, 0, -1);
     }
     QVector4D viewRay = invProjection * ndcPos;
     viewRay.setW(1.0f);
     viewRay /= viewRay.w();
     viewRay.setW(0.0f);    // 转为方向向量
 
     // ========== 4. 视图空间→初始世界空间（无旋转） ==========
     QMatrix4x4 viewMatrix;
     viewMatrix.lookAt(QVector3D(0.0f, 0.0f, 1.0f),
         QVector3D(0.0f, 0.0f, 0.0f),
         QVector3D(0.0f, 1.0f, 0.0f));
     QVector4D initialWorldRay = viewMatrix.inverted() * viewRay;
     QVector3D initialRayDir = QVector3D(initialWorldRay.x(), initialWorldRay.y(), initialWorldRay.z()).normalized();
 
     // ========== 5. 射线求交：无旋转的缩放球面 ==========
     QVector3D rayOrigin(0.0f, 0.0f, 1.0f);
     float sphereRadius = 1.0;
     QVector3D oc = rayOrigin - QVector3D(0, 0, 0);
     float a = QVector3D::dotProduct(initialRayDir, initialRayDir);
     float b = 2 * QVector3D::dotProduct(oc, initialRayDir);
     float c = QVector3D::dotProduct(oc, oc) - sphereRadius * sphereRadius;
     float discriminant = b * b - 4 * a * c;
 
     if (discriminant < 0) {
         return QVector3D(0, 0, -1).normalized();
     }
 
     float sqrtDisc = qSqrt(discriminant);
     float t1 = (-b - sqrtDisc) / (2 * a); // 近交点（摄像机前方的有效交点）
     float t2 = (-b + sqrtDisc) / (2 * a); // 远交点（球背面，无效）
     // 原错误：t1 > 1e-6 阈值过大，改为t1 > 0（确保在摄像机前方）
     float t = (t1 > 0) ? t1 : t2;
 
     // 无旋转时的球面坐标
     QVector3D initialSpherePos = (rayOrigin + initialRayDir * t).normalized();
 
     // ========== 6. 核心：绝对欧拉角旋转（修复X+Y同时旋转） ==========
     // 步骤1：将角度转为弧度
     float rx = qDegreesToRadians(m_rotation.x()); // X轴旋转角度（弧度）
     float ry = qDegreesToRadians(-m_rotation.y()); // Y轴旋转角度（弧度）
 
     // 步骤2：构建绕世界X轴的旋转矩阵（固定轴）
     QMatrix4x4 rotX;
     rotX.setToIdentity();
     rotX(1, 1) = cos(rx); rotX(1, 2) = -sin(rx);
     rotX(2, 1) = sin(rx); rotX(2, 2) = cos(rx);
 
     // 步骤3：构建绕世界Y轴的旋转矩阵（固定轴）
     QMatrix4x4 rotY;
     rotY.setToIdentity();
     rotY(0, 0) = cos(ry); rotY(0, 2) = sin(ry);
     rotY(2, 0) = -sin(ry); rotY(2, 2) = cos(ry);
 
     // 步骤4：合并旋转矩阵（核心修复：先Y后X → 改为 rotY * rotX，匹配操作逻辑）
     QMatrix4x4 rotMatrix = rotY * rotX;
 
     // 步骤5：应用旋转到球面坐标
     QVector4D rotatedPos = rotMatrix * QVector4D(initialSpherePos, 1.0f);
     QVector3D finalSpherePos = QVector3D(rotatedPos.x(), -rotatedPos.y(), rotatedPos.z()).normalized();

     return finalSpherePos;
}

QPoint PanoramaWidget::sphereToScreen(const QVector3D& worldPos, const QMatrix4x4& view, const QMatrix4x4& projection)
{
// 1. 构建MVP矩阵（Model已合并到worldPos，故仅Projection×View）
    QMatrix4x4 mvp = projection * view;
    
    // 2. 转换为齐次坐标并计算裁剪空间坐标
    QVector4D clipPos = mvp * QVector4D(worldPos.x(), worldPos.y(), worldPos.z(), 1.0f);
    
    // 防护：避免透视除法除以0
    if (qFuzzyIsNull(clipPos.w())) {
        qWarning() << "clipPos.w()为0，无法计算屏幕坐标";
        return QPoint(0, 0);
    }
    
    // 3. 透视除法 → 归一化设备坐标（NDC: [-1,1]）
    float ndcX = clipPos.x() / clipPos.w();
    float ndcY = clipPos.y() / clipPos.w();
    
    // 4. NDC → 屏幕像素坐标（适配Qt Y轴向下）
    int screenX = qRound((ndcX + 1.0f) * width() / 2.0f);
    int screenY = qRound((1.0f - ndcY) * height() / 2.0f);
    
    return QPoint(screenX, screenY);
}

bool PanoramaWidget::getHotspotAtPosition(const QPoint& mousePos)
{
    QVector2D screenPoint(mousePos.x(), mousePos.y());
    if (!m_cutHotPoint.icon_visible) return false;
    return isPointNearHotspot(screenPoint, m_cutHotPoint.position);

}
void PanoramaWidget::setHotspotPosition(int index, const QVector3D& position)
{
    if (index >= 0 && index < m_mapPicHotspot[m_picPath].size()) {
        m_mapPicHotspot[m_picPath][index].position = position;
        update();
    }
}

bool PanoramaWidget::isPointNearHotspot(const QVector2D& screenPoint, const QVector3D& hotspotPos)
{    
    QPoint hpPoint = sphereToScreen(hotspotPos, m_view, m_projection);

    QVector2D hp2D = QVector2D(hpPoint.x(), hpPoint.y());

    // 考虑缩放的阈值
    float scaledThreshold = 20.0f;
    float distance = (screenPoint - hp2D).length();
    return distance < scaledThreshold;
}

QVector3D PanoramaWidget::getHotspotPosition(int index) const
{
   /* if (index >= 0 && index < m_mapPicHotspot[m_picPath].size()) 
    {
        return m_mapPicHotspot[m_picPath][index].position;
    }*/
    return QVector3D();
}

void PanoramaWidget::setupShaders()
{
     // 全景图着色器
    m_program = new QOpenGLShaderProgram(this);
    //
    //const char* panoramaVShader =
    //    "#version 330 core\n"
    //    "layout (location = 0) in vec3 aPos;\n"
    //    "layout (location = 1) in vec2 aTexCoord;\n"
    //    "out vec2 TexCoord;\n"
    //    "uniform mat4 projection;\n"
    //    "uniform mat4 view;\n"
    //    "uniform mat4 model;\n"
    //    "void main()\n"
    //    "{\n"
    //    "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    //    "    TexCoord = aTexCoord;\n"
    //    "}\0";

    // //片段着色器 - 支持过渡效果
    //const char* panoramaFShader =
    //    "#version 330 core\n"
    //    "in vec2 TexCoord;\n"
    //    "out vec4 FragColor;\n"
    //    "uniform sampler2D texture1;\n"
    //    "uniform sampler2D texture2;\n"
    //    "uniform float transitionProgress;\n"
    //    "void main()\n"
    //    "{\n"
    //    "    vec4 color1 = texture(texture1, TexCoord);\n"
    //    "    vec4 color2 = texture(texture2, TexCoord);\n"
    //    "    FragColor = mix(color1, color2, transitionProgress);\n"
    //    "}\n";

    const char* panoramaVShader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "out vec3 SphereLocalPos; // 新增：传递球体局部笛卡尔坐标到片段着色器\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "    SphereLocalPos = aPos; // 球体局部坐标（未经过视图/投影变换，固定不变）\n"
        "}\0";

    // 片段着色器（保留原有功能 + 新增圆形裁剪逻辑）
    const char* panoramaFShader =
        "#version 330 core\n"
        "in vec2 TexCoord;\n"
        "in vec3 SphereLocalPos; // 接收球体局部笛卡尔坐标\n"
        "out vec4 FragColor;\n"
        "uniform sampler2D texture1;\n"
        "uniform sampler2D texture2;\n"
        "uniform float transitionProgress;\n"
        "// 原有：固定球面区域（矩形裁剪）参数\n"
        "uniform bool regionValid;\n"
        "uniform float thetaMin;\n"
        "uniform float thetaMax;\n"
        "uniform float phiMin;\n"
        "uniform float phiMax;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // 步骤1：将球体局部笛卡尔坐标转换为球面角度（theta：方位角，phi：极角）\n"
        "    float theta = atan(SphereLocalPos.z, SphereLocalPos.x) * 180.0 / 3.1415926 + 180.0;\n"
        "    float phi = acos(clamp(SphereLocalPos.y, -1.0, 1.0)) * 180.0 / 3.1415926;\n"
        "\n"
        "    // 步骤2：裁剪判断：仅在固定区域内绘制纹理，否则丢弃片段\n"
        "    if (regionValid) {\n"
        "        bool keepFragment = false; // 是否保留当前片段（不丢弃）\n"
        "        // ========== 矩形裁剪逻辑（备用） ==========\n"
        "       if (theta >= thetaMin && theta <= thetaMax && phi >= phiMin && phi <= phiMax) {\n"
        "            keepFragment = true;\n"
        "        }\n"
        "\n"
        "        // 不满足裁剪条件则丢弃片段\n"
        "        if (!keepFragment) {\n"
        "            discard; // 丢弃该片段，不绘制任何内容\n"
        "        }\n"
        "    }\n"
        "\n"
        "    // 步骤3：原有纹理混合逻辑（不变）\n"
        "    vec4 color1 = texture(texture1, TexCoord);\n"
        "    vec4 color2 = texture(texture2, TexCoord);\n"
        "    FragColor = mix(color1, color2, transitionProgress);\n"
        "}\n";

    m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, panoramaVShader);
    m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, panoramaFShader);
    m_program->link();

    
    // 热点着色器
    m_hotspotProgram = new QOpenGLShaderProgram(this);
    const char* hotspotVShader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec2 aTexCoord;\n"
        "out vec2 TexCoord;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "    TexCoord = aTexCoord;\n"
        "}\0";

    const char* hotspotFShader =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec2 TexCoord;\n"
        "uniform sampler2D hotspotTexture;\n"
        "void main()\n"
        "{\n"
        "    FragColor = texture(hotspotTexture, TexCoord);\n"
        "}\0";

    m_hotspotProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, hotspotVShader);
    m_hotspotProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, hotspotFShader);
    m_hotspotProgram->link();

    // 标尺（形状）着色器：绘制纯色圆点和线段
    m_shapeProgram = new QOpenGLShaderProgram(this);
    const char* shapeVShader =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "uniform mat4 projection;\n"
        "uniform mat4 view;\n"
        "uniform mat4 model;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "}\0";

    const char* shapeFShader =
        "#version 330 core\n"
        "out vec4 FragColor;\n"
        "uniform vec3 shapeColor;\n"
        "void main()\n"
        "{\n"
        "    FragColor = vec4(shapeColor, 1.0f);\n"
        "}\0";

    m_shapeProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, shapeVShader);
    m_shapeProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, shapeFShader);
    m_shapeProgram->link();
}

void PanoramaWidget::zoomIn()
{
    m_zoom *= 1.2f;
    m_zoom = qMin(m_zoom, 5.0f);
    update();
}

void PanoramaWidget::zoomOut()
{
    m_zoom /= 1.2f;
    m_zoom = qMax(0.1f, m_zoom);
    update();
}

void PanoramaWidget::setZoom(float zoomLevel)
{
    m_zoom = qMax(0.1f, qMin(zoomLevel, 5.0f));
    update();
}

void PanoramaWidget::setHotspotMovable(int index, bool movable)
{
    if (index >= 0 && index < m_mapPicHotspot[m_picPath].size()) {
        m_mapPicHotspot[m_picPath][index].movable = movable;
    }
}

void PanoramaWidget::enableHotspotMoving(bool enabled)
{
    m_hotspotMovingEnabled = enabled;
}

// 添加热点
void PanoramaWidget::addHotspot(Hotspot hp)
{
    m_cutHotPoint.iconID = hp.iconID;
    m_cutHotPoint.position = sphericalToCartesian(m_rotation.x(), m_rotation.y());
    m_cutHotPoint.iconPath = hp.iconPath;
    m_cutHotPoint.name = hp.name;
    m_cutHotPoint.description = hp.description;
    m_cutHotPoint.style = hp.style;
    m_cutHotPoint.scale_x = hp.scale_x;
    m_cutHotPoint.scale_y = hp.scale_y;
    m_cutHotPoint.lock = hp.lock;
    m_cutHotPoint.icon_visible = hp.icon_visible;
    m_cutHotPoint.title_visible = hp.title_visible;
    m_cutHotPoint.movable = hp.movable;
    m_cutHotPoint.selected = hp.selected;
    m_cutHotPoint.isFollowSysZoom = hp.isFollowSysZoom;

    m_cutHotPoint.title_bg_color = hp.title_bg_color;
    m_cutHotPoint.title_text_color = hp.title_text_color;
    m_cutHotPoint.title_text_size = hp.title_text_size;

    makeCurrent();
    QImage img(hp.iconPath);
    if (img.isNull()) return;
    QImage textureImg = img.convertToFormat(QImage::Format_RGBA8888);
    textureImg = textureImg.mirrored(false, true); 
    m_cutHotPoint.texture = new QOpenGLTexture(textureImg);
    m_cutHotPoint.texture->setMinificationFilter(QOpenGLTexture::Linear);
    m_cutHotPoint.texture->setMagnificationFilter(QOpenGLTexture::Linear);
    m_cutHotPoint.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);

    update();
    doneCurrent();
    paintGL();
}

void PanoramaWidget::addHotspot2List(Hotspot hp)
{
    Hotspot tmpHp = hp;
    //
    makeCurrent();
    QImage img(hp.iconPath);
    if (img.isNull()) return;
    QImage textureImg = img.convertToFormat(QImage::Format_RGBA8888);
    textureImg = textureImg.mirrored(false, true);
    tmpHp.texture = new QOpenGLTexture(textureImg);
    tmpHp.texture->setMinificationFilter(QOpenGLTexture::Linear);
    tmpHp.texture->setMagnificationFilter(QOpenGLTexture::Linear);
    tmpHp.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
    doneCurrent();
    //
    m_mapPicHotspot[m_picPath].append(tmpHp);
    
    update();
}

void PanoramaWidget::removeHotspot(int index)
{
    if (index >= 0 && index < m_mapPicHotspot[m_picPath].size()) {
        m_mapPicHotspot[m_picPath].removeAt(index);
        update();
    }
}

void PanoramaWidget::setHotspotVisible(int index, bool visible)
{
    if (index >= 0 && index < m_mapPicHotspot[m_picPath].size()) {
        m_mapPicHotspot[m_picPath][index].icon_visible = visible;
        update();
    }
}

int PanoramaWidget::getHotspotCount() 
{
    return m_mapPicHotspot[m_picPath].size();
}

void PanoramaWidget::checkHotspotClick(const QPoint& mousePos)
{
    // 将屏幕坐标转换为球面坐标进行热点检测
    float x = (2.0f * mousePos.x()) / width() - 1.0f;
    float y = 1.0f - (2.0f * mousePos.y()) / height();

    // 简化版热点碰撞检测
    for (int i = 0; i < m_mapPicHotspot[m_picPath].size(); ++i) {
        Hotspot hotspot = m_mapPicHotspot[m_picPath][i];
        if (hotspot.icon_visible && isPointNearHotspot(QVector2D(x, y), hotspot.position)) {
            emit hotspotClicked(i, hotspot.name);
            break;
        }
    }
}

void PanoramaWidget::wheelEvent(QWheelEvent* event)
{
    m_autoRotate = false;

    // 定义缩放参数
    const float ZOOM_FACTOR = 1.1f;

    // 获取滚轮增量
    float delta = event->angleDelta().y() / 120.0f;
    // 计算缩放倍数
    float zoomMultiplier = (delta > 0) ? ZOOM_FACTOR : 1.0f / ZOOM_FACTOR;
    // 应用缩放
    float newZoom = m_zoom * zoomMultiplier;
    // 应用缩放限制
    m_zoom_min = m_mapPicAngle[m_picPath].zoom_range.split(';')[0].toFloat();
    m_zoom_max = m_mapPicAngle[m_picPath].zoom_range.split(';')[1].toFloat();
    newZoom = qMax(m_zoom_min, qMin(newZoom, m_zoom_max));
     
    // 只有当缩放值发生变化时才更新
    if (fabs(newZoom - m_zoom) > 0.001) 
    {
        m_zoom = newZoom;
        //场景视角
        emit sig_seeAngle(m_fov / m_zoom);
        emit sig_cutSeeAngle(QString::number(m_fov / m_zoom,10,1));
        update(); 
    }
}

void PanoramaWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        m_lastMousePos = event->pos();

        float distance = QVector2D(m_lastMousePos - m_cutHpScreenPos).length();
        if (distance < 20)
        {
            m_isMovingHotspot = true;
        }
        else
        {
            m_isDragging = true;
            m_isMovingHotspot = false;
        }
        //图文、场景切换 
        if (m_cutHotPoint.iconID != 0) return;
        for (auto tmpHp : m_mapAllHpScreenPos)
        {
            float distance = QVector2D(m_lastMousePos - tmpHp.second).length();
            if (distance < 20)
            {
                if (m_mapIconIDUP[tmpHp.first].style.contains("picText;") == true)
                {
                    QStringList textList = m_mapIconIDUP[tmpHp.first].style.split(';');
                    if (textList.size() == 2 && textList[1] != "")
                    {
                        emit sig_style_picText(tmpHp.first + ";" + textList[1]);
                        break;
                    }             
                }
                else if (m_mapIconIDUP[tmpHp.first].style.contains("picChange;") == true)
                {
                    QStringList picChangeList = m_mapIconIDUP[tmpHp.first].style.split(';');
                    if (picChangeList.size() == 2 && picChangeList[1] != "")
                    {
                        emit sig_style_picChange(m_mapIconIDUP[tmpHp.first].style);
                        break;
                    }                    
                }
            }   
        }
    }
}

void PanoramaWidget::mouseMoveEvent(QMouseEvent* event)
{
    m_autoRotate = false;
    emit sigShowElement(true);
    QPoint delta = event->pos() - m_lastMousePos;
    if (event->buttons() & Qt::LeftButton)
    {        
        //热点
        if (m_isMovingHotspot && m_cutHotPoint.lock == false)
        {
            // 移动热点
            QVector3D newPosition = screenToSphere(event->pos());
            m_cutHotPoint.position = newPosition;
            update();
        }
        else if (m_isDragging)
        {
            QPoint delta = event->pos() - m_lastMousePos;

            float tmp_x = m_rotation.x() - delta.y() * 0.05f;
            float tmp_y = m_rotation.y() - delta.x() * 0.05f;

            float newX, newY;
          
            m_H_min = -360;
            m_H_max = 360;
            m_V_min = -90;
            m_V_max = 90;

            newX = tmp_x;
            newY = tmp_y;
                    
            if (m_GroupStatus == u8"查看")//编辑状态不设定局限
            {
                if (m_mapPicAngle[m_picPath].h_range.split(';').size() == 2 && m_mapPicAngle[m_picPath].v_range.split(';').size() == 2)
                {
                    m_H_min = m_mapPicAngle[m_picPath].h_range.split(';')[0].toFloat();
                    m_H_max = m_mapPicAngle[m_picPath].h_range.split(';')[1].toFloat();
                    m_V_min = m_mapPicAngle[m_picPath].v_range.split(';')[0].toFloat();
                    m_V_max = m_mapPicAngle[m_picPath].v_range.split(';')[1].toFloat();
                }
                newX = qMax(m_V_min, qMin(tmp_x, m_V_max));
                newY = qMax(m_H_min, qMin(tmp_y, m_H_max));
            } 
            //
            m_rotation.setX(newX);
            m_rotation.setY(newY);

            m_lastMousePos = event->pos();
            update();
        }
        resetInactivityTimer();
    }
}

void PanoramaWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) 
    {   
        // 标尺编辑模式核心逻辑
        if (m_edit_ruler)
        {
           
        }


        //热点/场景逻辑
        QPoint releasePos = event->pos();
        if (m_cutHotPoint.iconID != 0) return;
        for (auto tmpHp : m_mapAllHpScreenPos)
        {
            float distance = QVector2D(releasePos - tmpHp.second).length();
            if (distance < 20)
            {
                if (m_mapIconIDUP[tmpHp.first].style.contains("picText;") == true)
                {
                    QStringList textList = m_mapIconIDUP[tmpHp.first].style.split(';');
                    if (textList.size() == 2)
                    {
                        emit sig_style_picText(tmpHp.first + ";" + textList[1]);
                        break;
                    }
                }
                else if (m_mapIconIDUP[tmpHp.first].style.contains("picChange;") == true)
                {
                    emit sig_style_picChange(m_mapIconIDUP[tmpHp.first].style);
                    break;
                }
            }
        }

        // 重置拖拽/热点移动状态
        m_isDragging = false;
        m_isMovingHotspot = false;

    }
}

void PanoramaWidget::setCompassLocation(const QString& location)
{
    m_compassN_location = location; 

    // 立即更新 compassLabel 的位置（关键！）
    if (m_blCompassShow && m_compassLabel) {
        if (location == "LeftTop") {
            m_compassLabel->move(20, 20);  
        }
        else if (location == "BottomCenter") {
            // 居中底部：x = (widget宽 - label宽)/2, y = 高度 - label高 - 边距
            int x = (this->width() - m_compassLabel->width()) / 2;
            int y = this->height() - m_compassLabel->height() - 10;
            m_compassLabel->move(x, y);
        }
		else if (location == "RightTop") {
			int x = this->width() - m_compassLabel->width() - 20;
			int y = 20;
			m_compassLabel->move(x, y);
		}

    }
	update();  // 重新绘制以应用位置更改

}

void PanoramaWidget::setCompassImagePath(const QString& path)
{
    m_compassN_picpath = path; 
    update();     // 重新加载并显示指北针图片
}
////////////////////////////////////////////
