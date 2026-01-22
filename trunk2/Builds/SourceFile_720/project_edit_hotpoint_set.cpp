#include "project_edit_hotpoint_set.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QPixmap>
#include "PanoramaWidget.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QColorDialog>


project_edit_hotpoint_set::project_edit_hotpoint_set(QString iconID, QWidget *parent)
	: QWidget(parent), m_AnimationGroup(nullptr)
{
	ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_QuitOnClose, true);
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrHeight = screenRect.height() - getBottomPix();
    //动态长度
    {
        resize(250, scrHeight - 60 + 10);
        ui.label_bg->resize(250, scrHeight - 60 + 10);
        ui.pushButton_done->move(10, scrHeight - 60 - 10 - 30);
    }

    m_isExist = false;
    m_iconUUID = iconID;
    m_iconPath = "";
 

    ui.horizontalSlider_w->setMinimum(1);
    ui.horizontalSlider_w->setMaximum(200);
    ui.horizontalSlider_w->setTickInterval(1);

    ui.horizontalSlider_h->setMinimum(1);
    ui.horizontalSlider_h->setMaximum(200);
    ui.horizontalSlider_h->setTickInterval(1);

    connect(ui.horizontalSlider_w, SIGNAL(valueChanged(int)), SLOT(setHorValueW(int)));
    connect(ui.horizontalSlider_h, SIGNAL(valueChanged(int)), SLOT(setHorValueH(int)));
    connect(ui.checkBox_zoom, SIGNAL(stateChanged(int)), SLOT(setHorValueHW(int)));
    connect(ui.textEdit_tilte, SIGNAL(textChanged()), SLOT(setIconName()));
    connect(ui.checkBox_ShowTitle, SIGNAL(stateChanged(int)), SLOT(setShowTitle(int)));

    connect(ui.pushButton_selPic, SIGNAL(clicked()), this, SLOT(slotSelPic()));  
    connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));

    connect(ui.pushButton_BgColor, SIGNAL(clicked()), this, SLOT(slotSetTitleBgColor()));
    connect(ui.pushButton_TextColor, SIGNAL(clicked()), this, SLOT(slotSetTitleTextColor()));

    ui.comboBox_TextSize->addItem("10");
    ui.comboBox_TextSize->addItem("12");
    ui.comboBox_TextSize->addItem("14");
    ui.comboBox_TextSize->addItem("16");
    ui.comboBox_TextSize->addItem("18");
    ui.comboBox_TextSize->addItem("20");

    ui.comboBox_TextSize->setCurrentIndex(-1);
    connect(ui.comboBox_TextSize, SIGNAL(currentIndexChanged(int)), this, SLOT(slotTextSizeChanged(int)));

    ui.horizontalSlider_w->setValue(5);
    ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));


    //新热点默认设置
    m_titleBgColor = QColor(30, 30, 30, 255);
    m_titleTextColor = QColor(255, 255, 255, 255);
    m_titleTextSize = 10;

    if (iconID != "")
    {
        m_isExist = true;

        QString iconName;
        QString iconPath;
        QString iconPosition;
        QString strWzoom;
        QString strHzoom;
        bool blTitleShow;

        QString data = QString("select * from hotpoint where id = '%1' LIMIT 1").arg(iconID);
        QSqlQuery query(data);//查询表的内容
        while (query.next())
        {    
            iconName = query.value(1).toString();
            iconPath = query.value(3).toString();
            iconPosition = query.value(5).toString();
            strWzoom = query.value(6).toString();
            strHzoom = query.value(7).toString();
            blTitleShow = query.value(10).toBool();

            QStringList bgColorList = query.value(13).toString().split(",");
            QStringList textColorList = query.value(14).toString().split(",");
            int size = query.value(15).toInt();
            if (bgColorList.size() == 3) 
                m_titleBgColor = QColor(bgColorList[0].toInt(), bgColorList[1].toInt(), bgColorList[2].toInt());
          
            if (textColorList.size() == 3)
                m_titleTextColor = QColor(textColorList[0].toInt(), textColorList[1].toInt(), textColorList[2].toInt());
         
            m_titleTextSize = size;
        }

        QPixmap pixmap(iconPath);
        QPixmap fitpixmap = pixmap.scaled(61, 61, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        ui.label_hp_icon->setPixmap(fitpixmap);
        m_iconPath = iconPath;

        if (strWzoom == strHzoom)
            ui.checkBox_zoom->setChecked(true);
        else
            ui.checkBox_zoom->setChecked(false);

        ui.horizontalSlider_w->setValue(strWzoom.toFloat() * 100.0);
        ui.horizontalSlider_h->setValue(strHzoom.toFloat() * 100.0);

        ui.textEdit_tilte->setPlainText(iconName);

        if (blTitleShow == true)
            ui.checkBox_ShowTitle->setChecked(true);
        else
            ui.checkBox_ShowTitle->setChecked(false);

        //旧数据
        {
            QStringList pList = iconPosition.split(",");
            if (pList.size() == 3)
                m_oldHpPam.position = QVector3D(pList[0].toFloat(), pList[1].toFloat(), pList[2].toFloat());
            m_oldHpPam.iconPath = iconPath;
            m_oldHpPam.scale_x = strWzoom.toFloat();
            m_oldHpPam.scale_y = strHzoom.toFloat();
            m_oldHpPam.name = iconName;
            m_oldHpPam.title_bg_color = m_titleBgColor;
            m_oldHpPam.title_text_color = m_titleTextColor;
            m_oldHpPam.title_text_size = m_titleTextSize;
        }
    }    
    ui.pushButton_BgColor->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_titleBgColor.red()).arg(m_titleBgColor.green()).arg(m_titleBgColor.blue()));      
    ui.pushButton_TextColor->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_titleTextColor.red()).arg(m_titleTextColor.green()).arg(m_titleTextColor.blue()));
    ui.comboBox_TextSize->setCurrentText(QString::number(m_titleTextSize)); 
}

project_edit_hotpoint_set::~project_edit_hotpoint_set()
{}

void project_edit_hotpoint_set::slotClose()
{
    emit sig_edit_hp_set_SetHPLocation(false);
    emit sig_DelTmpIcon();
    this->close();
    this->destroy(true);
}

void project_edit_hotpoint_set::slideOut(QWidget* mainWindow)
{
    // 临时添加：解除尺寸限制，让窗口自适应内容
    this->setMinimumSize(0, 0);
    this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
    this->adjustSize(); // 强制窗口适配内容尺寸

    // 获取「主窗口所在的屏幕」（而非系统主屏幕）
    QScreen* screen = mainWindow->screen();
    if (!screen) { // 极端情况：获取屏幕失败，降级处理
        screen = QApplication::primaryScreen();
    }
    QRect screenRect = screen->availableGeometry(); // 当前屏幕的可用区域（排除任务栏）

    // 1. 初始位置：完全在「当前屏幕右侧外部」（x=当前屏幕宽度，看不到）
    int initX = screenRect.width();
    int targetY = 65; // 垂直偏移（相对于当前屏幕顶部）

    // 关键：窗口位置基于当前屏幕的坐标系（而非全局）
    this->move(screenRect.x() + initX, screenRect.y() + targetY);
    this->show(); // 先显示（初始在当前屏幕右侧外，不可见）

    // 2. 位置动画（基于当前屏幕计算目标位置）
    QPropertyAnimation* pPosAnimation = new QPropertyAnimation(this, "pos");
    pPosAnimation->setDuration(600);
    // 起点：当前屏幕右侧外部（screenRect.x() + initX = 当前屏幕X + 屏幕宽度 = 右侧外）
    pPosAnimation->setStartValue(QPoint(screenRect.x() + initX, screenRect.y() + targetY));
    // 终点：当前屏幕右侧边缘（x=当前屏幕X + 屏幕宽度 - 窗口宽度，y不变）
    int targetX = screenRect.x() + screenRect.width() - this->width();
    pPosAnimation->setEndValue(QPoint(targetX, screenRect.y() + targetY));
    pPosAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    // 3. 顺序动画组（延迟+滑动，逻辑不变）
    QSequentialAnimationGroup* pPosGroup = new QSequentialAnimationGroup(this);
    pPosGroup->addPause(100);
    pPosGroup->addAnimation(pPosAnimation);

    // 4. 并行动画组（逻辑不变）
    m_AnimationGroup = new QParallelAnimationGroup(this);
    m_AnimationGroup->addAnimation(pPosGroup);

    // 启动动画（自动删除，避免内存泄漏）
    m_AnimationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void project_edit_hotpoint_set::slideInHide()
{
    QScreen* screen = QApplication::primaryScreen();
    QRect screenRect = screen->availableGeometry();

    int initX = screenRect.width();
    int targetY = 60;
    this->move(initX, targetY);
    this->show();

    raise(); // 提升层级，避免被遮挡
    activateWindow(); // 激活窗口，确保显示
    repaint();
    QPropertyAnimation* pPosAnimation = new QPropertyAnimation(this, "pos");
    pPosAnimation->setDuration(600);
    pPosAnimation->setStartValue(QPoint(initX, targetY));
    pPosAnimation->setEndValue(QPoint(screenRect.width() - this->width(), targetY));
    pPosAnimation->setEasingCurve(QEasingCurve::InOutQuad);

    QSequentialAnimationGroup* pPosGroup = new QSequentialAnimationGroup(this);
    pPosGroup->addPause(300);
    pPosGroup->addAnimation(pPosAnimation);

    m_AnimationGroup = new QParallelAnimationGroup(this);
    m_AnimationGroup->addAnimation(pPosGroup);
    m_AnimationGroup->start(QAbstractAnimation::DeleteWhenStopped);
}

void project_edit_hotpoint_set::slotSelPic()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();


    if (m_iconWin != nullptr)
    {
        delete m_iconWin;
    }
    m_iconWin = new project_hotpoint_icon();
    connect(m_iconWin, SIGNAL(sig_iconPath(QString)), this, SLOT(slotSetIcon(QString)));
    m_iconWin->setWindowModality(Qt::ApplicationModal);
    m_iconWin->move((scrWidth - 800) / 2, (scrHeight - 1000) / 2);
    m_iconWin->show();
}

void project_edit_hotpoint_set::slotSetIcon(QString iconPath)
{  
    if (iconPath == "" || m_iconPath == iconPath) return;

    if (m_iconUUID == "")
        m_iconUUID = getUUID();
    Hotspot hpIcon;
    hpIcon.iconID = m_iconUUID;
    hpIcon.position = QVector3D(0, 0, -1);
    hpIcon.iconPath = iconPath;
    hpIcon.style = "system";
    hpIcon.name = "";
    hpIcon.description = "";
    hpIcon.scale_x = ui.horizontalSlider_w->value() / 100.0;
    hpIcon.scale_y = ui.horizontalSlider_h->value() / 100.0;
    hpIcon.icon_visible = true;
    hpIcon.title_visible = ui.checkBox_ShowTitle->checkState();
    hpIcon.movable = true;
    hpIcon.selected = true;
    hpIcon.isFollowSysZoom = false;

    hpIcon.title_bg_color = m_titleBgColor;
    hpIcon.title_text_color = m_titleTextColor;
    hpIcon.title_text_size = m_titleTextSize;

    hpIcon.texture = nullptr;
    
    QPixmap pixmap(iconPath);
    QPixmap fitpixmap = pixmap.scaled(61, 61, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui.label_hp_icon->setPixmap(fitpixmap);

    if (m_iconPath == "")
    {       
        emit sig_addIconSet(hpIcon);
    }
    else
    {
        emit sig_updateIconSet(iconPath);
    }
    m_iconPath = iconPath;
}

void project_edit_hotpoint_set::setHorValueW(int value)
{
    if (ui.checkBox_zoom->isChecked() == true)
    {
        ui.lineEdit_wzoom->setText(QString::number(ui.horizontalSlider_w->value()) + "%");

        ui.horizontalSlider_h->setValue(value);
        ui.lineEdit_hzoom->setText(QString::number(ui.horizontalSlider_w->value()) + "%");
    }
    else
        ui.lineEdit_wzoom->setText(QString::number(ui.horizontalSlider_w->value()) + "%");

    if (m_iconPath != "")
    {
        emit sig_setScaleX(ui.horizontalSlider_w->value() / 100.0);
    }
}

void project_edit_hotpoint_set::setHorValueH(int value)
{
    if (ui.checkBox_zoom->isChecked() == true)
    {
        ui.lineEdit_hzoom->setText(QString::number(ui.horizontalSlider_h->value()) + "%");

        ui.horizontalSlider_w->setValue(value);
        ui.lineEdit_wzoom->setText(QString::number(ui.horizontalSlider_h->value()) + "%");
        
    }
    else
        ui.lineEdit_hzoom->setText(QString::number(ui.horizontalSlider_h->value()) + "%");

    if (m_iconPath != "")
    {
        emit sig_setScaleY(ui.horizontalSlider_h->value() / 100.0);
    }
}

void project_edit_hotpoint_set::setHorValueHW(int va)
{
    if (ui.checkBox_zoom->isChecked() == true)
    {
        ui.horizontalSlider_h->setValue(ui.horizontalSlider_w->value());
    }
}

void project_edit_hotpoint_set::setIconName()
{
    QString title = ui.textEdit_tilte->toPlainText();
    emit sig_setIconName(title);
}

void project_edit_hotpoint_set::setShowTitle(int)
{
    emit sig_IconNameShow(ui.checkBox_ShowTitle->isChecked());
}

void project_edit_hotpoint_set::slotSetTitleBgColor()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_titleBgColor.red(), m_titleBgColor.green(), m_titleBgColor.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_titleBgColor = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_BgColor->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
                                                     .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
        //
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        emit sig_setTitleBgColor(QColor(r, g, b));
    }
}

void project_edit_hotpoint_set::slotSetTitleTextColor()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_titleTextColor.red(), m_titleTextColor.green(), m_titleTextColor.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_titleTextColor = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_TextColor->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
        //
        int r = color.red();
        int g = color.green();
        int b = color.blue();
        emit sig_setTitleTextColor(QColor(r,g,b));
    }
}

void project_edit_hotpoint_set::slotTextSizeChanged(int index)
{
    m_titleTextSize = ui.comboBox_TextSize->currentText().toInt();
    //
    emit sig_setTextSize(m_titleTextSize);
}

void project_edit_hotpoint_set::slotDone()
{
    if (m_iconPath == "")
    {
        emit sig_Msg_Set(u8"请选择图标！");
        return;
    }
    QString title = ui.textEdit_tilte->toPlainText();
    if (title == "")
    {
        emit sig_Msg_Set(u8"请设置标题！");
        return;
    }

    Hotspot hpIcon;
    hpIcon.iconID = m_iconUUID;
    hpIcon.iconPath = m_iconPath;
    hpIcon.name = ui.textEdit_tilte->toPlainText();

    hpIcon.lock = false;
    hpIcon.icon_visible = true;

    hpIcon.title_bg_color = m_titleBgColor;
    hpIcon.title_text_color = m_titleTextColor;
    hpIcon.title_text_size = m_titleTextSize;

    if (m_isExist == false)//添加新热点
    {
        //添加到场景
        emit sig_AddIconEnd();
        //添加到list       
        emit sig_AddIcon(hpIcon);
    }
    else//更新存在的热点
    {
        //更新到场景
        emit sig_EditIconEnd();
        //更新到list
        emit sig_UpdateIcon(hpIcon);
    }
    //
    emit sig_Msg_Set(u8"保存成功!");
    emit sig_edit_hp_set_SetHPLocation(false);
    slotClose();
}

