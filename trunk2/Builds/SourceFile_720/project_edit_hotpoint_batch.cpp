#include "project_edit_hotpoint_batch.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include "project_edit_hotpoint_level_set_item.h"

project_edit_hotpoint_batch::project_edit_hotpoint_batch(QString projUUID, QString picPath, QString picID, QWidget *parent)
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
        ui.label_bg->resize(250, scrHeight - 60);
        ui.pushButton_batch->move(10, scrHeight - 60 - 10 - 30);
        //
        ui.label_BatchOperation->move(10, scrHeight - 60 - 10 - 30 - 30 * 5);
        ui.pushButton_hide->move(10, scrHeight - 60 - 10 - 30 - 30 * 5);
        ui.pushButton_show->move(10, scrHeight - 60 - 10 - 30 - 30 * 4);
        ui.pushButton_lock->move(10, scrHeight - 60 - 10 - 30 - 30 * 3);
        ui.pushButton_unlock->move(10, scrHeight - 60 - 10 - 30 - 30 * 2);
        ui.pushButton_del->move(10, scrHeight - 60 - 10 - 30 - 30 * 1);
    }
    //隐藏
    {
        ui.label->hide();
        ui.pushButton_filter->hide();
        ui.pushButton_style->hide();

        ui.label_BatchOperation->hide();
        ui.pushButton_hide->hide();
        ui.pushButton_show->hide();
        ui.pushButton_lock->hide();
        ui.pushButton_unlock->hide();
        ui.pushButton_del->hide();
    }
    ui.pushButton_delText->hide();

    //默认没有选择，不能失效
    enableBatch(false);

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

    connect(ui.checkBox_hpList, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
    connect(ui.pushButton_exit, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_batch, SIGNAL(clicked()), this, SLOT(slotOperationShowHide()));

    connect(ui.pushButton_hide, SIGNAL(clicked()), this, SLOT(slotHide()));
    connect(ui.pushButton_show, SIGNAL(clicked()), this, SLOT(slotShow()));
    connect(ui.pushButton_lock, SIGNAL(clicked()), this, SLOT(slotLock()));
    connect(ui.pushButton_unlock, SIGNAL(clicked()), this, SLOT(slotUnlock()));
    connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDel()));

    connect(ui.pushButton_delText, SIGNAL(clicked()), this, SLOT(slotClearSearchLine()));
    connect(ui.lineEdit_search, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));
    connect(this, SIGNAL(sig_UpdateHotpointList(QVector<Hotspot>&)), this, SLOT(slotUpdateHotpointList(QVector<Hotspot>&)));


    ui.pushButton_exit->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/exit_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/exit_1.png);}").arg(QApplication::applicationDirPath()));
    ui.pushButton_filter->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/filter_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/filter_1.png);}").arg(QApplication::applicationDirPath()));
    ui.pushButton_style->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/style_0.png);border:0px;background-color: rgb(52, 52, 52);}"
        "QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/style_1.png);}").arg(QApplication::applicationDirPath()));
    ui.pushButton_delText->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/del_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
        "QPushButton::hover{border-image:url(%1/Resource/edit/del_1.png);}").arg(QApplication::applicationDirPath()));

    
    ui.pushButton_hide->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);}"
        "QPushButton::hover{background-color: rgb(120, 120, 120);}"));
    ui.pushButton_show->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);}"
        "QPushButton::hover{background-color: rgb(120, 120, 120);}"));
    ui.pushButton_lock->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);}"
        "QPushButton::hover{background-color: rgb(120, 120, 120);}"));
    ui.pushButton_unlock->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);}"
        "QPushButton::hover{background-color: rgb(120, 120, 120);}"));
    ui.pushButton_del->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(255, 255, 255);color: rgb(0, 0, 0);}"
        "QPushButton::hover{background-color: rgb(120, 120, 120);}"));
    
    //加载热点
    addIcon2List(m_picID);
}

project_edit_hotpoint_batch::~project_edit_hotpoint_batch()
{}

void project_edit_hotpoint_batch::slotClose()
{
    emit sig_closeSet();
    emit sig_batch_ReloadHp(m_picID);
    this->close();
    this->destroy(true);
}

void project_edit_hotpoint_batch::addIcon2List(QString picID)
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
    //ui.label_iconCount->setText("(" + QString::number(m_vecCurPicHp.size()) + ")");
    QTimer::singleShot(0, this, &project_edit_hotpoint_batch::processNextIconItem);
}

void project_edit_hotpoint_batch::processNextIconItem()
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
    connect(item2, SIGNAL(sig_edit_hp_batch_item_RotateCutHotPoint(QString)), this, SIGNAL(sig_edit_hp_batch_RotateCutHotPoint(QString)));
 
    item2->setFocusPolicy(Qt::NoFocus);
    item2->resize(QSize(230, 36));
    ui.listWidget->setItemWidget(item, item2);
    m_curInsertNum++;
    //
    //更新个数
    QString num = QString::number(ui.listWidget->count());
    QString numStr = "(" + num + ")";
    ui.label_iconCount->setText(numStr);
    // 继续处理下一个图片，保持UI响应
    QTimer::singleShot(30, this, &project_edit_hotpoint_batch::processNextIconItem);
}

void project_edit_hotpoint_batch::enableBatch( bool bl)
{
    if (bl == true)
    {  
        ui.pushButton_batch->setEnabled(true);
        ui.pushButton_batch->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(0, 85, 255);color: rgb(255, 255, 255);}"));
    }
    else
    {
        ui.pushButton_batch->setEnabled(false);
        ui.pushButton_batch->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(144, 144, 144);color: rgb(255, 255, 255);}"));
    }
}

void project_edit_hotpoint_batch::SlotSelList(int sel)
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

void project_edit_hotpoint_batch::slotSelStatus(int nl)
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
    //
    if (selNum == 0)
        enableBatch(false);
    else
        enableBatch(true);
}

void project_edit_hotpoint_batch::slotOperationShowHide()
{
    m_operationMenuShow = !m_operationMenuShow;
    if (m_operationMenuShow == true)
    {
        ui.label_BatchOperation->show();
        ui.pushButton_hide->show();
        ui.pushButton_show->show();
        ui.pushButton_lock->show();
        ui.pushButton_unlock->show();
        ui.pushButton_del->show();
    }
    else
    {
        ui.label_BatchOperation->hide();
        ui.pushButton_hide->hide();
        ui.pushButton_show->hide();
        ui.pushButton_lock->hide();
        ui.pushButton_unlock->hide();
        ui.pushButton_del->hide();
    }
}

void project_edit_hotpoint_batch::slotOperationShow()
{
    ui.label_BatchOperation->show();
    ui.pushButton_hide->show();
    ui.pushButton_show->show();
    ui.pushButton_lock->show();
    ui.pushButton_unlock->show();
    ui.pushButton_del->show();
}

void project_edit_hotpoint_batch::slotOperationHide()
{
    ui.label_BatchOperation->hide();
    ui.pushButton_hide->hide();
    ui.pushButton_show->hide();
    ui.pushButton_lock->hide();
    ui.pushButton_unlock->hide();
    ui.pushButton_del->hide();
}

std::vector<QString> project_edit_hotpoint_batch::getSelIconID()
{
    std::vector<QString> vecSelIconID;
    for (size_t i = 0; i < ui.listWidget->count(); i++)
    {
        QListWidgetItem* listItem = ui.listWidget->item(i);
        QString iconID = listItem->data(Qt::UserRole + 1).toString();
        project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
        if (item)
        {
            bool blCheck = item->getCheckStatus();
            if (blCheck == true)
                vecSelIconID.push_back(iconID);
        }
    }
    return vecSelIconID;
}

void project_edit_hotpoint_batch::slotHide()
{
    std::vector<QString> vecSelIconID = getSelIconID(); 
    for (auto iconID : vecSelIconID)
    {
        //场景
        emit sig_batch_setItemShow_Lock(iconID, u8"隐藏");
        //数据库
        QString updata = QString("update hotpoint set isIconShow = '0' where id = '%1'").arg(iconID);
        QSqlQuery query;
        query.exec(updata);
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
    slotOperationHide();
}

void project_edit_hotpoint_batch::slotShow()
{
    std::vector<QString> vecSelIconID = getSelIconID();
    for (auto iconID : vecSelIconID)
    {
        //场景
        emit sig_batch_setItemShow_Lock(iconID, u8"显示");
        //数据库
        QString updata = QString("update hotpoint set isIconShow = '1' where id = '%1'").arg(iconID);
        QSqlQuery query;
        query.exec(updata);
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
     slotOperationHide();
}

void project_edit_hotpoint_batch::slotLock()
{
    std::vector<QString> vecSelIconID = getSelIconID();
    for (auto iconID : vecSelIconID)
    {
        //场景
        emit sig_batch_setItemShow_Lock(iconID, u8"锁定");
        //数据库
        QString updata = QString("update hotpoint set isLock = '1' where id = '%1'").arg(iconID);
        QSqlQuery query;
        query.exec(updata);
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
    slotOperationHide();
}

void project_edit_hotpoint_batch::slotUnlock()
{
    std::vector<QString> vecSelIconID = getSelIconID();
    for (auto iconID : vecSelIconID)
    {
        //场景
        emit sig_batch_setItemShow_Lock(iconID, u8"解锁");
        //数据库
        QString updata = QString("update hotpoint set isLock = '0' where id = '%1'").arg(iconID);
        QSqlQuery query;
        query.exec(updata);
        //更新修改时间
        {
            QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
            QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
            QSqlQuery query;
            query.exec(updata);
        }
    }
    slotOperationHide();
}

void project_edit_hotpoint_batch::slotDel()
{
    std::vector<QString> vecSelIconID = getSelIconID();
    for (auto iconID : vecSelIconID)
    {
        //菜单
        for (size_t i = 0; i < ui.listWidget->count(); i++)
        {
            QListWidgetItem* listItem = ui.listWidget->item(i);
            QString tmpID = listItem->data(Qt::UserRole + 1).toString();
            if (tmpID == iconID)
            {
                project_edit_hotpoint_level_set_item* item = qobject_cast<project_edit_hotpoint_level_set_item*>(ui.listWidget->itemWidget(listItem));
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
        //场景
        emit sig_batch_edit_hp_DelIcon(iconID);
        //更新个数
        QString num = QString::number(ui.listWidget->count());
        QString strNum = "(" + num + ")";
        ui.label_iconCount->setText(strNum);
        //数据库
        {
            //热点
            QString sql = QString("delete from hotpoint where id = '%1'").arg(iconID);
            QSqlQuery query;
            query.exec(sql);
        }
        {
            //层次
            QString sql = QString("delete from levelshow where icon_id = '%1'").arg(iconID);
            QSqlQuery query;
            query.exec(sql);
        }
        {
            //分组
            QString sql = QString("delete from groupshow where icon_id = '%1'").arg(iconID);
            QSqlQuery query;
            query.exec(sql);
        }
    }
    slotOperationHide();
}

void project_edit_hotpoint_batch::slotclearHpList()
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

void project_edit_hotpoint_batch::slotSearch(const QString& text)
{
    {
        if (text.size() == 0)
            ui.pushButton_delText->hide();
        else
            ui.pushButton_delText->show();
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

void project_edit_hotpoint_batch::slotUpdateHotpointList(QVector<Hotspot>& filterHpList)
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
        connect(item2, SIGNAL(sig_edit_hp_batch_item_RotateCutHotPoint(QString)), this, SIGNAL(sig_edit_hp_batch_RotateCutHotPoint(QString)));

        item2->setFocusPolicy(Qt::NoFocus);
        item2->resize(QSize(230, 36));
        ui.listWidget->setItemWidget(item, item2);

        //更新个数
        QString num = QString::number(ui.listWidget->count());
        ui.label_iconCount->setText("(" + num + ")");
    }
}

void project_edit_hotpoint_batch::slotClearSearchLine()
{
    ui.lineEdit_search->setText("");
}
