#include "project_edit_hotpoint_group_set.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include "PanoramaWidget.h"
#include "project_edit_hotpoint_level_set_item.h"


project_edit_hotpoint_group_set::project_edit_hotpoint_group_set(QString projUUID, QString picPath, QString picID, QString groupName, QString groupID,bool isExist, QWidget *parent)
    :m_projUUID(projUUID), m_cutPicPath(picPath), m_picID(picID),m_groupName(groupName), m_groupID(groupID), m_isExist(isExist), QWidget(parent)
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
        ui.label_bottom_bg->resize(231, scrHeight - 60 - 430 - 10 - 2 * 10 - 30);
        ui.listWidget->resize(251, scrHeight - 60 - 600 - 10 - 2 * 10 - 30);
        ui.pushButton_done->move(10, scrHeight - 60 - 10 - 30);
    }

    ui.checkBox_groupShow->setChecked(true);
    ui.checkBox_blGroupAngle->setChecked(false);
    ui.checkBox_blSeeAngleZoom->setChecked(false);

    //隐藏
    {
        ui.label->hide();
        ui.pushButton_filter->hide();
        ui.pushButton_style->hide();
    }

    ui.pushButton_del->hide();

    ui.comboBox->addItem(u8"指定热点");
    //ui.comboBox->addItem(u8"所有热点");
    ui.comboBox->setCurrentIndex(0);
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(SlotHPComboBoxShow(int)));
    
    connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));
    connect(ui.lineEdit_groupName, SIGNAL(textChanged(const QString&)), this, SLOT(slot_edit_hp_group_set_groupName(const QString&)));

    connect(ui.checkBox_groupShow, SIGNAL(stateChanged(int)), this, SLOT(SlotGroupItemHide(int)));
    connect(ui.checkBox_blGroupAngle, SIGNAL(stateChanged(int)), this, SLOT(Slot_GroupAngleWin(int)));

    connect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
 
    connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotClearSearchLine()));
    connect(ui.lineEdit_search, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));
    connect(this, SIGNAL(sig_UpdateHotpointList(QVector<Hotspot>&)), this, SLOT(slotUpdateHotpointList(QVector<Hotspot>&)));


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
    if (m_isExist == true)
    {

        m_isIncludeAll = false;
        HotspotGroupShow hpGroupShow;
        QString data = QString("select d.icon_id,u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.GroupID = '%1'").arg(m_groupID);
        QSqlQuery query(data);//查询表的内容
        while (query.next())
        {
            m_vecGroupIcon.push_back(query.value(0).toString());

            hpGroupShow.iconID = query.value(0).toString();
            hpGroupShow.picID = query.value(1).toString();
            hpGroupShow.groupName = query.value(2).toString();
            hpGroupShow.isGroupShowInPic = query.value(3).toBool();
            hpGroupShow.isPerGroupSeeAngle = query.value(4).toBool();
            hpGroupShow.group_rotate_x = query.value(5).toFloat();
            hpGroupShow.group_rotate_y = query.value(6).toFloat();
            hpGroupShow.gruop_zoom = query.value(7).toFloat();
            hpGroupShow.isToGroupZoom = query.value(8).toBool();
            hpGroupShow.isdefOpen = query.value(9).toBool();
            hpGroupShow.isIncludeAll = query.value(10).toBool();
            hpGroupShow.GroupID = query.value(11).toString();

            //原始旋转数据
            m_gsAngZm.rotation = QVector3D(query.value(5).toFloat(), query.value(6).toFloat(),0);
            m_gsAngZm.zoom = query.value(7).toFloat();
            m_gsAngZm.isToGroupZoom = query.value(8).toBool();
        }
        ui.lineEdit_groupName->setText(hpGroupShow.groupName);
        ui.checkBox_groupShow->setChecked(hpGroupShow.isGroupShowInPic);
        ui.checkBox_blGroupAngle->setChecked(hpGroupShow.isPerGroupSeeAngle);
        //分组视角
        QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
        QString Groups = dirName + "/Groups/";
        QFileInfo fileInfo(m_cutPicPath);
        QString suf = fileInfo.completeSuffix();
        QString groupAngleZoomPic = Groups + m_groupID + "." + suf;
        QImage image(groupAngleZoomPic);
        if (hpGroupShow.isPerGroupSeeAngle == true && !image.isNull())
        { 
            QPixmap pixmap = QPixmap::fromImage(image);
            ui.label_SeeAngle->setPixmap(pixmap);
            //显示分组视图
            emit sig_edit_hp_group_set_GroupAngleWin(2);
        }       
        //
        ui.checkBox_blSeeAngleZoom->setChecked(hpGroupShow.isToGroupZoom);
        ui.comboBox->setCurrentIndex(hpGroupShow.isIncludeAll);
        if (hpGroupShow.isIncludeAll == true)
            m_isIncludeAll = true;
    }
    //加载热点
    addIcon2List(m_picID);
}

project_edit_hotpoint_group_set::~project_edit_hotpoint_group_set()
{}

void project_edit_hotpoint_group_set::slotClose()
{
    //恢复都不选择
    emit sig_edit_hp_group_set_SetSelStyle("");
    //删除临时组
    if(m_isExist == false)
        emit sig_edit_hp_group_set_DelTmpGroup(m_groupID);   
    //隐藏视角框线
    emit sig_edit_hp_group_set_GroupAngleWin(0);
    //返回默认选择
    emit sig_ReturnDefSel();
    this->close();
    this->destroy(true);
}

void project_edit_hotpoint_group_set::addIcon2List(QString picID)
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
    if (m_isIncludeAll == true)
    {
        ui.lineEdit_search->hide();
        ui.label->hide();
        ui.pushButton_filter->hide();
        ui.pushButton_style->hide();
        ui.checkBox_hpList->hide();
        ui.label_iconCount->hide();
        ui.listWidget->hide();
        ui.label_10->show();
        ui.label_bottom_bg->resize(231,161);
    }
    else
    {
        ui.lineEdit_search->show();
        ui.label->show();
        ui.pushButton_filter->show();
        ui.pushButton_style->show();
        ui.checkBox_hpList->show();
        ui.label_iconCount->show();
        ui.listWidget->show();
        ui.label_10->hide();

        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenRect = screen->geometry();
        int scrHeight = screenRect.height() - getBottomPix();
        ui.label_bottom_bg->resize(231, scrHeight - 60 - 450 - 10 - 2 * 10 - 30);

        ui.label_iconCount->setText("(" + QString::number(m_vecCurPicHp.size()) + ")");
        QTimer::singleShot(0, this, &project_edit_hotpoint_group_set::processNextIconItem);
    }   
}

void project_edit_hotpoint_group_set::processNextIconItem()
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
    connect(item2, SIGNAL(sig_SelStatus(int)), this, SLOT(slotSelStatus(int)));
    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    for (auto tmp : m_vecGroupIcon)
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
    QTimer::singleShot(30, this, &project_edit_hotpoint_group_set::processNextIconItem);
}

void project_edit_hotpoint_group_set::slot_edit_hp_group_set_groupName(const QString& text)
{
    emit sig_edit_hp_group_set_groupName(m_groupID, text);
}

void project_edit_hotpoint_group_set::SlotSelList(int sel)
{
    int tmp = 0;
    if (sel == 2)
        tmp = 1;
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString iconID = listItem->data(Qt::UserRole + 1).toString();
        project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
        if (item)
        {
            item->setCheckStatus(tmp);             
        }
    }
    
}

void project_edit_hotpoint_group_set::SlotGroupItemHide(int iHide)
{
    emit sig_edit_hp_group_set_setGroupItemHide(m_groupID, ui.checkBox_groupShow->isChecked());
}

void project_edit_hotpoint_group_set::SlotGroupSeeAngleZoom(GroupSeeAngleZoom gsAngZm)
{
    m_gsAngZm = gsAngZm;
    QPixmap fitpixmap = gsAngZm.pixmap.scaled(210, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    ui.label_SeeAngle->setPixmap(fitpixmap);
    //保存图片
    {
        QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
        QString Groups = dirName + "/Groups/";
        QDir dir(Groups);
        if (!dir.exists())
        {
            dir.mkdir(Groups);
        }
        QFileInfo fileInfo(m_cutPicPath);
        QString suf = fileInfo.completeSuffix();
        QString groupAngleZoomPic = Groups + m_groupID + "." + suf;
        bool isSaved = fitpixmap.save(groupAngleZoomPic);
    }
}

void project_edit_hotpoint_group_set::SlotHPComboBoxShow(int index)
{
    if (index == 1)
    {
        ui.lineEdit_search->hide();
        ui.label->hide();
        ui.pushButton_filter->hide();
        ui.pushButton_style->hide();
        ui.checkBox_hpList->hide();
        ui.label_iconCount->hide();
        ui.listWidget->hide();
        ui.label_10->show();
        ui.label_bottom_bg->resize(231, 161);
    }
    else
    {
        ui.lineEdit_search->show();
        ui.label->show();
        ui.pushButton_filter->show();
        ui.pushButton_style->show();
        ui.checkBox_hpList->show();
        ui.label_iconCount->show();
        ui.listWidget->show();
        ui.label_10->hide();

        QScreen* screen = QGuiApplication::primaryScreen();
        QRect screenRect = screen->geometry();
        int scrHeight = screenRect.height() - getBottomPix();
        ui.label_bottom_bg->resize(231, scrHeight - 60 - 450 - 10 - 2 * 10 - 30);
        ui.label_iconCount->setText("(" + QString::number(m_vecCurPicHp.size()) + ")");
    }
}

void project_edit_hotpoint_group_set::Slot_GroupAngleWin(int bl)
{
    //删除
    if (bl == false)
    {
        QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
        QString Groups = dirName + "/Groups/";
        QFileInfo fileInfo(m_cutPicPath);
        QString suf = fileInfo.completeSuffix();
        QString groupAngleZoomPic = Groups + m_groupID + "." + suf;
        LPCWSTR winImagePath = (LPCWSTR)groupAngleZoomPic.utf16();
        BOOL isDeleted = DeleteFileW(winImagePath);

        QPixmap pixmap;
        ui.label_SeeAngle->setPixmap(pixmap);
        ui.checkBox_blSeeAngleZoom->setChecked(false);
    }
   //显示分组视图
    emit sig_edit_hp_group_set_GroupAngleWin(bl);
}

void project_edit_hotpoint_group_set::slotSelStatus(int nl)
{
    int selNum = 0;
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString iconID = listItem->data(Qt::UserRole + 1).toString();
        project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
        if (item)
        {
            bool blCheck = item->getCheckStatus();
            if (blCheck == true)
                selNum++;
        }
    }
    if (selNum == ui.listWidget->count())
    {
        disconnect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
        ui.checkBox_hpList->setChecked(true);
        connect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
    }
    else
    {
        disconnect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
        ui.checkBox_hpList->setChecked(false);
        connect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
    }
}

void project_edit_hotpoint_group_set::slotSearch(const QString& text)
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

void project_edit_hotpoint_group_set::slotUpdateHotpointList(QVector<Hotspot>& filterHpList)
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
        connect(item2, SIGNAL(sig_SelStatus(int)), this, SLOT(slotSelStatus(int)));
        item2->setFocusPolicy(Qt::NoFocus);
        item2->resize(QSize(230, 36));
        for (auto tmp : m_vecGroupIcon)
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

void project_edit_hotpoint_group_set::slotClearSearchLine()
{
    ui.lineEdit_search->setText("");
}

void project_edit_hotpoint_group_set::slotclearHpList()
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

void project_edit_hotpoint_group_set::slotDone()
{
    if (ui.listWidget->count() == 0)
    {
        return;
    }
    if (ui.checkBox_blGroupAngle->isChecked() == true)
    {
        QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
        QString Groups = dirName + "/Groups/";
        QFileInfo fileInfo(m_cutPicPath);
        QString suf = fileInfo.completeSuffix();
        QString groupAngleZoomPic = Groups + m_groupID + "." + suf;
        // 判断文件是否存在
        QFileInfo fileInfo2(groupAngleZoomPic);
        bool isExist = fileInfo2.exists();
        if (!isExist)
        {
            emit sig_Msg_Set_lvSet(u8"请设置分组视角!");
            return;
        }      
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
    if (ui.comboBox->currentIndex() == 0 && vecIconID.size() == 0)
    {
        emit sig_Msg_Set_lvSet(u8"至少选择一个热点!");
        return;
    }
    //插入数据库
    if (m_isExist == true)
    {
        //删除旧数据
        QString sql = QString("delete from groupshow where GroupID = '%1' ").arg(m_groupID);
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!");
        }
    }
    //
    std::vector<HotspotGroupShow> vecGroupShow;
    //插入新数据
    int isIncludeAll = ui.comboBox->currentIndex();
    QString rotation_x = QString::number(m_gsAngZm.rotation.x(), 10, 1);
    QString rotation_y = QString::number(m_gsAngZm.rotation.y(), 10, 1);
    QString zoom = QString::number(m_gsAngZm.zoom, 10, 1);
    if (rotation_x == "0.0")
        rotation_x = "";
    if (rotation_y == "0.0")
        rotation_y = "";
    if (zoom == "0.0")
        zoom = "";
    if (m_isExist == false)
    {
        QString pic_id = m_picID;
        bool isdefOpen = false;
        QString sql = QString("INSERT INTO `group`( \
            pic_id,\
			group_name, \
            isGroupShowInPic,\
            isPerGroupSeeAngle, \
            group_rotate_x,\
            group_rotate_y, \
            gruop_zoom,\
            isToGroupZoom, \
            isdefOpen,\
            isIncludeAll,\
            GroupID\
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
            '%10', \
            '%11' \
		); ")
            .arg(pic_id)
            .arg(ui.lineEdit_groupName->text())
            .arg(ui.checkBox_groupShow->isChecked())
            .arg(ui.checkBox_blGroupAngle->isChecked())
            .arg(rotation_x)
            .arg(rotation_y)
            .arg(zoom)
            .arg(ui.checkBox_blSeeAngleZoom->isChecked())
            .arg(isdefOpen)
            .arg(isIncludeAll)
            .arg(m_groupID);
        //执行语句
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
        }
    }
    else
    {
        //更新
        QString pic_id = m_picID;
        QString sql = QString("UPDATE `group` set \
				            pic_id = '%1', \
				            group_name = '%2', \
				            isGroupShowInPic = '%3',\
                            isPerGroupSeeAngle = '%4',\
				            group_rotate_x = '%5',\
				            group_rotate_y = '%6',\
				            gruop_zoom = '%7',\
				            isToGroupZoom = '%8',\
				            isIncludeAll = '%9' WHERE\
                            GroupID = '%10'")
            .arg(pic_id)
            .arg(ui.lineEdit_groupName->text())
            .arg(ui.checkBox_groupShow->isChecked())
            .arg(ui.checkBox_blGroupAngle->isChecked())
            .arg(rotation_x)
            .arg(rotation_y)
            .arg(zoom)
            .arg(ui.checkBox_blSeeAngleZoom->isChecked())
            .arg(isIncludeAll)
            .arg(m_groupID);
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
        bool isdefOpen = false;
        QString sql = QString("INSERT INTO groupshow(icon_id, GroupID, pic_id) VALUES ('%1','%2','%3');").arg(icon_id).arg(m_groupID).arg(pic_id);
        //执行语句
        QSqlQuery query;
        if (!query.exec(sql))
        {
            qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
        }
        //
        HotspotGroupShow tmpGroupShow;
        tmpGroupShow.iconID = icon_id;
        tmpGroupShow.picID = pic_id;
        tmpGroupShow.groupName = ui.lineEdit_groupName->text();
        tmpGroupShow.isGroupShowInPic = ui.checkBox_groupShow->isChecked();
        tmpGroupShow.isPerGroupSeeAngle = ui.checkBox_blGroupAngle->isChecked();
        tmpGroupShow.group_rotate_x = rotation_x.toFloat();
        tmpGroupShow.group_rotate_y = rotation_y.toFloat();
        tmpGroupShow.gruop_zoom = zoom.toFloat();
        tmpGroupShow.isToGroupZoom = ui.checkBox_blSeeAngleZoom->isChecked();
        tmpGroupShow.isdefOpen = false;
        tmpGroupShow.isPerGroupSeeAngle = ui.checkBox_blGroupAngle->isChecked();   
        tmpGroupShow.GroupID = m_groupID;
        tmpGroupShow.isIncludeAll = isIncludeAll;
        vecGroupShow.push_back(tmpGroupShow);
    }
    if (m_isExist == false)
    {
        //添加到List
        emit sig_AddGroup(vecGroupShow);
    }
    else
    {
        //更新到List
        emit sig_UpdateGroup(vecGroupShow);
    }
    emit sig_Msg_Set_lvSet(u8"保存成功!");
    //恢复都不选择
    emit sig_edit_hp_group_set_SetSelStyle("");
    //隐藏视角框线
    emit sig_edit_hp_group_set_GroupAngleWin(0);
    //返回默认选择
    emit sig_ReturnDefSel();
    this->close();
    this->destroy(true);
}

