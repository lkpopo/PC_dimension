#include "project_edit_hotpoint_level_item.h"
#include <QEnterEvent>
#include <QPushButton>
#include <QSqlQuery>
#include <QSqlError>

project_edit_hotpoint_level_item::project_edit_hotpoint_level_item(std::vector<HotspotLevelShow> vecLevel, QWidget *parent)
	: m_vecLevel(vecLevel), QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_levelID = vecLevel[0].LevelID;
	m_min_zoom = vecLevel[0].min_zoom;
	m_max_zoom = vecLevel[0].max_zoom;

	m_IconMum = 0;
	for (auto tmp : vecLevel)
	{
		if (tmp.iconID != "")
			m_IconMum++;
	}

	QString title = "FOV " + QString::number(m_min_zoom,10,1) + "-" + QString::number(m_max_zoom, 10, 1);
	ui.pushButton_title->setText(title);

	QString iconCount = QString::number(m_IconMum) + u8"热点";
	ui.pushButton_iconCount->setText(iconCount);

	connect(ui.pushButton_title, SIGNAL(clicked()), this, SLOT(slotLevelEdit()));
	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDelLevel()));

	ui.pushButton_del->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/del_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/del_1.png);}").arg(QApplication::applicationDirPath()));

}

project_edit_hotpoint_level_item::~project_edit_hotpoint_level_item()
{}

void project_edit_hotpoint_level_item::updateItem(std::vector<HotspotLevelShow> vecLevelShow)
{
	m_min_zoom = vecLevelShow[0].min_zoom;
	m_max_zoom = vecLevelShow[0].max_zoom;
	m_IconMum = vecLevelShow.size();

	QString title = "FOV " + QString::number(m_min_zoom, 10, 1) + "-" + QString::number(m_max_zoom, 10, 1);
	ui.pushButton_title->setText(title);
	QString iconCount = QString::number(m_IconMum) + u8"热点";
	ui.pushButton_iconCount->setText(iconCount);	
}

void project_edit_hotpoint_level_item::slotLevelEdit()
{
	emit sig_EditLevel(m_levelID);
}

void project_edit_hotpoint_level_item::slotChangePic()
{

}

void project_edit_hotpoint_level_item::slotDelLevel()
{
	//数据库删除
	{
		QString sql = QString("delete from level where level_id = '%1' ").arg(m_levelID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!");
		}
	}
	{
		QString sql = QString("delete from levelshow where level_id = '%1' ").arg(m_levelID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!");
		}
	}	
	//场景中删除
	emit sig_edit_hp_lv_item_RelaodLevel();
	//列表中删除
	emit sig_del_level_list(m_levelID);
}


