#include "project_edit_hotpoint.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include "..\..\Include_720\project_edit_hotpoint.h"
#include "PanoramaWidget.h"
#include "project_edit_hotpoint_item.h"
#include <QtConcurrent/QtConcurrentRun>
#include <QLayout>
#include <QLayoutItem>
#include <QListWidgetItem>

project_edit_hotpoint::project_edit_hotpoint(QString projUUID, QString picPath, QString picID, QWidget *parent)
	:m_projUUID(projUUID), m_cutPicPath(picPath), m_picID(picID), QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_hpSet = nullptr;

	//隐藏
	{
		ui.label->hide();
		ui.pushButton_filter->hide();
		ui.pushButton_style->hide();
	}
	ui.pushButton_del->hide();

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	//动态长度
	{
		resize(250, scrHeight - 60);
		ui.label_bg->resize(250, scrHeight - 60);
		ui.listWidget->resize(241, scrHeight - 60 - 130 - 10);
	}
	ui.listWidget->setViewMode(QListView::ListMode);
	ui.listWidget->setSpacing(5);
	ui.listWidget->setFocusPolicy(Qt::NoFocus);
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

	connect(ui.pushButton_ceng, SIGNAL(clicked()), this, SLOT(slotAddLevel()));
	connect(ui.pushButton_group, SIGNAL(clicked()), this, SLOT(slotAddGroup()));
	connect(ui.pushButton_pi, SIGNAL(clicked()), this, SLOT(slotPiSetPic()));

	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotClearSearchLine()));
	connect(ui.lineEdit_search, SIGNAL(textChanged(const QString&)), this, SLOT(slotSearch(const QString&)));

	connect(this, SIGNAL(sig_UpdateHotpointList(QVector<Hotspot>&)), this, SLOT(slotUpdateHotpointList(QVector<Hotspot>&)));

	ui.pushButton_ceng->setToolTip(u8"热点层级");
	ui.pushButton_group->setToolTip(u8"热点分组");
	ui.pushButton_pi->setToolTip(u8"批量处理");

	connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(slotAddHotPoint()));

	ui.pushButton_filter->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/filter_0.png);border:0px;background-color: rgb(52, 52, 52);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/filter_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_style->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/style_0.png);border:0px;background-color: rgb(52, 52, 52);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/style_1.png);}").arg(QApplication::applicationDirPath()));

	ui.pushButton_ceng->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/ceng_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/ceng_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_group->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/group_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/group_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_pi->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/pi_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/pi_1.png);}").arg(QApplication::applicationDirPath()));

	ui.pushButton_del->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/del_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/edit/del_1.png);}").arg(QApplication::applicationDirPath()));

	//加载热点
	addIcon2List(m_picID);
}

project_edit_hotpoint::~project_edit_hotpoint()
{}

void project_edit_hotpoint::addIcon2List(QString picID)
{
	m_vecCurPicHp.clear();
	clearListWidget();
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
	QTimer::singleShot(0, this, &project_edit_hotpoint::processNextIconItem);
}

void project_edit_hotpoint::slotSearch(const QString& text)
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

void project_edit_hotpoint::slotUpdateHotpointList(QVector<Hotspot>& filterHpList)
{
	slotclearHpList();
	for (auto& hp : filterHpList)
	{
		QListWidgetItem* item = new QListWidgetItem();
		item->setSizeHint(QSize(230, 36));
		item->setData(Qt::UserRole, hp.iconPath);
		item->setData(Qt::UserRole + 1, hp.iconID);
		ui.listWidget->addItem(item);

		project_edit_hotpoint_item* item2 = new project_edit_hotpoint_item(hp);
		connect(item2, SIGNAL(sig_CopyIcon(Hotspot)), this, SIGNAL(sig_copy_icon(Hotspot)));
		connect(item2, SIGNAL(sig_insertNew(QString, Hotspot)), this, SLOT(slot_insert_new(QString, Hotspot)));
		connect(item2, SIGNAL(sig_editIcon(QString)), this, SLOT(slot_edit_icon(QString)));
		connect(item2, SIGNAL(sig_operate(QString, QString)), this, SLOT(slot_Operation(QString, QString)));
		connect(this, SIGNAL(sig_setMenuItemText(QString, QString, QString)), item2, SLOT(slot_SetMenuItemText(QString, QString, QString)));

		item2->setFocusPolicy(Qt::NoFocus);
		item2->resize(QSize(230, 36));
		ui.listWidget->setItemWidget(item, item2);

		//更新个数
		QString num = QString::number(ui.listWidget->count());
		ui.label_hpCount->setText(num);
	}
}

void project_edit_hotpoint::slotClearSearchLine()
{
	ui.lineEdit_search->setText("");
}

void project_edit_hotpoint::search(QString text)
{
	for (int i = 0; i < m_vecCurPicHp.size(); i++)
	{
		if (m_vecCurPicHp[i].name.contains(text))
		{
			QListWidgetItem* item = new QListWidgetItem();
			item->setSizeHint(QSize(230, 36));
			item->setData(Qt::UserRole, m_vecCurPicHp[i].iconPath);
			item->setData(Qt::UserRole + 1, m_vecCurPicHp[i].iconID);
			ui.listWidget->addItem(item);

			project_edit_hotpoint_item* item2 = new project_edit_hotpoint_item(m_vecCurPicHp[i]);
			connect(item2, SIGNAL(sig_CopyIcon(Hotspot)), this, SIGNAL(sig_copy_icon(Hotspot)));
			connect(item2, SIGNAL(sig_insertNew(QString, Hotspot)), this, SLOT(slot_insert_new(QString, Hotspot)));
			connect(item2, SIGNAL(sig_editIcon(QString)), this, SLOT(slot_edit_icon(QString)));
			connect(item2, SIGNAL(sig_operate(QString, QString)), this, SLOT(slot_Operation(QString, QString)));
			connect(this, SIGNAL(sig_setMenuItemText(QString, QString, QString)), item2, SLOT(slot_SetMenuItemText(QString, QString, QString)));

			item2->setFocusPolicy(Qt::NoFocus);
			item2->resize(QSize(230, 36));
			ui.listWidget->setItemWidget(item, item2);
		}
	}
}

void project_edit_hotpoint::processNextIconItem()
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

	project_edit_hotpoint_item* item2 = new project_edit_hotpoint_item(hp);
	connect(item2, SIGNAL(sig_CopyIcon(Hotspot)), this, SIGNAL(sig_copy_icon(Hotspot)));
	connect(item2, SIGNAL(sig_insertNew(QString, Hotspot)), this, SLOT(slot_insert_new(QString, Hotspot)));
	connect(item2, SIGNAL(sig_editIcon(QString)), this, SLOT(slot_edit_icon(QString)));
	connect(item2, SIGNAL(sig_operate(QString, QString)), this, SLOT(slot_Operation(QString, QString)));
	connect(this, SIGNAL(sig_setMenuItemText(QString, QString, QString)), item2, SLOT(slot_SetMenuItemText(QString, QString, QString)));

	item2->setFocusPolicy(Qt::NoFocus);
	item2->resize(QSize(230, 36));
	ui.listWidget->setItemWidget(item, item2);
	//更新个数
	QString num = QString::number(ui.listWidget->count());
	ui.label_hpCount->setText(num);

	m_curInsertNum++;
	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit_hotpoint::processNextIconItem);
}

void project_edit_hotpoint::clearListWidget()
{
	// 1. 遍历所有项（从后往前删，避免索引错乱）
	for (int i = ui.listWidget->count() - 1; i >= 0; --i) {
		// 步骤1：取出当前项（takeItem会将项从列表中移除，但不会删除内存）
		QListWidgetItem* item = ui.listWidget->takeItem(i);
		if (item == nullptr) {
			continue; // 极端情况：项为空，跳过
		}

		// 步骤2：获取绑定的自定义控件并清理
		project_edit_hotpoint_item* customItem =
			qobject_cast<project_edit_hotpoint_item*>(ui.listWidget->itemWidget(item));
		if (customItem) {
			customItem->disconnect(); // 断开所有信号槽，避免野指针触发信号
			delete customItem; // 显式删除自定义控件（核心：释放内存）
			customItem = nullptr;
		}

		// 步骤3：删除QListWidgetItem本身
		delete item;
		item = nullptr;
	}
	// 2. 清空列表的内部缓存（兜底，确保无残留）
	ui.listWidget->clear();
}

void project_edit_hotpoint::slotAddHotPoint()
{
	emit sig_edit_hp_SetHPLocation(true);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_hpSet != nullptr)
	{
		emit sig_del_tmp_icon();
		delete m_hpSet;
	}
	m_hpSet = new project_edit_hotpoint_set("", this);
	connect(m_hpSet, SIGNAL(sig_Msg_Set(QString)), this, SIGNAL(sig_Msg(QString)));
	connect(m_hpSet, SIGNAL(sig_addIconSet(Hotspot)), this, SIGNAL(sig_addIcon(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_updateIconSet(QString)), this, SIGNAL(sig_updateIcon(QString)));
	connect(m_hpSet, SIGNAL(sig_setScaleX(float)), this, SIGNAL(sig_scale_x(float)));
	connect(m_hpSet, SIGNAL(sig_setScaleY(float)), this, SIGNAL(sig_scale_y(float)));
	connect(m_hpSet, SIGNAL(sig_setIconName(QString)), this, SIGNAL(sig_name(QString)));
	connect(m_hpSet, SIGNAL(sig_IconNameShow(bool)), this, SIGNAL(sig_name_show(bool)));
	connect(m_hpSet, SIGNAL(sig_AddIconEnd()), this, SIGNAL(sig_add_icon_end()));
	connect(m_hpSet, SIGNAL(sig_AddIcon(Hotspot)), this, SLOT(slot_add_icon_list(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_UpdateIcon(Hotspot)), this, SLOT(slot_update_icon(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_DelTmpIcon()), this, SIGNAL(sig_del_tmp_icon()));
	connect(m_hpSet, SIGNAL(sig_setTitleBgColor(QColor)), this, SIGNAL(sig_set_titleBg_color(QColor)));
	connect(m_hpSet, SIGNAL(sig_setTitleTextColor(QColor)), this, SIGNAL(sig_set_titleText_color(QColor)));
	connect(m_hpSet, SIGNAL(sig_setTextSize(int)), this, SIGNAL(sig_set_titleText_Size(int)));
	connect(m_hpSet, SIGNAL(sig_clearFocus()), this, SLOT(slot_clear_focus()));
	connect(this, SIGNAL(sig_close()), m_hpSet, SLOT(slotClose()));
	connect(m_hpSet, SIGNAL(sig_edit_hp_set_SetHPLocation(bool)), this, SIGNAL(sig_edit_hp_SetHPLocation(bool)));
	m_hpSet->move(scrWidth - 250, 60);
	m_hpSet->showNormal();
}
void project_edit_hotpoint::slot_add_icon_list(Hotspot hp)
{
	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(230, 36));
	item->setData(Qt::UserRole, hp.iconPath);
	item->setData(Qt::UserRole + 1, hp.iconID);
	ui.listWidget->addItem(item);

	project_edit_hotpoint_item* item2 = new project_edit_hotpoint_item(hp);
	connect(item2, SIGNAL(sig_CopyIcon(Hotspot)), this, SIGNAL(sig_copy_icon(Hotspot)));
	connect(item2, SIGNAL(sig_insertNew(QString, Hotspot)), this, SLOT(slot_insert_new(QString, Hotspot)));
	connect(item2, SIGNAL(sig_editIcon(QString)), this, SLOT(slot_edit_icon(QString)));
	connect(item2, SIGNAL(sig_operate(QString, QString)), this, SLOT(slot_Operation(QString, QString)));
	connect(this, SIGNAL(sig_setMenuItemText(QString,QString, QString)), item2, SLOT(slot_SetMenuItemText(QString,QString, QString)));

	item2->setFocusPolicy(Qt::NoFocus);
	item2->resize(QSize(230, 36));
	ui.listWidget->setItemWidget(item, item2);

	ui.listWidget->setCurrentItem(item);
	ui.listWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);
	ui.listWidget->clearSelection();

}

void project_edit_hotpoint::slot_update_icon(Hotspot hp)
{
	for (size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString iconID = listItem->data(Qt::UserRole + 1).toString();
		if (iconID == hp.iconID)
		{
			project_edit_hotpoint_item* item = qobject_cast<project_edit_hotpoint_item*>(ui.listWidget->itemWidget(listItem));
			if (item)
			{
				item->updateItem(hp);
				break;
			}
		}
	}
	//刷新
	if (ui.lineEdit_search->text() != "")
		slotSearch(ui.lineEdit_search->text());
}

void project_edit_hotpoint::slot_insert_new(QString oldUUID, Hotspot newHp)
{
	for (size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString iconID = listItem->data(Qt::UserRole + 1).toString();
		if (iconID == oldUUID)
		{
			int insertIndex = i + 1;
			QListWidgetItem* item = new QListWidgetItem();
			item->setSizeHint(QSize(230, 36));
			item->setData(Qt::UserRole, newHp.iconPath);
			item->setData(Qt::UserRole + 1, newHp.iconID);
			insertIndex = qBound(0, insertIndex, ui.listWidget->count());
			ui.listWidget->insertItem(insertIndex, item);

			project_edit_hotpoint_item* item2 = new project_edit_hotpoint_item(newHp);
			connect(item2, SIGNAL(sig_CopyIcon(Hotspot)), this, SIGNAL(sig_copy_icon(Hotspot)));
			connect(item2, SIGNAL(sig_insertNew(QString, Hotspot)), this, SLOT(slot_insert_new(QString, Hotspot)));
			connect(item2, SIGNAL(sig_editIcon(QString)), this, SLOT(slot_edit_icon(QString)));
			connect(item2, SIGNAL(sig_operate(QString, QString)), this, SLOT(slot_Operation(QString, QString)));
			connect(this, SIGNAL(sig_setMenuItemText(QString, QString, QString)), item2, SLOT(slot_SetMenuItemText(QString, QString, QString)));

			item2->setFocusPolicy(Qt::NoFocus);
			item2->resize(QSize(230, 36));
			ui.listWidget->setItemWidget(item, item2);

			ui.listWidget->setCurrentItem(item);
			ui.listWidget->scrollToItem(item, QAbstractItemView::EnsureVisible);

			break;
		}
	}
	//
	m_vecCurPicHp.push_back(newHp);
}

void project_edit_hotpoint::slot_edit_icon(QString iconID)
{
	emit sig_edit_hp_SetHPLocation(true);
	//设置界面
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_hpSet != nullptr)
	{		
		delete m_hpSet;
	}
	//
	emit sig_edit_copyTo_tmp_icon(iconID);

	m_hpSet = new project_edit_hotpoint_set(iconID, this);
	connect(m_hpSet, SIGNAL(sig_Msg_Set(QString)), this, SIGNAL(sig_Msg(QString)));
	connect(m_hpSet, SIGNAL(sig_addIconSet(Hotspot)), this, SIGNAL(sig_addIcon(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_updateIconSet(QString)), this, SIGNAL(sig_updateIcon(QString)));
	connect(m_hpSet, SIGNAL(sig_setScaleX(float)), this, SIGNAL(sig_scale_x(float)));
	connect(m_hpSet, SIGNAL(sig_setScaleY(float)), this, SIGNAL(sig_scale_y(float)));
	connect(m_hpSet, SIGNAL(sig_setIconName(QString)), this, SIGNAL(sig_name(QString)));
	connect(m_hpSet, SIGNAL(sig_IconNameShow(bool)), this, SIGNAL(sig_name_show(bool)));
	connect(m_hpSet, SIGNAL(sig_AddIconEnd()), this, SIGNAL(sig_add_icon_end()));
	connect(m_hpSet, SIGNAL(sig_EditIconEnd()), this, SIGNAL(sig_edit_icon_end()));
	connect(m_hpSet, SIGNAL(sig_AddIcon(Hotspot)), this, SLOT(slot_add_icon(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_UpdateIcon(Hotspot)), this, SLOT(slot_update_icon(Hotspot)));
	connect(m_hpSet, SIGNAL(sig_DelTmpIcon()), this, SIGNAL(sig_del_tmp_icon()));
	connect(m_hpSet, SIGNAL(sig_setTitleBgColor(QColor)), this, SIGNAL(sig_set_titleBg_color(QColor)));
	connect(m_hpSet, SIGNAL(sig_setTitleTextColor(QColor)), this, SIGNAL(sig_set_titleText_color(QColor)));
	connect(m_hpSet, SIGNAL(sig_setTextSize(int)), this, SIGNAL(sig_set_titleText_Size(int)));	
	connect(this, SIGNAL(sig_close()), m_hpSet, SLOT(slotClose()));
	connect(m_hpSet, SIGNAL(sig_edit_hp_set_SetHPLocation(bool)), this, SIGNAL(sig_edit_hp_SetHPLocation(bool)));
	m_hpSet->move(scrWidth - 250, 60);
	m_hpSet->showNormal();
}

void project_edit_hotpoint::slotAddLevel()
{
	emit sig_edit_hp_SetHPLocation(false);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_level != nullptr)
	{
		delete m_level;
	}
	m_level = new project_edit_hotpoint_level(m_projUUID, m_cutPicPath, m_picID,this);
	connect(m_level, SIGNAL(sig_edit_hp_level_addLevel(int)), this, SIGNAL(sig_edit_hp_addLevel(int)));
	connect(m_level, SIGNAL(sig_Msg_Set(QString)), this, SIGNAL(sig_Msg(QString)));
	connect(this, SIGNAL(sig_close()), m_level, SLOT(slotClose()));
	connect(this, SIGNAL(sig_update_see_angle(float)), m_level, SIGNAL(sig_update_see_angle_level(float)));
	connect(m_level, SIGNAL(sig_cut_zoom(float)), this, SIGNAL(sig_cut_zoom_hp(float)));
	connect(m_level, SIGNAL(sig_edit_hp_lv_RelaodLevel()), this, SIGNAL(sig_edit_hp_RelaodLevel()));
	m_level->move(scrWidth - 250, 60);
	m_level->showNormal();
}

void project_edit_hotpoint::slotAddGroup()
{
	emit sig_edit_hp_SetHPLocation(false);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_group != nullptr)
	{
		delete m_group;
	}
	m_group = new project_edit_hotpoint_group(m_projUUID, m_cutPicPath, m_picID, this);
	connect(this, SIGNAL(sig_close()), m_group, SLOT(slotClose()));
	connect(m_group, SIGNAL(sig_Msg_Set(QString)), this, SIGNAL(sig_Msg(QString)));
	connect(m_group, SIGNAL(sig_edit_hp_group_addGroupItem(GroupItemStyle)), this, SIGNAL(sig_edit_hp_addGroupItem(GroupItemStyle)));
	connect(m_group, SIGNAL(sig_edit_hp_group_groupName(QString, const QString&)), this, SIGNAL(sig_edit_hp_groupName(QString, const QString&)));
	connect(m_group, SIGNAL(sig_edit_hp_group_setGroupItemHide(QString, bool)), this, SIGNAL(sig_edit_hp_setGroupItemHide(QString, bool)));
	connect(m_group, SIGNAL(sig_edit_hp_group_GroupAngleWin(int)), this, SIGNAL(sig_edit_hp_GroupAngleWin(int)));
	connect(this, SIGNAL(sig_edit_hp_GroupSeeAngleZoom(GroupSeeAngleZoom)), m_group, SIGNAL(sig_edit_hp_group_GroupSeeAngleZoom(GroupSeeAngleZoom)));
	connect(m_group, SIGNAL(sig_edit_hp_group_rotatezoom(GroupSeeAngleZoom)), this, SIGNAL(sig_edit_hp_rotatezoom(GroupSeeAngleZoom)));
	connect(m_group, SIGNAL(sig_edit_hp_group_DelTmpGroup(QString)), this, SIGNAL(sig_edit_hp_DelTmpGroup(QString)));
	connect(m_group, SIGNAL(sig_edit_hp_group_SetSelStyle(QString)), this, SIGNAL(sig_edit_hp_SetSelStyle(QString)));
	connect(m_group, SIGNAL(sig_edit_hp_gp_RelaodGroup(QString)), this, SIGNAL(sig_edit_hp_RelaodGroup(QString)));
	m_group->move(scrWidth - 250, 60);
	m_group->showNormal();
}

void project_edit_hotpoint::slotPiSetPic()
{
	emit sig_edit_hp_SetHPLocation(false);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_batch != nullptr)
	{
		delete m_batch;
	}
	m_batch = new project_edit_hotpoint_batch(m_projUUID, m_cutPicPath, m_picID, this);
	connect(this, SIGNAL(sig_close()), m_batch, SLOT(slotClose()));
	connect(m_batch, SIGNAL(sig_batch_setItemShow_Lock(QString,QString)), this, SIGNAL(sig_setItemShow_Lock(QString, QString)));
	connect(m_batch, SIGNAL(sig_batch_edit_hp_DelIcon(QString)), this, SIGNAL(sig_edit_hp_DelIcon(QString)));
	connect(m_batch, SIGNAL(sig_batch_ReloadHp(QString)), this, SLOT(addIcon2List(QString)));
	connect(m_batch, SIGNAL(sig_edit_hp_batch_RotateCutHotPoint(QString)), this, SIGNAL(sig_edit_hp_RotateCutHotPoint(QString)));

	m_batch->move(scrWidth - 250, 60);
	m_batch->showNormal();
}

void project_edit_hotpoint::slot_Operation(QString iconID, QString opera)
{
	//锁定，隐藏与删除
	if (opera == u8"隐藏" || opera == u8"显示")
	{
		if (opera == u8"隐藏")
		{
			emit sig_setMenuItemText(iconID, u8"隐藏", u8"显示");
			//场景
			emit sig_setItemShow_Lock(iconID, u8"隐藏");
			//数据库
			QString updata = QString("update hotpoint set isIconShow = '0' where id = '%1'").arg(iconID);
			QSqlQuery query;
			query.exec(updata);
		}
		else//显示
		{
			emit sig_setMenuItemText(iconID, u8"显示", u8"隐藏");
			//场景
			emit sig_setItemShow_Lock(iconID, u8"显示");
			//数据库
			QString updata = QString("update hotpoint set isIconShow = '1' where id = '%1'").arg(iconID);
			QSqlQuery query;
			query.exec(updata);
		}
		
	}
	else if(opera == u8"锁定" || opera == u8"解锁")
	{
		if (opera == u8"锁定")
		{
			emit sig_setMenuItemText(iconID, u8"锁定", u8"解锁");
			//场景
			emit sig_setItemShow_Lock(iconID, u8"锁定");
			//数据库
			QString updata = QString("update hotpoint set isLock = '1' where id = '%1'").arg(iconID);
			QSqlQuery query;
			query.exec(updata);
		}
		else //解锁
		{
			emit sig_setMenuItemText(iconID, u8"解锁", u8"锁定");
			//场景
			emit sig_setItemShow_Lock(iconID, u8"解锁");
			//数据库
			QString updata = QString("update hotpoint set isLock = '0' where id = '%1'").arg(iconID);
			QSqlQuery query;
			query.exec(updata);
		}
		
	}
	else//删除
	{
		//成员变量
		for (auto it = m_vecCurPicHp.begin(); it != m_vecCurPicHp.end(); ++it) 
		{
			if (it->iconID == iconID) 
			{ 
				m_vecCurPicHp.erase(it);
				break; 
			}
		}
		//菜单
		for (size_t i = 0; i < ui.listWidget->count(); i++)
		{
			QListWidgetItem* listItem = ui.listWidget->item(i);
			QString tmpID = listItem->data(Qt::UserRole + 1).toString();
			if (tmpID == iconID)
			{
				project_edit_hotpoint_item* item = qobject_cast<project_edit_hotpoint_item*>(ui.listWidget->itemWidget(listItem));
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
		emit sig_edit_hp_DelIcon(iconID);
		//更新个数
		QString num = QString::number(ui.listWidget->count());
		ui.label_hpCount->setText(num);
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
}

void project_edit_hotpoint::slotDelIcon(QString iconID)
{
	int e = 0;
}

void project_edit_hotpoint::slotclearHpList()
{
	if (ui.listWidget->count() == 0) return;
	for (int i = ui.listWidget->count() - 1; i >= 0; i--)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		project_edit_hotpoint_item* item = qobject_cast<project_edit_hotpoint_item*>(ui.listWidget->itemWidget(listItem));
		if (item)
		{
			delete item;
			QListWidgetItem* removedItem = ui.listWidget->takeItem(i);
			if (removedItem)
				delete removedItem;
		}		
	}
}

void project_edit_hotpoint::slotTest(QString ee)
{
	emit sig_Test(ee);
	emit sig_edit_hp_RotateCutHotPoint(ee);
}


