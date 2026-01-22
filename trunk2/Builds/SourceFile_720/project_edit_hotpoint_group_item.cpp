#include "project_edit_hotpoint_group_item.h"
#include <QSqlQuery>
#include <QSqlError>



project_edit_hotpoint_group_item::project_edit_hotpoint_group_item(std::vector<HotspotGroupShow> lvShow, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_groupID = lvShow[0].GroupID;
	m_isPerGroupSeeAngle = lvShow[0].isPerGroupSeeAngle;
	m_group_rotate_x = lvShow[0].group_rotate_x;
	m_group_rotate_y = lvShow[0].group_rotate_y;
	m_gruop_zoom = lvShow[0].gruop_zoom;
	m_isToGroupZoom = lvShow[0].isToGroupZoom;

	ui.pushButton_title->setText(lvShow[0].groupName);

	int iconCt = 0;
	for (auto tmp : lvShow)
	{
		if (tmp.iconID != "")
			iconCt++;
	}
	QString iconCount = QString::number(iconCt) + u8"热点";
	ui.pushButton_iconCount->setText(iconCount);

	ui.pushButton_defOpen->setToolTip(u8"设为默认打开");
	ui.pushButton_del->setToolTip(u8"删除");

	connect(ui.pushButton_title, SIGNAL(clicked()), this, SLOT(slotGroupEdit()));
	connect(ui.pushButton_defOpen, SIGNAL(clicked()), this, SLOT(slotDeflOpen()));
	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDelGroup()));

	ui.pushButton_defOpen->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/defaultOpen_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/defaultOpen_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_del->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/del_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/del_1.png);}").arg(QApplication::applicationDirPath()));
}

project_edit_hotpoint_group_item::~project_edit_hotpoint_group_item()
{}

void project_edit_hotpoint_group_item::updateItem(std::vector<HotspotGroupShow> vecGroupShow)
{
	m_groupID = vecGroupShow[0].GroupID;
	m_isPerGroupSeeAngle = vecGroupShow[0].isPerGroupSeeAngle;
	m_group_rotate_x = vecGroupShow[0].group_rotate_x;
	m_group_rotate_y = vecGroupShow[0].group_rotate_y;
	m_gruop_zoom = vecGroupShow[0].gruop_zoom;
	m_isToGroupZoom = vecGroupShow[0].isToGroupZoom;

	ui.pushButton_title->setText(vecGroupShow[0].groupName);
	QString iconCount = QString::number(vecGroupShow.size()) + u8"热点";
	ui.pushButton_iconCount->setText(iconCount);
}

void project_edit_hotpoint_group_item::slotGroupEdit()
{
	//旋转到分组视角
	if (m_isPerGroupSeeAngle == true)
	{
		GroupSeeAngleZoom angZoom;
		angZoom.rotation = QVector3D(m_group_rotate_x, m_group_rotate_y, 0);
		angZoom.zoom = m_gruop_zoom;
		angZoom.isToGroupZoom = m_isToGroupZoom;
		//视角缩放
		emit sig_edit_hp_group_item_rotatezoom(angZoom);
		//显示分组视图窗口
		emit sig_edit_hp_group_item_GroupAngleWin(2);
	}
	//分组Set窗口
	emit sig_EditGroup(m_groupID);
	//编辑窗口分组变色,场景赋值选中分组
	emit sig_edit_hp_group_set_SetSelStyle(m_groupID);
}

void project_edit_hotpoint_group_item::slotDefGroup()
{
	//旋转到分组视角
	if (m_isPerGroupSeeAngle == true)
	{
		GroupSeeAngleZoom angZoom;
		angZoom.rotation = QVector3D(m_group_rotate_x, m_group_rotate_y, 0);
		angZoom.zoom = m_gruop_zoom;
		angZoom.isToGroupZoom = m_isToGroupZoom;
		//视角缩放
		emit sig_edit_hp_group_item_rotatezoom(angZoom);
	}
	//分组Set窗口
	emit sig_EditGroup(m_groupID);
	//编辑窗口分组变色,场景赋值选中分组
	emit sig_edit_hp_group_set_SetSelStyle(m_groupID);
}


void project_edit_hotpoint_group_item::slotDeflOpen()
{
	if (m_blDefOpen == true)
	{
		m_blDefOpen = false;
		//选中样式，修改数据库
		emit sig_setFirstOpen("");
		//编辑窗口分组变色,场景赋值选中分组
		emit sig_edit_hp_group_set_SetSelStyle("");
	}
	else
	{
		m_blDefOpen = true;
		//选中样式，修改数据库
		emit sig_setFirstOpen(m_groupID);
		//编辑窗口分组变色,场景赋值选中分组
		emit sig_edit_hp_group_set_SetSelStyle(m_groupID);
	}
	
}

void project_edit_hotpoint_group_item::slotDelGroup()
{
	//数据库删除
	{
		QString sql = QString("delete from `group` where GroupID = '%1'").arg(m_groupID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!");
		}
	}
	{
		QString sql = QString("delete from groupshow where GroupID = '%1'").arg(m_groupID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!");
		}
	}
	
	//场景中删除
	emit sig_edit_hp_gp_item_RelaodGroup(m_groupID);
	//列表中删除
	emit sig_del_group_list(m_groupID,m_blDefOpen);
}
