#ifndef PANORAMAWIDGET_H
#define PANORAMAWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QTimer>
#include <QOpenGLVertexArrayObject>
#include <QVector3D>
#include <QMatrix4x4>
#include <QPoint>
#include <QLabel>
#include <QPropertyAnimation>
#include "RotatableLabel.h"
#include "CommonTool.h"
#include <QMutex>
#include <QSqlQuery>
#include <QSqlError>


#define M_PI 3.14159265358979323846

//初始化窗口
struct WindowParameter
{
    QString start_angle;
    QString fov;
    QString zoom;
    QString zoom_range;
    QString h_range;
    QString v_range;   
};

//指北针
struct CompassParameter
{
    float compassAngle;
    QString picPath;
    QString location;
};

// 热点数据结构
struct Hotspot 
{
    QString iconID;         //唯一标识码
    QVector3D position;      // 球面坐标位置
    QString iconPath;       // 热点图片路径
    QString name;           // 热点名称
    QString description;    // 热点描述
    QString style;          //类型
    float scale_x;           // 热点缩放比例 X
    float scale_y;           // 热点缩放比例 Y
    
    bool lock = false;

    bool icon_visible;          // 是否可见
    bool title_visible;          // 是否可见
  
    bool isFollowSysZoom;   //是否跟随系统缩放

    bool movable;           // 是否可移动
    bool selected;          // 是否被选中

    QColor title_bg_color;      //标题背景颜色
    QColor title_text_color;      //标题文本颜色
    int title_text_size;      //标题文本大小

    QOpenGLTexture* texture;
};

//按层级显示
struct HotspotLevelShow
{
    QString iconID;
    QString picID;
    float min_zoom;
    float max_zoom;
    QString LevelID;
};

//按分组显示
struct HotspotGroupShow
{
    QString iconID;
    QString picID;

    QString groupName;
    bool isGroupShowInPic;//在场景中显示

    bool isPerGroupSeeAngle;//分组选择后，是否旋转到指定角度

    float group_rotate_x;
    float group_rotate_y;
    float gruop_zoom;

    bool isToGroupZoom;//转到旋转的x，y后，再缩放到此组的gruop_zoom

    bool isdefOpen;//默认显示此组
    
    QString GroupID;

    bool isIncludeAll;//是否包括所有热点，包括刚添加的
};

//分组样式
struct GroupItemStyle
{
    QString groupName; 
    QString groupID;
    QString unsel_bgColor; 
    QString unsel_textColor;
    QString sel_bgColor; 
    QString sel_textColor;
};

//分组视角
struct GroupSeeAngleZoom
{
    QVector3D rotation;
    float zoom;
    QPixmap pixmap;
    bool isToGroupZoom;
};

class PanoramaWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT
    Q_PROPERTY(float transitionProgress READ getTransitionProgress WRITE setTransitionProgress)
public:
    //explicit PanoramaWidget(QString m_picPath,bool autoRotate = false, bool blCompassShow = false, QString compassN_location="LeftTop", QWidget* parent = nullptr);
    explicit PanoramaWidget(QString m_picPath,bool autoRotate = false, bool blCompassShow = false, QWidget* parent = nullptr);
    ~PanoramaWidget();
protected:
    void initializeGL() override;
	void paintGL() override;  //渲染函数
    void resizeGL(int w, int h) override;
    void InsertHPToDB();//热点插入数据库
    void UpdateHPToDB();//热点更新数据库

signals:
    void sigShowElement(bool);
public:
   // QLabel* m_compassLabel;
	RotatableLabel* m_compassLabel;  //指北针标签
 
    std::map<QString, WindowParameter> m_mapPicAngle;
    std::map<QString, CompassParameter> m_mapPicCompass;
    std::map<QString, QList<Hotspot>> m_mapPicHotspot;
    std::map<QString, std::vector<HotspotLevelShow>> m_mapVecIconID_Zoom;//热点ID,对应可以显示的多个zoom区域
    std::map<QString, std::vector<HotspotGroupShow>> m_mapVecIconID_Group;//热点ID,对应可以显示的多个group区域

    float getRotationY() const { return m_rotation.y(); }  

    Hotspot m_cutHotPoint;//临时热点
    QPoint m_cutHpScreenPos;//临时热点屏幕坐标
    void setCompassLocation(const QString& location);

    // 全景图加载
    void loadPanorama(const QString& imagePath);
    // 缩放控制
    void zoomIn();
    void zoomOut();
    void setZoom(float zoomLevel);
    void removeHotspot(int index);
    void setHotspotVisible(int index, bool visible);
    int getHotspotCount();
    void initHotspotGeometry();
  
    // 热点位置控制
    void setHotspotPosition(int index, const QVector3D& position);
    QVector3D getHotspotPosition(int index) const;

    // 热点移动模式控制
    void enableHotspotMoving(bool enabled);
    //bool isHotspotMovingEnabled() const;
    void setHotspotMovable(int index, bool movable);
    void changePic(QString picPath, QString id,bool isTransitioning = true);
    void initWindow(QString&);

    //渲染热点
    void randerHotspotList();
    void randerHotspot_tmp();

	// 指北针图片路径设置
    void setCompassImagePath(const QString& path);

public slots:
    // 自动旋转控制
    void startAutoRotation();
    void stopAutoRotation();
    void setAutoRotationSpeed(float speed);

    void resetCompassRotation() {
        m_rotation.setY(0);  // 重置指北针旋转角度
    }
signals:
    // 热点点击信号
    void hotspotClicked(int index, const QString& name);
    // 热点移动信号
    //void hotspotMoved(int index, const QVector3D& newPosition);
    // 热点选择状态变化
    void hotspotSelectionChanged(int index, bool selected);
    // 图片过渡完成
    void sig_transEnd();
    //场景缩放对应的视角
    void sig_seeAngle(float);
    //当前视角
    void sig_cutSeeAngle(QString);
public:
    // 鼠标事件处理
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private slots:
    void onAnimationTimer();
private:
    // 辅助函数：判断矩阵是否可逆（替代isInvertible()）
    bool isMatrixInvertible(const QMatrix4x4& mat) {
        const float epsilon = 1e-8f; // 浮点精度容错
        return qAbs(mat.determinant()) > epsilon;
    }

private:
    void createSphereGeometry();
    void setupShaders();
    void resetInactivityTimer();

    //void setupHotspotSystem();
    void checkHotspotClick(const QPoint& mousePos);

    // 热点交互检测
    bool getHotspotAtPosition(const QPoint& mousePos);
    bool isPointNearHotspot(const QVector2D& screenPoint, const QVector3D& hotspotPos);
    //
    QVector3D screenToSphere(const QPoint& mousePos);
    QPoint sphereToScreen(const QVector3D& spherePos);
    //优化修正
    QVector3D screenToSphere(const QPoint& screenPos,
        const QMatrix4x4& view,
        const QMatrix4x4& projection,
        const QVector3D& rotation);
    QPoint sphereToScreen(const QVector3D& worldPos, const QMatrix4x4& view, const QMatrix4x4& projection);
    QPoint rotationZoomToPixelOffset(const QVector3D& rotation, float zoom);
    //实时热点默认位置
    QVector3D sphericalToCartesian(double theta, double phi) 
    {
        float theta0 = theta * M_PI / 180;
        float phi0 = phi * M_PI / 180 - M_PI / 2;
        double x = cos(theta0) * cos(phi0);
        double y = -sin(theta0);
        double z = cos(theta0) * sin(phi0);
        return QVector3D(x, y, z);
    }
    //初始化Hotspot
    void InitHotspot()
    {
        m_cutHotPoint.iconID = "";
        m_cutHotPoint.position = QVector3D(0, 0, -1);
        m_cutHotPoint.iconPath = "";
        m_cutHotPoint.name = "";
        m_cutHotPoint.description = "";
        m_cutHotPoint.style = "";
        m_cutHotPoint.scale_x = 0.0;
        m_cutHotPoint.scale_y = 0.0;
        m_cutHotPoint.lock = false;
        m_cutHotPoint.icon_visible = true;
        m_cutHotPoint.title_visible = true;
        m_cutHotPoint.movable = true;
        m_cutHotPoint.selected = true;
        m_cutHotPoint.isFollowSysZoom = false;

        m_cutHotPoint.title_bg_color = QColor(30, 30, 30, 255);
        m_cutHotPoint.title_text_color = QColor(255, 255, 255, 255);
        m_cutHotPoint.title_text_size = 10;

        m_cutHotPoint.texture = nullptr;
    }
    void CopyIcon(const Hotspot& hp)
    {
        Hotspot newCutHotPoint = deepCopyHotspot(hp);
        m_mapPicHotspot[m_picPath].append(newCutHotPoint);
    }
    // 封装深拷贝函数
    Hotspot deepCopyHotspot(const Hotspot& src) 
    {
        Hotspot dst;
        dst.iconID = src.iconID;
        dst.position = src.position;
        dst.iconPath = src.iconPath;
        dst.name = src.name;
        dst.description = src.description;
        dst.style = src.style;
        dst.scale_x = src.scale_x;
        dst.scale_y = src.scale_y;  
        dst.lock = src.lock;
        dst.icon_visible = src.icon_visible;
        dst.title_visible = src.title_visible;
        dst.movable = src.movable;
        dst.selected = src.selected;
        dst.isFollowSysZoom = src.isFollowSysZoom;

        dst.title_bg_color = src.title_bg_color;
        dst.title_text_color = src.title_text_color;
        dst.title_text_size = src.title_text_size;

        makeCurrent();
        QImage img(src.iconPath);
        if (img.isNull()) return dst;
        QImage textureImg = img.convertToFormat(QImage::Format_RGBA8888);
        textureImg = textureImg.mirrored(false, true);
        dst.texture = new QOpenGLTexture(textureImg);
        dst.texture->setMinificationFilter(QOpenGLTexture::Linear);
        dst.texture->setMagnificationFilter(QOpenGLTexture::Linear);
        dst.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        doneCurrent();

        return dst;
    }

    QOpenGLTexture* m_texture;  // 纹理
    QMatrix4x4 m_projection;  // 投影矩阵声明

    // 几何数据
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;

    // 球体参数
    int m_sphereStacks;
    int m_sphereSlices;
    // 定时器
    QTimer* m_animationTimer;
    QTimer* m_inactivityTimer;

    // 自动旋转状态
    bool m_autoRotate;
    float m_rotationSpeed;
    int m_inactivityDelay;

    // 当前场景路径
    QString m_picID;
    QString m_picPath;

    bool m_blLevelShow = false; // 按层级显示
    bool m_blGroupShow = false; // 按组显示

    //热点是否显示
    bool m_blHotspotShow = false;
    //指北针是否显示
    bool m_blCompassShow = false;

    QMatrix4x4 m_view;

    QOpenGLShaderProgram* m_program; //全景图
    QOpenGLShaderProgram* m_maskProgram; //遮罩
    QOpenGLShaderProgram* m_hotspotProgram;//热点


    QOpenGLVertexArrayObject m_panoramaVAO;
    QOpenGLVertexArrayObject m_hotspotVAO;

    QOpenGLBuffer m_hotspotVBO;

    // 热点移动模式
    bool m_hotspotMovingEnabled;

    // 交互状态
    bool m_isDragging;
    bool m_isMovingHotspot;
    int m_selectedHotspot;

    // 过渡效果相关
    bool m_transitionEnabled;
    bool m_isTransitioning;
    float m_transitionProgress;
    QPropertyAnimation* m_animation;
    QOpenGLTexture* m_nextTexture;

    // 过渡进度属性访问器
    float getTransitionProgress() const;
    void setTransitionProgress(float progress);

    QPoint m_lastMousePos;

    // 视图控制 
    float m_fov;
    QVector3D m_rotation;  
    float m_zoom;
    float m_zoom_min;
    float m_zoom_max;
    float m_H_min;
    float m_H_max;
    float m_V_min;
    float m_V_max;

    //指北针
    float m_compassN;
	QString m_compassN_picpath;  //指北针图片路径
    QString m_compassN_location; //指北针位置：LeftTop, BottomCenter

    //当前选择分组
    QString m_SelGroupID = "";
    //当前分组状态：编辑，查看
    QString m_GroupStatus = "";

// ---------------供外部访问-------------------
public:
    void addHotspot(Hotspot hp);// 添加热点
    void addHotspot2List(Hotspot hp);// 添加热点
    QVector3D getRotatedPosition();//默认位置
    void SetAllPicAngle(std::map<QString, WindowParameter>&);//初始化视角
    void SetOnePicAngle(QString, WindowParameter);//初始化一个视角
    void SetAllPicCompass(std::map<QString, CompassParameter>&);//初始化指北针
    void SetOnePicCompass(QString, CompassParameter);//初始化一个指北针
    void SetAutoRorate(bool bl) 
    {
        if (bl == true)
        {
            connect(m_animationTimer, &QTimer::timeout, this, &PanoramaWidget::onAnimationTimer);
            connect(m_inactivityTimer, &QTimer::timeout, this, &PanoramaWidget::startAutoRotation);
        }
        else
        {
            disconnect(m_animationTimer, &QTimer::timeout, this, &PanoramaWidget::onAnimationTimer);
            disconnect(m_inactivityTimer, &QTimer::timeout, this, &PanoramaWidget::startAutoRotation);
        }   
    };
    void SetPicID(QString id) 
    {
        m_picID = id; 
    }; // 设置热点ID
    void setHotspotShow(bool bl) { m_blHotspotShow = bl; };
    void setCompassShow(bool bl) { m_blCompassShow = bl; };
    QVector3D getRotation() { return m_rotation; };
    float getZoom() { return m_zoom; };
    float getCAngle() { return m_rotation.y(); };//现在的旋转角度
    QString getPicID() { return m_picID; };
    void setCompassNShow(bool bl) //设置指北针显隐
    {
        m_blCompassShow = bl;
        if (bl == true)
            m_compassLabel->show();
        else
            m_compassLabel->hide();
    };
    //清空当前场景下的热点
    void clearCutPicHp()
    {
        m_mapPicHotspot[m_picPath].clear();
    };
    //设置分组状态
    void setGroupStatus(QString status) 
    {
        m_GroupStatus = status;
    };
    void setCompassN(float ang)//设置指北针角度 
    {
        m_compassN = ang;
		update();
    };
    void setLevelShow(bool bl)//设置层级显示
    {
        m_blLevelShow = bl;
    }
    void set_SelGroupID(QString id)//设置当前选择分组
    {
        m_SelGroupID = id;
        if (m_SelGroupID != "")
            m_blGroupShow = true;       
        else
            m_blGroupShow = false;
    }
    void updateHotspotPath(QString imagePath)//更新图标
    {
        QImage img(imagePath);
        if (img.isNull()) return;
        QImage textureImg = img.convertToFormat(QImage::Format_RGBA8888);
        textureImg = textureImg.mirrored(false, true); 
        m_cutHotPoint.texture = new QOpenGLTexture(textureImg);
        m_cutHotPoint.texture->setMinificationFilter(QOpenGLTexture::Linear);
        m_cutHotPoint.texture->setMagnificationFilter(QOpenGLTexture::Linear);
        m_cutHotPoint.texture->setFormat(QOpenGLTexture::RGBA8_UNorm);
        m_cutHotPoint.iconPath = imagePath;
        update();
    };
    void Update()//刷新
    {
        update();
    }
    void setScale_X(float scale_x)//设置图标缩放X
    {
        m_cutHotPoint.scale_x = scale_x;
        update();
    };
    void setScale_Y(float scale_y) //设置图标缩放Y
    {
        m_cutHotPoint.scale_y = scale_y;
        update();
    };
    void setIconName(QString text) //设置图标文本
    {
        m_cutHotPoint.name = text;
        update();
    };
    void setTitleBgColor(QColor cl) //设置标题背景颜色
    {
        m_cutHotPoint.title_bg_color = cl;
        update();
    };
    void setTitleTextColor(QColor cl) //设置标题文本颜色
    {
        m_cutHotPoint.title_text_color = cl;
        update();
    };
    void setTitleTextSize(int size) //设置图标文本大小
    {
        m_cutHotPoint.title_text_size = size;
        update();
    };
    void slotNameShow(bool blShow) //是否显示标题
    {
        m_cutHotPoint.title_visible = blShow;
        update();
    };
    void slotAddIconEnd() //添加热点
    {
        Hotspot newCutHotPoint = deepCopyHotspot(m_cutHotPoint);
        m_mapPicHotspot[m_picPath].append(newCutHotPoint);
        //插入数据库
        InsertHPToDB();
       
        InitHotspot();
        update();
    };
    void slotDemTmpIcon() //初始化临时热点
    {
        InitHotspot();
        update();
    };
    void slotCopyIcon(Hotspot& hp) //复制热点
    {
        CopyIcon(hp);
        update();
    };
    void slotEditCopyToTmpIcon(QString iconID)
    {
        //赋值到当前热点
        QList<Hotspot> hpList = m_mapPicHotspot[m_picPath];
        for (auto hp : hpList)
        {
            if (hp.iconID == iconID)
            {
                m_cutHotPoint = hp;
                
                break;
            }
        } 
        //转到热点位置
        double x = m_cutHotPoint.position.x();
        double y = m_cutHotPoint.position.y();
        double z = m_cutHotPoint.position.z();
        float theta = RAD2DEG(atan2(-y, sqrt(x * x + z * z)));
        float phi = RAD2DEG(atan2(z, x) + M_PI / 2);
        m_rotation = QVector3D(theta, phi, m_rotation.z());
        //
        update();
    }
    //旋转到当前热点
    void rotateCutHotPoint(QString iconID)
    {
        Hotspot SelHp;
        QList<Hotspot> hpList = m_mapPicHotspot[m_picPath];
        for (auto hp : hpList)
        {
            if (hp.iconID == iconID)
            {
                SelHp = hp;
                break;
            }
        }
        //转到热点位置
        double x = SelHp.position.x();
        double y = SelHp.position.y();
        double z = SelHp.position.z();
        float theta = RAD2DEG(atan2(-y, sqrt(x * x + z * z)));
        float phi = RAD2DEG(atan2(z, x) + M_PI / 2);
        m_rotation = QVector3D(theta, phi, m_rotation.z());
        //
        update();
    }

    void slotEditTmpIconToList()
    {
        QList<Hotspot>& hpList = m_mapPicHotspot[m_picPath];
        for (auto& hp : hpList)
        {
            if (hp.iconID == m_cutHotPoint.iconID)
            {
                hp = m_cutHotPoint;
                //更新数据库
                UpdateHPToDB();
                InitHotspot();
                break;
            }
        }     
        //
        update();
    }
    void setCutZoom(float zoom)
    { 
        m_zoom = zoom;
        update();
    };
    void setRotate(float x, float y)
    {
        m_rotation.setX(x);
        m_rotation.setY(y);
    }
    //层次
    void setIconShowZooms(std::map<QString, std::vector<HotspotLevelShow>>& mapVecIconID_Zoom)
    {
        m_mapVecIconID_Zoom.clear();
        m_mapVecIconID_Zoom = mapVecIconID_Zoom;
    };
    //分组
    void setIconShowGroups(std::map<QString, std::vector<HotspotGroupShow>>& mapVecIconID_Group)
    {
        m_mapVecIconID_Group.clear();
        m_mapVecIconID_Group = mapVecIconID_Group;
    };

    //判断热点显示
    bool isIconShowLevel(QString iconID) 
    {
        std::vector<HotspotLevelShow> tmpLevelShow = m_mapVecIconID_Zoom[iconID];
        if (tmpLevelShow.size() == 0)
        {
            return true;
        }
        for (auto tmp : tmpLevelShow)
        {
            float min = 60.0 / tmp.max_zoom;
            float max = 60.0 / tmp.min_zoom;
            if (m_zoom > min && m_zoom < max)
            {
                return true;
            }
        }
        return false;
    };
    bool isIconShowGroup(QString iconID)
    {
        if (m_SelGroupID == "")
        {
            return false;
        }
        std::vector<HotspotGroupShow> tmpGroupShow = m_mapVecIconID_Group[iconID];
        if (tmpGroupShow.size() == 0)
        {
            return true;
        }
        for (auto tmp : tmpGroupShow)
        {
            if (tmp.GroupID == m_SelGroupID)
            {
                return true;
            }
        }
        return false;
    };
    //删除热点
    void DelIcon(QString iconID)
    {
        QMutex mutex;
        mutex.lock();

        for (int i = 0; i < m_mapPicHotspot[m_picPath].size(); ++i)
        {
            if (m_mapPicHotspot[m_picPath][i].iconID == iconID)
            {
                m_mapPicHotspot[m_picPath].removeAt(i); 
                break; 
            }
        }
        //
        m_mapVecIconID_Zoom.erase(iconID);
        //
        m_mapVecIconID_Group.erase(iconID);

        mutex.unlock();
    };
    //删除场景
    void DelPic(std::vector<QString> vecPicID)
    {
        QMutex mutex;
        mutex.lock();
        for (auto tmpID : vecPicID)
        {
            m_mapPicAngle.erase(tmpID);
            m_mapPicCompass.erase(tmpID);
            m_mapPicHotspot.erase(tmpID);
            //
            std::vector<QString> vecHpID;
            QString data = QString("select id from hotpoint where pic_id = '%1'").arg(tmpID);
            QSqlQuery query(data);//查询表的内容
            while (query.next())
            {
                vecHpID.push_back(query.value(0).toString());
            }
            for (auto tmphpId : vecHpID)
            {
                m_mapVecIconID_Zoom.erase(tmpID);
                m_mapVecIconID_Group.erase(tmpID);
            }
        }
        mutex.unlock();
    }
    //修改显示、锁定
    void setIconShow_Lock(QString iconID, QString opera)
    {
        for (auto& tmphotspot : m_mapPicHotspot[m_picPath])
        {
            if (tmphotspot.iconID == iconID)
            {
                if (opera == u8"显示")        
                    tmphotspot.icon_visible = true;
                else if(opera == u8"隐藏")
                    tmphotspot.icon_visible = false;
                else if (opera == u8"锁定")
                    tmphotspot.lock = true;
                else if (opera == u8"解锁")
                    tmphotspot.lock = false;

                break;
            }      
        }
    }

    float getCompassN() { return m_compassN; };
    QString getCompass_picPath() { return m_compassN_picpath; };
    QString getCompass_location() { return m_compassN_location; };

};

#endif // PANORAMAWIDGET_H