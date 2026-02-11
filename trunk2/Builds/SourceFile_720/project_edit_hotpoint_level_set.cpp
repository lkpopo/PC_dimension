#include "project_edit_hotpoint_level_set.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include "project_edit_hotpoint_level_set_item.h"
#include "PanoramaWidget.h"
#include <QSqlQuery>
#include <QSqlError>


project_edit_hotpoint_level_set::project_edit_hotpoint_level_set(QString projUUID, QString picPath, QString picID, QString levelID, QWidget *parent)
    :m_projUUID(projUUID), m_cutPicPath(picPath), m_picID(picID), QWidget(parent)
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
        resize(250, scrHeight - 60);
        ui.label_bottom_bg->resize(231, scrHeight - 60 - 330 - 10 - 2 * 10 - 30);
        ui.listWidget->resize(251, scrHeight - 60 - 490 - 10 - 2 * 10 - 30);
        ui.pushButton_done->move(10, scrHeight - 60 - 10 - 30);
    }
    ui.pushButton_del->hide();
    m_isExist = false;
    m_levelID = levelID;

    //隐藏
    {
        ui.label->hide();
        ui.pushButton_filter->hide();
        ui.pushButton_style->hide();
    }

    ui.horizontalSlide->setStyleSheet("QSlider::handle { background: transparent; }"
    "QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");

    connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));

    connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotClearSearchLine()));
    connect(ui.lineEdit_search, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));
    connect(this, SIGNAL(sig_UpdateHotpointList(QVector<Hotspot>&)), this, SLOT(slotUpdateHotpointList(QVector<Hotspot>&)));
   
    ui.horizontalSlide->setRange(0.0, 180.0);
    ui.horizontalSlide->setHandleValue(Handle_Bottom_Left, 0.1);
    ui.horizontalSlide->setHandleValue(Handle_Bottom_Right, 180.0);
    ui.horizontalSlide->setTopValue(60.0);
    ui.horizontalSlide->setTickInterval(0.1);
    connect(ui.horizontalSlide, SIGNAL(handleValueChanged(HandlePos, double)), this, SLOT(slotSlideValueChanged(HandlePos, double)));

    ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
    ui.pushButton_filter->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/filter_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/filter_1.png);}").arg(QApplication::applicationDirPath()));
    ui.pushButton_style->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/style_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/style_1.png);}").arg(QApplication::applicationDirPath()));
    ui.pushButton_del->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/del_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
        "QPushButton::hover{border-image:url(%1/Resource/edit/del_1.png);}").arg(QApplication::applicationDirPath()));

    
    ui.listWidget->setViewMode(QListView::IconMode);
    ui.listWidget->setSpacing(5);
    ui.listWidget->setMovement(QListView::Static);
    ui.listWidget->setFocusPolicy(Qt::ClickFocus);
    ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection); 
    ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.listWidget->setStyleSheet(
        "QListWidget{"
        "background-color: rgb(26, 26, 26 );"
        "border: 0px;"
        "border-radius: 5px;"
        "}"
        "QListWidget::item {"
        "background-color: rgb(60, 60, 60);"
        "border-radius: 5px;"
        "}"
        "QListWidget::item:hover {"
        "background-color: rgb(100, 100, 100);"
        "border-radius: 5px;"
        "}"
    );
    //如果存在，查询数据库
    if (m_levelID != "")
    {
        m_isExist = true;

        float min_zoom;
        float max_zoom;
        {
            QString data = QString("select * from level where level_id = '%1'").arg(m_levelID);
            QSqlQuery query(data);//查询表的内容
            while (query.next())
            {
                min_zoom = query.value(1).toFloat();
                max_zoom = query.value(2).toFloat();
            }
        }
        {
            QString data = QString("select * from levelshow where level_id = '%1'").arg(m_levelID);
            QSqlQuery query(data);//查询表的内容
            while (query.next())
            {
                m_vecLevelIcon.push_back(query.value(0).toString());
            }
        }
        ui.horizontalSlide->setHandleValue(Handle_Bottom_Left, min_zoom);
        ui.horizontalSlide->setHandleValue(Handle_Bottom_Right, max_zoom);
        ui.label_fov_min->setText(QString::number(min_zoom, 10, 1));
        ui.label_fov_max->setText(QString::number(max_zoom, 10, 1));
    }
    //加载热点
    addIcon2List(m_picID);
}

project_edit_hotpoint_level_set::~project_edit_hotpoint_level_set()
{}

void project_edit_hotpoint_level_set::slotClose()
{
    this->close();
    this->destroy(true);
}

void project_edit_hotpoint_level_set::addIcon2List(QString picID)
{
    m_curInsertNum = 0;
    QString data = QString("select * from hotpoint where picture_id = '%1'").arg(picID);
    QSqlQuery query(data);//查询表的内容
    while (query.next())
    {
        Hotspot hp;
        hp.iconID = query.value(0).toString();
        hp.position = QVector3D(0, 0, -1);
        QString strPos = query.value(5).toString();
        QStringList posList = strPos.split(',');
        if (posList.size() == 3)
            hp.position = QVector3D(posList[0].toFloat(), posList[1].toFloat(), posList[2].toFloat());
        hp.iconPath = query.value(3).toString();
        hp.name = query.value(1).toString();
        hp.description = "";
        hp.style = query.value(4).toString();
        hp.scale_x = query.value(6).toFloat();
        hp.scale_y = query.value(7).toFloat();
        hp.lock = query.value(8).toBool();
        hp.icon_visible = query.value(9).toBool();
        hp.title_visible = query.value(10).toBool();
        hp.isFollowSysZoom = query.value(11).toBool();
        hp.title_bg_color = QColor(30, 30, 30, 255);
        hp.title_text_color = QColor(255, 255, 255, 255);
        QStringList bgColorList = query.value(13).toString().split(",");
        QStringList textColorList = query.value(14).toString().split(",");
        int size = query.value(15).toInt();
        if (bgColorList.size() == 3)
            hp.title_bg_color = QColor(bgColorList[0].toInt(), bgColorList[1].toInt(), bgColorList[2].toInt());

        if (textColorList.size() == 3)
            hp.title_text_color = QColor(textColorList[0].toInt(), textColorList[1].toInt(), textColorList[2].toInt());
        hp.title_text_size = size;
        //
        m_vecCurPicHp.push_back(hp);
    }
    ui.label_iconCount->setText("("+ QString::number(m_vecCurPicHp.size()) +")");
    QTimer::singleShot(0, this, &project_edit_hotpoint_level_set::processNextIconItem);
}

void project_edit_hotpoint_level_set::processNextIconItem()
{
    //加载热点
    if (m_curInsertNum >= m_vecCurPicHp.size())
    {
        return;
    }
    //添加到List
    Hotspot hp = m_vecCurPicHp[m_curInsertNum];
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(230, 36));
    item->setData(Qt::UserRole, hp.iconPath);
    item->setData(Qt::UserRole + 1, hp.iconID);
    ui.listWidget->addItem(item);

    project_edit_hotpoint_level_set_item* item2 = new project_edit_hotpoint_level_set_item(hp);
    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    for (auto tmp : m_vecLevelIcon)
    {
        if (tmp == hp.iconID)
        {
            item2->setCheckStatus(true);
            break;
        }
    }
    ui.listWidget->setItemWidget(item, item2);
    m_curInsertNum++;
    // 继续处理下一个图片，保持UI响应
    QTimer::singleShot(30, this, &project_edit_hotpoint_level_set::processNextIconItem);
}

void project_edit_hotpoint_level_set::slotSlideValueChanged(HandlePos pos, double value)
{
    float zoom = 60.0 / value;
    if (pos == Handle_Top)
        ui.label_fov_cut->setText(QString::number(value,10,1));
    else if (pos == Handle_Bottom_Left)
        ui.label_fov_min->setText(QString::number(value, 10, 1));   
    else
        ui.label_fov_max->setText(QString::number(value, 10, 1));
    //
    if ((pos == Handle_Bottom_Left || pos == Handle_Bottom_Right) && (zoom > 0.5 && zoom < 3.0))
    {
        emit sig_cutZoom(zoom);
    }
       
}

void project_edit_hotpoint_level_set::slotUpdateTopButton(float value)
{
    ui.horizontalSlide->setTopValue(value);
    ui.label_fov_cut->setText(QString::number(value, 10, 1));
}

void project_edit_hotpoint_level_set::slotSearch(const QString& text)
{
    {
        if (text.size() == 0)
            ui.pushButton_del->hide();
        else
            ui.pushButton_del->show();
    }
    //查找	
    QVector<Hotspot> filterHpList;
    for (const auto& hp : m_vecCurPicHp)
    {
        if (hp.name.contains(text, Qt::CaseInsensitive))
            filterHpList.append(hp);
    }
    emit sig_UpdateHotpointList(filterHpList);
}

void project_edit_hotpoint_level_set::slotUpdateHotpointList(QVector<Hotspot>& filterHpList)
{
    slotclearHpList();
    for (auto& hp : filterHpList)
    {
        QListWidgetItem* item = new QListWidgetItem();
        item->setSizeHint(QSize(230, 36));
        item->setData(Qt::UserRole, hp.iconPath);
        item->setData(Qt::UserRole + 1, hp.iconID);
        ui.listWidget->addItem(item);

        project_edit_hotpoint_level_set_item* item2 = new project_edit_hotpoint_level_set_item(hp);
        item2->setFocusPolicy(Qt::NoFocus);
        item2->resize(QSize(230, 36));
        for (auto tmp : m_vecLevelIcon)
        {
            if (tmp == hp.iconID)
            {
                item2->setCheckStatus(true);
                break;
            }
        }

        //更新个数
        QString num = QString::number(ui.listWidget->count());
        ui.label_iconCount->setText("(" + num + ")");
    }
}

void project_edit_hotpoint_level_set::slotClearSearchLine()
{
    ui.lineEdit_search->setText("");
}

void project_edit_hotpoint_level_set::slotDone()
{
    if (ui.listWidget->count() == 0)
    {
        return;
    }
    //选择的热点
    std::vector<QString> vecIconID;
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString iconID = listItem->data(Qt::UserRole + 1).toString();
        project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
        if (item)
        {
            bool blCheck = item->getCheckStatus();
            if (blCheck == true)
            {
                vecIconID.push_back(iconID);
            }
            
        }
    }
    if (vecIconID.size() == 0)
    {
        emit sig_Msg_Set_lvSet(u8"至少选择一个热点!");
        return;
    }
    //插入数据库
    QString level_id = m_levelID;
    if (level_id == "")
        level_id = getUUID();
    else
    {
        //删除旧数据
        QString sql = QString("delete from levelshow where level_id = '%1' ").arg(m_levelID);
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!");
        }
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
    //
    std::vector<HotspotLevelShow> vecLevelShow;
    //插入新数据  
    QString min_zoom = ui.label_fov_min->text();
    QString max_zoom = ui.label_fov_max->text();
    if (m_isExist == false)
    {
        QString pic_id = m_picID;
        QString sql = QString("INSERT INTO level( \
            pic_id,\
			min_zoom, \
            max_zoom,\
            level_id\
		) VALUES (\
			'%1',\
			'%2', \
            '%3', \
            '%4' \
		); ")
            .arg(pic_id)
            .arg(min_zoom)
            .arg(max_zoom)
            .arg(level_id);
        //执行语句
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
        }
    }
    for (auto tmpID : vecIconID)
    {
        QString icon_id = tmpID;
        QString pic_id = m_picID;       
        QString sql = QString("INSERT INTO levelshow(icon_id, level_id, pic_id) VALUES ('%1','%2','%3');").arg(icon_id).arg(level_id).arg(pic_id);
        //执行语句
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
        }
        //
        HotspotLevelShow tmpLevelShow;
        tmpLevelShow.iconID = icon_id;
        tmpLevelShow.picID = "";
        tmpLevelShow.min_zoom = ui.label_fov_min->text().toFloat();
        tmpLevelShow.max_zoom = ui.label_fov_max->text().toFloat();
        tmpLevelShow.LevelID = level_id;

        vecLevelShow.push_back(tmpLevelShow);
    }  
    //重新载入
    emit sig_edit_hp_lv_set_RelaodLevel();
    if (m_isExist == false)//添加新Level
    {
        //添加到List
        emit sig_AddLevel(vecLevelShow);
    }
    else//更新Level
    {
       //更新到List
        emit sig_UpdateLevel(vecLevelShow);
    }
    emit sig_Msg_Set_lvSet(u8"保存成功!");
    slotClose();
}

void project_edit_hotpoint_level_set::slotclearHpList()
{
    if (ui.listWidget->count() == 0) return;
    for (int i = ui.listWidget->count() - 1; i >= 0; i--)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
        if (item)
        {
            delete item;
            QListWidgetItem* removedItem = ui.listWidget->takeItem(i);
            if (removedItem)
                delete removedItem;
        }
    }
}




