#include "project_edit_hotpoint_group.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QListView>
#include "project_edit_hotpoint_group_item.h"

project_edit_hotpoint_group::project_edit_hotpoint_group(QString projUUID, QString picPath, QString picID, QWidget *parent)
    :m_projUUID(projUUID), m_cutPicPath(picPath), m_picID(picID), QWidget(parent)
{
	ui.setupUi(this);
    //隐藏
    ui.pushButton_groupStyle->hide();

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_QuitOnClose, true);
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrHeight = screenRect.height() - getBottomPix();
    //动态长度
    {
        resize(250, scrHeight - 60);
        ui.listWidget->resize(241, scrHeight - 60 - 90 - 10);
        ui.label_bg->resize(250, scrHeight - 60);
        ui.pushButton_groupStyle->move(10, scrHeight - 60 - 10 - 30);
    }
    connect(ui.pushButton_exit, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(slotAddGroup()));
    connect(ui.pushButton_groupStyle, SIGNAL(clicked()), this, SLOT(slotGroupStyle()));

    ui.pushButton_exit->setToolTip(u8"退出");

    ui.pushButton_exit->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/exit_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/exit_1.png);}").arg(QApplication::applicationDirPath()));
   
    ui.listWidget->setViewMode(QListView::ListMode);
    ui.listWidget->setSpacing(5);
    ui.listWidget->setFocusPolicy(Qt::NoFocus);
    ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui.listWidget->setIconSize(QSize(0, 36)); // 高度和自定义Widget一致
    ui.listWidget->setMinimumHeight(300);
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
        "QListWidget::item:selected {"
        "    border-radius: 5px;"
        "    border: 2px solid #fa6400;"
        "}"
        "QListWidget::item:hover {"
        "background-color: rgb(100, 100, 100);"
        "border-radius: 5px;"
        "}"
    ); 
    //查找默认打开的分组
    {
        QString data = QString("select GroupID from `group` where pic_id = '%1' AND isdefOpen = '1' LIMIT 1").arg(m_picID);
        QSqlQuery query(data);//查询表的内容
        while (query.next())
        {
            m_DefopenGroupID = query.value(0).toString();
        }
    }
    //加载分组
    addGroup2List(m_picID);
}

project_edit_hotpoint_group::~project_edit_hotpoint_group()
{}

void project_edit_hotpoint_group::slotClose()
{
    emit sig_closeSet();
    this->close();
    this->destroy(true);
}


void project_edit_hotpoint_group::addGroup2List(QString picID)
{
    QString data = QString("select d.icon_id,u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.pic_id = '%1'").arg(picID);
    QSqlQuery query(data);//查询表的内容
    while (query.next())
    {
        HotspotGroupShow group;
        group.iconID = query.value(0).toString();
        group.picID = query.value(1).toString();
        group.groupName = query.value(2).toString();
        group.isGroupShowInPic = query.value(3).toBool();
        group.isPerGroupSeeAngle = query.value(4).toBool();
        group.group_rotate_x = query.value(5).toFloat();
        group.group_rotate_y = query.value(6).toFloat();
        group.gruop_zoom = query.value(7).toFloat();
        group.isToGroupZoom = query.value(8).toBool();
        group.isdefOpen = query.value(9).toBool();
        group.isIncludeAll = query.value(10).toBool();
        group.GroupID = query.value(11).toString();
       
        m_mapVecGroup[group.GroupID].push_back(group);
        //
        {
            bool isExist = false;
            for (const QString& str : m_vecGroupID) { // 遍历容器中所有元素
                if (str == group.GroupID) {        // 逐元素对比
                    isExist = true;
                    break;                   // 找到后立即退出循环，优化性能
                }
            }
            if (isExist == false)
            {
                m_vecGroupID.push_back(group.GroupID);
            }
        }
    }
    QTimer::singleShot(0, this, &project_edit_hotpoint_group::processNextGroupItem);
}

void project_edit_hotpoint_group::processNextGroupItem()
{
    //加载分组
    if (m_curInsertNum >= m_mapVecGroup.size())
    {      
        return;
    }
    //添加到List
    std::vector<HotspotGroupShow> lvShow = m_mapVecGroup[m_vecGroupID[m_curInsertNum]];
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(230, 36));
    item->setData(Qt::UserRole, m_vecGroupID[m_curInsertNum]);
    ui.listWidget->addItem(item);

    project_edit_hotpoint_group_item* item2 = new project_edit_hotpoint_group_item(lvShow);
    connect(item2, SIGNAL(sig_EditGroup(QString)), this, SLOT(slotEditGroup(QString)));
    connect(item2, SIGNAL(sig_edit_hp_group_set_SetSelStyle(QString)), this, SIGNAL(sig_edit_hp_group_SetSelStyle(QString)));
    connect(item2, SIGNAL(sig_edit_hp_group_item_rotatezoom(GroupSeeAngleZoom)), this, SIGNAL(sig_edit_hp_group_rotatezoom(GroupSeeAngleZoom)));
    connect(item2, SIGNAL(sig_edit_hp_group_item_GroupAngleWin(int)), this, SIGNAL(sig_edit_hp_group_GroupAngleWin(int)));
    connect(item2, SIGNAL(sig_edit_hp_gp_item_RelaodGroup(QString)), this, SIGNAL(sig_edit_hp_gp_RelaodGroup(QString)));
    connect(item2, SIGNAL(sig_del_group_list(QString, bool)), this, SLOT(slotDelGroupItem(QString, bool)));
    connect(item2, SIGNAL(sig_setFirstOpen(QString)), this, SLOT(slotSetFirstOpen(QString)));

    item2->resize(QSize(230, 36));
    ui.listWidget->setItemWidget(item, item2);

    if (m_vecGroupID[m_curInsertNum] == m_DefopenGroupID)
    {
        item->setSelected(true);
        item2->setFocus();
        //默认打开
        item2->setDefOpen(true);
        emit sig_edit_hp_group_SetSelStyle(m_DefopenGroupID);
    } 
    //更新分组个数
    QString num = u8"分组" + QString::number(ui.listWidget->count());
    ui.label_groupNum->setText(num);
    //
    m_curInsertNum++;
    // 继续处理下一个图片，保持UI响应
    QTimer::singleShot(30, this, &project_edit_hotpoint_group::processNextGroupItem);
}

void project_edit_hotpoint_group::slotAddGroup()
{
    //edit场景中添加
    GroupItemStyle gpStyle;
    gpStyle.groupName = u8"未命名";
    gpStyle.groupID = getUUID();
    gpStyle.unsel_bgColor = "";
    gpStyle.unsel_textColor = "";
    gpStyle.sel_bgColor = "";
    gpStyle.sel_textColor = "";
    emit sig_edit_hp_group_addGroupItem(gpStyle);
    emit sig_edit_hp_group_SetSelStyle(gpStyle.groupID);
    //
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();
    if (m_groupSet != nullptr)
    {
        delete m_groupSet;
    }
    m_groupSet = new project_edit_hotpoint_group_set(m_projUUID, m_cutPicPath,m_picID, u8"未命名", gpStyle.groupID, false, this);
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_SetSelStyle(QString)), this, SIGNAL(sig_edit_hp_group_SetSelStyle(QString)));
    connect(this, SIGNAL(sig_closeSet()), m_groupSet, SLOT(slotClose()));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_groupName(QString, const QString&)), this, SIGNAL(sig_edit_hp_group_groupName(QString, const QString&)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_setGroupItemHide(QString, bool)), this, SIGNAL(sig_edit_hp_group_setGroupItemHide(QString, bool)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_GroupAngleWin(int)), this, SIGNAL(sig_edit_hp_group_GroupAngleWin(int)));
    connect(this, SIGNAL(sig_edit_hp_group_GroupSeeAngleZoom(GroupSeeAngleZoom)), m_groupSet, SLOT(SlotGroupSeeAngleZoom(GroupSeeAngleZoom)));
    connect(m_groupSet, SIGNAL(sig_Msg_Set_lvSet(QString)), this, SIGNAL(sig_Msg_Set(QString)));
    connect(m_groupSet, SIGNAL(sig_AddGroup(std::vector<HotspotGroupShow>)), this, SLOT(SlotAddGroupItem(std::vector<HotspotGroupShow>)));
    connect(m_groupSet, SIGNAL(sig_UpdateGroup(std::vector<HotspotGroupShow>)), this, SLOT(SlotUpdateGroupItem(std::vector<HotspotGroupShow>)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_DelTmpGroup(QString)), this, SIGNAL(sig_edit_hp_group_DelTmpGroup(QString)));
    connect(m_groupSet, SIGNAL(sig_ReturnDefSel()), this, SLOT(SlotReturnDefSel()));

    m_groupSet->move(scrWidth - 250, 60);
    m_groupSet->show();
}

void project_edit_hotpoint_group::slotGroupStyle()
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();
    if (m_style != nullptr)
    {
        delete m_style;
    }
    m_style = new project_edit_hotpoint_group_ResetGroupStyle(m_projUUID, m_picID,this);

    m_style->move(scrWidth - 250, 60);
    m_style->show();
}

void project_edit_hotpoint_group::slotEditGroup(QString groupID)
{
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();
    if (m_groupSet != nullptr)
    {
        delete m_groupSet;
    }
    m_groupSet = new project_edit_hotpoint_group_set(m_projUUID, m_cutPicPath, m_picID, u8"", groupID, true, this);
    connect(this, SIGNAL(sig_closeSet()), m_groupSet, SLOT(slotClose()));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_SetSelStyle(QString)), this, SIGNAL(sig_edit_hp_group_SetSelStyle(QString)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_groupName(QString, const QString&)), this, SIGNAL(sig_edit_hp_group_groupName(QString, const QString&)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_setGroupItemHide(QString, bool)), this, SIGNAL(sig_edit_hp_group_setGroupItemHide(QString, bool)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_GroupAngleWin(int)), this, SIGNAL(sig_edit_hp_group_GroupAngleWin(int)));
    connect(this, SIGNAL(sig_edit_hp_group_GroupSeeAngleZoom(GroupSeeAngleZoom)), m_groupSet, SLOT(SlotGroupSeeAngleZoom(GroupSeeAngleZoom)));
    connect(m_groupSet, SIGNAL(sig_Msg_Set_lvSet(QString)), this, SIGNAL(sig_Msg_Set(QString)));
    connect(m_groupSet, SIGNAL(sig_AddGroup(std::vector<HotspotGroupShow>)), this, SLOT(SlotAddGroupItem(std::vector<HotspotGroupShow>)));
    connect(m_groupSet, SIGNAL(sig_UpdateGroup(std::vector<HotspotGroupShow>)), this, SLOT(SlotUpdateGroupItem(std::vector<HotspotGroupShow>)));
    connect(m_groupSet, SIGNAL(sig_edit_hp_group_set_DelTmpGroup(QString)), this, SIGNAL(sig_edit_hp_group_DelTmpGroup(QString)));
    connect(m_groupSet, SIGNAL(sig_ReturnDefSel()), this, SLOT(SlotReturnDefSel()));
    m_groupSet->move(scrWidth - 250, 60);
    m_groupSet->show();
}

void project_edit_hotpoint_group::SlotAddGroupItem(std::vector<HotspotGroupShow> vecGroup)
{
    //
    for (auto tmp : vecGroup)
    {
        m_mapVecGroup[tmp.GroupID].push_back(tmp);
    }
    //添加到List
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(230, 36));
    item->setData(Qt::UserRole, vecGroup[0].GroupID);
    ui.listWidget->addItem(item);

    project_edit_hotpoint_group_item* item2 = new project_edit_hotpoint_group_item(vecGroup);
    connect(item2, SIGNAL(sig_EditGroup(QString)), this, SLOT(slotEditGroup(QString)));
    connect(item2, SIGNAL(sig_edit_hp_group_set_SetSelStyle(QString)), this, SIGNAL(sig_edit_hp_group_SetSelStyle(QString)));
    connect(item2, SIGNAL(sig_edit_hp_group_item_rotatezoom(GroupSeeAngleZoom)), this, SIGNAL(sig_edit_hp_group_rotatezoom(GroupSeeAngleZoom)));
    connect(item2, SIGNAL(sig_edit_hp_group_item_GroupAngleWin(int)), this, SIGNAL(sig_edit_hp_group_GroupAngleWin(int)));
    connect(item2, SIGNAL(sig_edit_hp_gp_item_RelaodGroup(QString)), this, SIGNAL(sig_edit_hp_gp_RelaodGroup(QString)));
    connect(item2, SIGNAL(sig_del_group_list(QString, bool)), this, SLOT(slotDelGroupItem(QString, bool)));
    connect(item2, SIGNAL(sig_setFirstOpen(QString)), this, SLOT(slotSetFirstOpen(QString)));

    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    ui.listWidget->setItemWidget(item, item2);
    //更新分组个数
    QString num = u8"分组" + QString::number(ui.listWidget->count());
    ui.label_groupNum->setText(num);
}

void project_edit_hotpoint_group::SlotUpdateGroupItem(std::vector<HotspotGroupShow> vecGroupShow)
{
    //更新到List
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString tmpID = listItem->data(Qt::UserRole).toString();
        if (tmpID == vecGroupShow[0].GroupID)
        {
            project_edit_hotpoint_group_item* item = qobject_cast<project_edit_hotpoint_group_item*>(ui.listWidget->itemWidget(listItem));
            if (item)
            {
                item->updateItem(vecGroupShow);
                break;
            }
        }
    }
}

void project_edit_hotpoint_group::slotDelGroupItem(QString groupID, bool blDefOpen)
{
    //删除项
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString tmpID = listItem->data(Qt::UserRole).toString();
        project_edit_hotpoint_group_item* item = qobject_cast<project_edit_hotpoint_group_item*>(ui.listWidget->itemWidget(listItem));
        if (tmpID == groupID)
        {        
            if (item)
            {
                delete item;
                QListWidgetItem* removedItem = ui.listWidget->takeItem(i);
                if (removedItem)
                    delete removedItem;

                break;
            }
        }
        else
        {
            if (item)
            {
                listItem->setSelected(false);
                item->clearFocus();
            }    
        }
    }
    //如果默认项删除了，其余列表项失去焦点
    if (blDefOpen == true)
    {
        for (size_t i = 0; i < ui.listWidget->count(); i++)
        {
            QListWidgetItem* listItem = ui.listWidget->item(i);
            QString tmpID = listItem->data(Qt::UserRole).toString();
            project_edit_hotpoint_group_item* item = qobject_cast<project_edit_hotpoint_group_item*>(ui.listWidget->itemWidget(listItem));
            if (item)
            {
                listItem->setSelected(false);
                item->clearFocus();
            }
        }
    }
  
    //编辑场景中删除
    emit sig_edit_hp_group_DelTmpGroup(groupID);
    //更新分组个数
    QString num = u8"分组" + QString::number(ui.listWidget->count());
    ui.label_groupNum->setText(num);
}

void project_edit_hotpoint_group::slotSetFirstOpen(QString groupID)
{
    //样式
    for (int i = 0; i < ui.listWidget->count(); ++i)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString tmpID = listItem->data(Qt::UserRole).toString();
        project_edit_hotpoint_group_item* customItem = qobject_cast<project_edit_hotpoint_group_item*>(ui.listWidget->itemWidget(listItem));
        if (tmpID != groupID)
        {
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
                "QListWidget::item:selected {"
                "    border-radius: 5px;"
                "}"
                "QListWidget::item:hover {"
                "background-color: rgb(100, 100, 100);"
                "border-radius: 5px;"
                "}"
            );
            customItem->setDefOpen(false);
            customItem->clearFocus();
            continue;
        }
        else
        {
            if (customItem)
            {
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
                    "QListWidget::item:selected {"
                    "    border-radius: 5px;"
                    "    border: 2px solid #fa6400;"
                    "}"
                    "QListWidget::item:hover {"
                    "background-color: rgb(100, 100, 100);"
                    "border-radius: 5px;"
                    "}"
                );

                customItem->setDefOpen(true);
                customItem->setFocus();
                break;
            }
        }    
    }
    //修改数据库
    {
        if (groupID != "")
        {
            QString updata1 = QString("update `group` set isdefOpen = '1' where pic_id = '%1' AND GroupID = '%2'").arg(m_picID).arg(groupID);
            QSqlQuery query1;
            query1.exec(updata1);

            QString updata2 = QString("update `group` set isdefOpen = '0' where pic_id = '%1' AND GroupID != '%2'").arg(m_picID).arg(groupID);
            QSqlQuery query2;
            query2.exec(updata2);
        }
        else
        {
            QString updata = QString("update `group` set isdefOpen = '0' where pic_id = '%1'").arg(m_picID);
            QSqlQuery query;
            query.exec(updata);
        }
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
}

void project_edit_hotpoint_group::SlotReturnDefSel()
{
    if (m_DefopenGroupID != "")
        emit sig_edit_hp_group_SetSelStyle(m_DefopenGroupID);
}

