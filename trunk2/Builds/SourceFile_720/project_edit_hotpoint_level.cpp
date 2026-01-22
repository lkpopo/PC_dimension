#include "project_edit_hotpoint_level.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include "project_edit_hotpoint_level_item.h"

project_edit_hotpoint_level::project_edit_hotpoint_level(QString projUUID, QString picPath, QString picID, QWidget *parent)
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
        ui.listWidget->resize(241, scrHeight - 60 - 90 - 10);
    }
    connect(ui.pushButton_exit, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(slotAddLevel()));

    ui.pushButton_exit->setToolTip(u8"退出");

    ui.pushButton_exit->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/exit_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/exit_1.png);}").arg(QApplication::applicationDirPath()));
    // 1. 设置列表支持拖拽（自身项可被拖拽）
    ui.listWidget->setDragEnabled(true);
    // 2. 设置列表支持放置（允许拖拽项放入）
    ui.listWidget->setAcceptDrops(true);
    // 3. 设置拖拽模式：内部移动（仅在列表内拖拽重排，不向外拖/向内拖外部数据）
    ui.listWidget->setDragDropMode(QListWidget::InternalMove);
    // 4. 设置默认选中模式（可选，提升交互体验）
    ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    ui.listWidget->setViewMode(QListView::ListMode);
    ui.listWidget->setSpacing(5);
    ui.listWidget->setFocusPolicy(Qt::NoFocus);
    //ui.listWidget->setMovement(QListView::Static);
    //ui.listWidget->setFocusPolicy(Qt::ClickFocus);
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
        "QListWidget::item:hover {"
        "background-color: rgb(100, 100, 100);"
        "border-radius: 5px;"
        "}"
    );
    //加载层级
    addLevel2List(m_picID);
}

project_edit_hotpoint_level::~project_edit_hotpoint_level()
{}

void project_edit_hotpoint_level::addLevel2List(QString picID)
{   
    QString data = QString("select d.icon_id, u.* from level u LEFT JOIN levelshow d ON u.level_id = d.level_id where u.pic_id = '%1' ").arg(picID);
    QSqlQuery query(data);//查询表的内容
    while (query.next())
    {
        HotspotLevelShow level;  
        level.iconID = query.value(0).toString();
        level.picID = query.value(1).toString();
        level.min_zoom = query.value(2).toFloat();
        level.max_zoom = query.value(3).toFloat();
        level.LevelID = query.value(4).toString();
      
        m_mapVecLevel[level.LevelID].push_back(level);
        //
        {
            bool isExist = false;
            for (const QString& str : m_vecLevelID) { // 遍历容器中所有元素
                if (str == level.LevelID) {        // 逐元素对比
                    isExist = true;
                    break;                   // 找到后立即退出循环，优化性能
                }
            }
            if (isExist == false)
            {
                m_vecLevelID.push_back(level.LevelID);
            }
        }     
    }
    QTimer::singleShot(0, this, &project_edit_hotpoint_level::processNextLevelItem);
}

void project_edit_hotpoint_level::processNextLevelItem()
{
    //加载层次
    if (m_curInsertNum >= m_mapVecLevel.size())
    {
        return;
    }
    //添加到List
    std::vector<HotspotLevelShow> lvShow = m_mapVecLevel[m_vecLevelID[m_curInsertNum]];
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(230, 36));
    item->setData(Qt::UserRole, m_vecLevelID[m_curInsertNum]);
    ui.listWidget->addItem(item);

    project_edit_hotpoint_level_item* item2 = new project_edit_hotpoint_level_item(lvShow);
    connect(item2, SIGNAL(sig_EditLevel(QString)), this, SLOT(slotEditLevel(QString)));
    connect(item2, SIGNAL(sig_del_level_list(QString)), this, SLOT(slotDelLevelItem(QString)));
    connect(item2, SIGNAL(sig_edit_hp_lv_item_RelaodLevel()), this, SIGNAL(sig_edit_hp_lv_RelaodLevel()));
    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    ui.listWidget->setItemWidget(item, item2);
    //更新分组个数
    QString num = u8"层级" + QString::number(ui.listWidget->count());
    ui.label_levelCount->setText(num);
    m_curInsertNum++;
    // 继续处理下一个图片，保持UI响应
    QTimer::singleShot(30, this, &project_edit_hotpoint_level::processNextLevelItem);
}

void project_edit_hotpoint_level::slotClose()
{
    emit sig_closeSet();
    this->close();
    this->destroy(true);
}

void project_edit_hotpoint_level::slotAddLevel()
{
    //场景视角恢复60度
    emit sig_cut_zoom(1.0); 

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();
    if (m_levelSet != nullptr)
    {
        delete m_levelSet;
    }
    m_levelSet = new project_edit_hotpoint_level_set(m_projUUID, m_cutPicPath, m_picID, "",this);
    connect(this, SIGNAL(sig_closeSet()), m_levelSet, SLOT(slotClose()));
    connect(m_levelSet, SIGNAL(sig_Msg_Set_lvSet(QString)), this, SIGNAL(sig_Msg_Set(QString)));
    connect(this, SIGNAL(sig_update_see_angle_level(float)), m_levelSet, SLOT(slotUpdateTopButton(float)));
    connect(m_levelSet, SIGNAL(sig_cutZoom(float)), this, SIGNAL(sig_cut_zoom(float)));
    connect(m_levelSet, SIGNAL(sig_AddLevel(std::vector<HotspotLevelShow>)), this, SLOT(slotAddLevelItem(std::vector<HotspotLevelShow>)));
    connect(m_levelSet, SIGNAL(sig_UpdateLevel(std::vector<HotspotLevelShow>)), this, SIGNAL(slotUpdateLevelItem(std::vector<HotspotLevelShow>)));
    connect(m_levelSet, SIGNAL(sig_edit_hp_lv_set_RelaodLevel()), this, SIGNAL(sig_edit_hp_lv_RelaodLevel()));

    m_levelSet->move(scrWidth - 250, 60);
    m_levelSet->show();
}

void project_edit_hotpoint_level::slotEditLevel(QString levelID)
{
    //场景视角恢复60度
    emit sig_cut_zoom(1.0);

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrWidth = screenRect.width();
    int scrHeight = screenRect.height() - getBottomPix();
    if (m_levelSet != nullptr)
    {
        delete m_levelSet;
    }
    m_levelSet = new project_edit_hotpoint_level_set(m_projUUID, m_cutPicPath, m_picID, levelID, this);
    connect(this, SIGNAL(sig_closeSet()), m_levelSet, SLOT(slotClose()));
    connect(m_levelSet, SIGNAL(sig_Msg_Set_lvSet(QString)), this, SIGNAL(sig_Msg_Set(QString)));
    connect(this, SIGNAL(sig_update_see_angle_level(float)), m_levelSet, SLOT(slotUpdateTopButton(float)));
    connect(m_levelSet, SIGNAL(sig_cutZoom(float)), this, SIGNAL(sig_cut_zoom(float)));
    connect(m_levelSet, SIGNAL(sig_AddLevel(std::vector<HotspotLevelShow>)), this, SLOT(std::vector<HotspotLevelShow>));
    connect(m_levelSet, SIGNAL(sig_UpdateLevel(std::vector<HotspotLevelShow>)), this, SLOT(slotUpdateLevelItem(std::vector<HotspotLevelShow>)));
    connect(m_levelSet, SIGNAL(sig_edit_hp_lv_set_RelaodLevel()), this, SIGNAL(sig_edit_hp_lv_RelaodLevel()));
    m_levelSet->move(scrWidth - 250, 60);
    m_levelSet->show();
}

void project_edit_hotpoint_level::slotAddLevelItem(std::vector<HotspotLevelShow> vecLevel)
{
    //
    for (auto tmp : vecLevel)
    {
        m_mapVecLevel[tmp.LevelID].push_back(tmp);
    }
    //添加到List
    QListWidgetItem* item = new QListWidgetItem();
    item->setSizeHint(QSize(230, 36));
    item->setData(Qt::UserRole, vecLevel[0].LevelID);
    ui.listWidget->addItem(item);

    project_edit_hotpoint_level_item* item2 = new project_edit_hotpoint_level_item(vecLevel);
    connect(item2, SIGNAL(sig_EditLevel(QString)), this, SLOT(slotEditLevel(QString)));
    connect(item2, SIGNAL(sig_del_level_list(QString)), this, SLOT(slotDelLevelItem(QString)));
    connect(item2, SIGNAL(sig_edit_hp_lv_item_RelaodLevel()), this, SIGNAL(sig_edit_hp_lv_RelaodLevel()));
    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    ui.listWidget->setItemWidget(item, item2);
    //更新分组个数
    QString num = u8"层级" + QString::number(ui.listWidget->count());
    ui.label_levelCount->setText(num);
    //
    emit sig_edit_hp_level_addLevel(ui.listWidget->count());
}

void project_edit_hotpoint_level::slotUpdateLevelItem(std::vector<HotspotLevelShow> vecLevelShow)
{
    //更新到List
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString tmpID = listItem->data(Qt::UserRole).toString();
        if (tmpID == vecLevelShow[0].LevelID)
        {
            project_edit_hotpoint_level_item* item = qobject_cast<project_edit_hotpoint_level_item*>(ui.listWidget->itemWidget(listItem));
            if (item)
            {
                item->updateItem(vecLevelShow);
                break;
            }
        }
    }
}

void project_edit_hotpoint_level::slotDelLevelItem(QString levelID)
{
    //删除
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString tmpID = listItem->data(Qt::UserRole).toString();
        if (tmpID == levelID)
        {
            project_edit_hotpoint_level_item* item = qobject_cast<project_edit_hotpoint_level_item*>(ui.listWidget->itemWidget(listItem));
            if (item)
            {
                delete item;
                QListWidgetItem* removedItem = ui.listWidget->takeItem(i);
                if (removedItem) 
                    delete removedItem;
                
                break;
            }
        }
    }
    //更新分组个数
    QString num = u8"层级" + QString::number(ui.listWidget->count());
    ui.label_levelCount->setText(num);
}




