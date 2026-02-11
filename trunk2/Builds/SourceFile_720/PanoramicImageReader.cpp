#include "PanoramicImageReader.h"
#include "CommonTool.h"
#include <QScreen>
#include <QTextCodec>
#include <QtConcurrent/QtConcurrentRun>
#include <fstream>
#include <string>
#include <QSqlQuery>
#include <QSqlError>

#include "styleSheet.h"
#include "project_item.h"
#include "..\..\Include_720\PanoramicImageReader.h"

int homePage_top1_height = 36;
int homePage_top2_height = 36;
int homePage_left_wight = 100;


PanoramicImageReader::PanoramicImageReader(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->availableGeometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height();

	ui.listWidget_project->setSpacing(10);
	m_msgBox = nullptr;

	//初始化窗口
	initWindows();

	//connect
	connect(ui.pushButton_new, SIGNAL(clicked()), this, SLOT(slotNew()));
	connect(ui.pushButton_pidel, SIGNAL(clicked()), this, SLOT(slotBatchDel()));
	connect(ui.pushButton_sort, SIGNAL(clicked()), this, SLOT(slotSort()));  //分类排序功能


	int offset = 100; // 向左移动的距离
	ui.pushButton_new->move(scrWidth - 120 * 2 - 100 - offset, 20);
	ui.pushButton_pidel->move(scrWidth - 120 - 100 - offset, 20);
	ui.pushButton_sort->move(scrWidth - 100 - offset, 20);

	ui.listWidget_project->move(0, 69);
	ui.listWidget_project->resize(scrWidth - 80, scrHeight - 70);

	//创建表
	create_linename_sqlite();
	create_project_sqlite();
	create_picture_sqlite();
	create_hotpoint_sqlite();
	create_ruler_sqlite();
	create_level_sqlite();
	create_group_sqlite();
	create_levelshow_sqlite();
	create_groupshow_sqlite();
	create_mask_sqlite();

	// 窗口显示后延迟初始化列表
	QString data = QString("select * from project order by create_time desc");
	QSqlQuery query(data);//查询表的内容
	m_allQuery = query;
	m_allQuery.last();    
	m_ProjCount = m_allQuery.at() + 1;
	
	m_currentProjIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载作品...", u8"取消", 0, m_ProjCount, this);
	m_progressDialog->setWindowTitle(u8"请稍等");
	m_progressDialog->setStyleSheet("QProgressDialog {"
		"background-color: white;"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"padding: 10px;"
		"}");

	ui.pushButton_new->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_pidel->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(210, 110, 25);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(245, 110, 25);}"));
	ui.pushButton_sort->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(100, 200, 100);color: rgb(255, 210, 255);}"
		"QPushButton::hover{background-color: rgb(85, 235, 85);}"));



	m_progressDialog->move(screenRect.width() / 2 - 50, screenRect.height() / 2 - 20);

	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setWindowFlags(Qt::FramelessWindowHint);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimumDuration(0);
	QTimer::singleShot(0,this, &PanoramicImageReader::initListWidget);
}

PanoramicImageReader::~PanoramicImageReader()
{}

void PanoramicImageReader::initListWidget()
{
	if (m_currentProjIndex >= m_ProjCount || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();

		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentProjIndex);
	}

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	if (m_allQuery.seek(m_currentProjIndex)) 
	{
		QString project_id = m_allQuery.value(0).toString();
		QString project_name = m_allQuery.value(1).toString();
		QString line_name = m_allQuery.value(2).toString();
		int picture_count = m_allQuery.value(5).toInt();
		QString front_cover = m_allQuery.value(6).toString();
		QString create_time = m_allQuery.value(7).toString();
		QString update_time = m_allQuery.value(8).toString();
		QString classifyType = m_allQuery.value(9).toString();

		QListWidgetItem* item = new QListWidgetItem(ui.listWidget_project);
		item->setSizeHint(QSize(scrWidth - 205, 100));
		item->setData(Qt::UserRole, project_id);
		ui.listWidget_project->addItem(item);
		project_item* item2 = new project_item(front_cover, project_id,project_name, line_name, picture_count,create_time, update_time, classifyType);
		item2->resize(QSize(scrWidth - 25, 100));
		connect(item2, SIGNAL(sigDel(QString)), this, SLOT(slotDelProj(QString)));
		ui.listWidget_project->setItemWidget(item, item2);
	}
	m_currentProjIndex++;
	// 继续处理下一个项目，保持UI响应
	QTimer::singleShot(10, this, &PanoramicImageReader::initListWidget);
}

void PanoramicImageReader::initListWidgetPure()
{
	if (m_currentProjIndex >= m_ProjCount)
		return;
	
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	if (m_allQuery.seek(m_currentProjIndex))
	{
		QString project_id = m_allQuery.value(0).toString();
		QString project_name = m_allQuery.value(1).toString();
		QString line_name = m_allQuery.value(2).toString();
		int picture_count = m_allQuery.value(5).toInt();
		QString front_cover = m_allQuery.value(6).toString();
		QString create_time = m_allQuery.value(7).toString();
		QString update_time = m_allQuery.value(8).toString();
		QString classifyType = m_allQuery.value(9).toString();

		QListWidgetItem* item = new QListWidgetItem(ui.listWidget_project);
		item->setSizeHint(QSize(scrWidth - 205, 100));
		item->setData(Qt::UserRole, project_id);
		ui.listWidget_project->addItem(item);
		project_item* item2 = new project_item(front_cover, project_id, project_name, line_name, picture_count, create_time, update_time, classifyType);
		item2->resize(QSize(scrWidth - 25, 100));
		connect(item2, SIGNAL(sigDel(QString)), this, SLOT(slotDelProj(QString)));
		ui.listWidget_project->setItemWidget(item, item2);
	}
	m_currentProjIndex++;
	// 继续处理下一个项目，保持UI响应
	QTimer::singleShot(10, this, &PanoramicImageReader::initListWidgetPure);
}

void PanoramicImageReader::slotClose()
{
	//删除tmp文件夹
	QString tmpFolder = QString("%1/tmp").arg(QApplication::applicationDirPath());
	QDir dir;
	dir.setPath(tmpFolder);
	dir.removeRecursively();

	this->close();
	this->destroy(true);
}

void PanoramicImageReader::slotEdit(QString projUUID)
{
	for (int i = 0; i < ui.listWidget_project->count(); i++) 
	{
		QListWidgetItem* pitem = ui.listWidget_project->item(i);
		QString tmpUUID = pitem->data(Qt::UserRole).toString();
		if (tmpUUID != projUUID) continue;
		QWidget* pwig = ui.listWidget_project->itemWidget(pitem);
		QList<QPushButton*> clickedButtons = pwig->findChildren<QPushButton*>();
		for(auto btn : clickedButtons)
		{
			if (btn->text() == u8"编辑")
			{
				btn->click();
				break;
			}			
		}
		break;
	}
}

void PanoramicImageReader::slotSee(QString projUUID)
{
	for (int i = 0; i < ui.listWidget_project->count(); i++)
	{
		QListWidgetItem* pitem = ui.listWidget_project->item(i);
		QString tmpUUID = pitem->data(Qt::UserRole).toString();
		if (tmpUUID != projUUID) continue;
		QWidget* pwig = ui.listWidget_project->itemWidget(pitem);
		QList<QPushButton*> clickedButtons = pwig->findChildren<QPushButton*>();
		for (auto btn : clickedButtons)
		{
			if (btn->text() == u8"浏览")
			{
				btn->click();
				break;
			}
		}
		break;
	}
}

void PanoramicImageReader::slotNew()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	project_new* project = new project_new();
	project->setWindowModality(Qt::ApplicationModal);
	project->move((scrWidth - project->width()) / 2, (scrHeight - project->height()) / 2);
	connect(project, SIGNAL(sig_Update()), this, SLOT(slotUpdateProjList()));
	connect(project, SIGNAL(sig_Edit(QString)), this, SLOT(slotEdit(QString)));
	connect(project, SIGNAL(sig_See(QString)), this, SLOT(slotSee(QString)));
	connect(project, SIGNAL(sig_New()), this, SLOT(slotNew()));
	project->show();
}

void PanoramicImageReader::slotBatchDel()
{
	if (ui.listWidget_project->count() == 0)
	{
		noticeWin.Notice(this, u8"当前无任何项目可删除！", 1000);
		return;
	}

	int deleteCount = 0;
	for (int i = 0; i < ui.listWidget_project->count(); i++)
	{
		QListWidgetItem* selectedItem = ui.listWidget_project->item(i);
		if (!selectedItem) {
			noticeWin.Notice(this, u8"未选中任何项目！", 1000);
			return;
		}
		project_item* projItem = dynamic_cast<project_item*>(ui.listWidget_project->itemWidget(selectedItem));
		if (!projItem) {
			return;
		}
		bool isChecked = projItem->isCheckBoxChecked();
		QString projUUID = selectedItem->data(Qt::UserRole).toString();
		if (isChecked == true)
		{
			if (m_msgBox)
			{
				delete m_msgBox;
			}
			m_msgBox = new MsgBox(3, u8"确认要删除吗?", this);
			if (m_msgBox->exec() == QDialog::Rejected) return;
			// 删除作品记录
			{
				QString sql = QString("delete from project where id = '%1'").arg(projUUID);
				QSqlQuery query;
				if (!query.exec(sql))
				{
					qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
				}
			}
			{
				QString sql = QString("delete from picture where project_name_id = '%1'").arg(projUUID);
				QSqlQuery query;
				if (!query.exec(sql))
				{
					qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
				}
			}
			{
				QString sql = QString("delete from hotpoint where project_id = '%1'").arg(projUUID);
				QSqlQuery query;
				if (!query.exec(sql))
				{
					qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
				}
			}
			//删除文件夹
			{
				QString dirName = QApplication::applicationDirPath() + "/DataBase/" + projUUID;
				QDir dir(dirName);
				bool success = dir.removeRecursively();
			}
			deleteCount++;
		}


	}
	if (deleteCount == 0)
	{
		noticeWin.Notice(this, u8"未选中任何项目！", 1000);
		return;
	}
	else {
		noticeWin.Notice(this, QString(u8"已删除 %1 个项目").arg(deleteCount), 1000);	
	}
	//刷新
	UpdateList();
}
QSize project_item::sizeHint() const
{
	// 根据你的 UI 内容返回合适尺寸,因为未使用布局，这里需要手动设置一个合适的大小
	return QSize(600, 80); // 宽600，高80
}
//分类排序功能
void PanoramicImageReader::slotSort()
{
	// 清空现有列表
	ui.listWidget_project->clear();

	// 查询所有项目，按 classifyType 和 project_name 排序
	QString sql = "SELECT id, project_name, line_name, keyword, introduction, "
		"picture_count, front_cover, create_time, update_time, classifyType "
		"FROM project "
		"ORDER BY classifyType ASC, project_name ASC;";

	QSqlQuery query;

	if (!query.exec(sql)) {
		qDebug() << "Failed to query project for sorting:" << query.lastError();
		noticeWin.Notice(this, u8"排序失败！", 1500);
		return;
	}

	int itemCount = 0;
	while (query.next()) {
		QString id = query.value("id").toString();
		QString projectName = query.value("project_name").toString();
		QString lineName = query.value("line_name").toString();
		QString classifyType = query.value("classifyType").toString();
		QString keyword = query.value("keyword").toString();
		QString introduction = query.value("introduction").toString();
		int pictureCount = query.value("picture_count").toInt();
		QString frontCover = query.value("front_cover").toString();
		QString createTime = query.value("create_time").toString();
		QString updateTime = query.value("update_time").toString();

		// 创建列表项
		QListWidgetItem* item = new QListWidgetItem(ui.listWidget_project);
		//project_item* widget = new project_item(projectName, lineName, classifyType, this);
		project_item* widget = new project_item(frontCover, id, projectName, lineName, pictureCount, createTime, updateTime, classifyType,this);

		// 如果 project_item 支持更多初始化（如设置封面、数量等），可在此扩展
		// 例如：widget->setPictureCount(pictureCount);
		//       widget->setFrontCover(frontCover);

		item->setSizeHint(widget->sizeHint());
		item->setData(Qt::UserRole, id); // 保存项目 UUID

		ui.listWidget_project->setItemWidget(item, widget);
		++itemCount;
	}
	if (itemCount == 0)
	{
		noticeWin.Notice(this, u8"当前无任何项目可排序！", 1500);
		return;
	}

	// 提示用户
	noticeWin.Notice(this, QString(u8"已按类型排序，共 %1 个项目").arg(itemCount), 1000);
}

void PanoramicImageReader::slotUpdateProjList()
{
	ui.listWidget_project->clear();
	// 窗口显示后延迟初始化列表
	QString data = QString("select * from project order by create_time desc");
	QSqlQuery query(data);//查询表的内容
	m_allQuery = query;
	m_allQuery.last();
	m_ProjCount = m_allQuery.at() + 1;

	m_currentProjIndex = 0;
	QTimer::singleShot(0, this, &PanoramicImageReader::initListWidgetPure);
}

void PanoramicImageReader::slotDelProj(QString projUUID)
{
	if (m_msgBox)
	{
		delete m_msgBox;
	}
	m_msgBox = new MsgBox(3, u8"确认要删除吗?", this);
	if (m_msgBox->exec() == QDialog::Rejected) return;
	// 删除作品记录
	{
		QString sql = QString("delete from project where id = '%1'").arg(projUUID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	{
		QString sql = QString("delete from picture where project_name_id = '%1'").arg(projUUID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	{
		QString sql = QString("delete from hotpoint where project_id = '%1'").arg(projUUID);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	//删除文件夹
	{
		QString dirName = QApplication::applicationDirPath() + "/DataBase/" + projUUID;
		QDir dir(dirName);
		bool success = dir.removeRecursively();
	}
	noticeWin.Notice(this, u8"删除成功！", 1000);
	//刷新
	UpdateList();
}


void PanoramicImageReader::initWindows()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->availableGeometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height();

	ui.listWidget_project->setGeometry(0, 70, scrWidth, scrHeight - 70 - 50);

	ui.label_bg->resize(scrWidth, scrHeight);
}

void PanoramicImageReader::UpdateList()
{
	ui.listWidget_project->clear();
	QString data = QString("select * from project order by create_time desc");
	QSqlQuery query(data);//查询表的内容
	m_allQuery = query;
	m_allQuery.last();
	m_ProjCount = m_allQuery.at() + 1;

	m_currentProjIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载作品...", u8"取消", 0, m_ProjCount, this);
	m_progressDialog->setWindowTitle(u8"请稍等");
	m_progressDialog->setStyleSheet("QProgressDialog {"
		"background-color: white;"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"padding: 10px;"
		"}");

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	m_progressDialog->move(screenRect.width() / 2 - 50, screenRect.height() / 2 - 20);

	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setWindowFlags(Qt::FramelessWindowHint);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimumDuration(0);
	QTimer::singleShot(0, this, &PanoramicImageReader::initListWidget);
}

void PanoramicImageReader::create_linename_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `line_name` \
		( \
		`line_name` varchar(128)  NULL DEFAULT NULL, \
		`voltage_level` varchar(32) NULL DEFAULT NULL, \
		`create_time` varchar(64)  NULL DEFAULT NULL , \
		`more1` varchar(32) NULL DEFAULT NULL, \
		`more2` varchar(64) NULL DEFAULT NULL, \
		`more3` varchar(128) NULL DEFAULT NULL, \
		PRIMARY KEY(`line_name`));");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

//void PanoramicImageReader::create_project_sqlite()
//{
//	QString sql = QString(
//		"CREATE TABLE IF NOT EXISTS `project` \
//		( \
//		`id` varchar(64)  NOT NULL, \
//		`project_name` varchar(128)  NULL DEFAULT NULL, \
//		`line_name` varchar(128)  NULL DEFAULT NULL, \
//		`keyword` varchar(64) NULL DEFAULT NULL, \
//		`introduction` varchar(256) NULL DEFAULT NULL, \
//		`picture_count` int(8)  DEFAULT '0', \
//		`front_cover` varchar(128) NULL DEFAULT NULL, \
//		`create_time` varchar(64)  NULL DEFAULT NULL , \
//		`update_time` varchar(64)  NULL DEFAULT NULL , \
//		`more1` varchar(32) NULL DEFAULT NULL, \
//		`more2` varchar(64) NULL DEFAULT NULL, \
//		`more3` varchar(128) NULL DEFAULT NULL, \
//		PRIMARY KEY(`id`));");
//	QSqlQuery query;
//	if (!query.exec(sql))
//	{
//		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
//	}
//}

void PanoramicImageReader::create_project_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `project` \
        ( \
        `id` varchar(64)  NOT NULL, \
        `project_name` varchar(128)  NULL DEFAULT NULL, \
        `line_name` varchar(128)  NULL DEFAULT NULL, \
        `keyword` varchar(64) NULL DEFAULT NULL, \
        `introduction` varchar(256) NULL DEFAULT NULL, \
        `picture_count` int(8)  DEFAULT '0', \
        `front_cover` varchar(128) NULL DEFAULT NULL, \
        `create_time` varchar(64)  NULL DEFAULT NULL , \
        `update_time` varchar(64)  NULL DEFAULT NULL , \
        `classifyType` varchar(64) NULL DEFAULT NULL, \
        `more1` varchar(32) NULL DEFAULT NULL, \
        `more2` varchar(64) NULL DEFAULT NULL, \
        `more3` varchar(128) NULL DEFAULT NULL, \
        PRIMARY KEY(`id`));");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_picture_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `picture` \
		( \
		`id` varchar(64)  NOT NULL, \
		`picture_path` varchar(128)  NULL DEFAULT NULL, \
		`project_name_id` varchar(64) NULL DEFAULT NULL, \
		`project_name` varchar(128)  NULL DEFAULT NULL,  \
		`start_angle` varchar(16) NULL DEFAULT NULL, \
		`fov` varchar(16) NULL DEFAULT NULL, \
		`zoom` varchar(16) NULL DEFAULT NULL, \
		`zoom_range` varchar(32) NULL DEFAULT NULL, \
		`h_range` varchar(32) NULL DEFAULT NULL, \
		`v_range` varchar(32) NULL DEFAULT NULL, \
		`compass_s_angle` varchar(16) NULL DEFAULT NULL, \
		`compass_pic_path` varchar(128) NULL DEFAULT NULL, \
		`compass_location` varchar(16) NULL DEFAULT NULL, \
		`group_unsel_bg_color` varchar(16) NULL DEFAULT NULL, \
		`group_unsel_text_color` varchar(16) NULL DEFAULT NULL, \
		`group_sel_bg_color` varchar(16) NULL DEFAULT NULL, \
		`group_sel_text_color` varchar(16) NULL DEFAULT NULL, \
		`isLevelShow` varchar(16) NULL DEFAULT NULL, \
		`more1` varchar(32) NULL DEFAULT NULL, \
		`more2` varchar(64) NULL DEFAULT NULL, \
		PRIMARY KEY(`id`));");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_hotpoint_sqlite()
{
	//isFollowSysZoom 跟随系统缩放
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `hotpoint` \
		( \
		`id` varchar(64)  NOT NULL, \
		`hotpoint_name` varchar(128)  NULL DEFAULT NULL, \
		`picture_id` varchar(64)  NULL DEFAULT NULL, \
		`hotpoint_path` varchar(128)  NULL DEFAULT NULL, \
		`style` varchar(16) NULL DEFAULT NULL, \
		`position` varchar(16) NULL DEFAULT NULL, \
		`w_zoom` varchar(16) NULL DEFAULT NULL, \
		`h_zoom` varchar(16) NULL DEFAULT NULL, \
		`isLock` varchar(4) NULL DEFAULT NULL, \
		`isIconShow` varchar(4) NULL DEFAULT NULL, \
		`isTitleShow` varchar(4) NULL DEFAULT NULL, \
		`isFollowSysZoom` varchar(4) NULL DEFAULT NULL, \
		`project_id` varchar(64)  NULL DEFAULT NULL, \
		`title_bg_color` varchar(16) NULL DEFAULT NULL, \
		`title_text_color` varchar(16) NULL DEFAULT NULL, \
		`title_text_size` varchar(4) NULL DEFAULT NULL, \
		`more1` varchar(32) NULL DEFAULT NULL, \
		`more2` varchar(64) NULL DEFAULT NULL, \
		PRIMARY KEY(`id`));");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_ruler_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `ruler` \
		( \
		`id` varchar(64)  NOT NULL, \
		`ruler_name` varchar(128)  NULL DEFAULT NULL, \
		`picture_id` varchar(64)  NULL DEFAULT NULL, \
		`position` varchar(16) NULL DEFAULT NULL, \
		`text_font_style` varchar(16) NULL DEFAULT NULL, \
		`text_font_size` varchar(16) NULL DEFAULT NULL, \
		`text_color` varchar(16) NULL DEFAULT NULL, \
		`text_bg_color` varchar(16) NULL DEFAULT NULL, \
		`text_pos` varchar(16) NULL DEFAULT NULL, \
		`text_isFollowSysZoom` varchar(16) NULL DEFAULT NULL, \
		`text_isCustomAngle` varchar(8) NULL DEFAULT NULL, \
		`text_angle` varchar(8) NULL DEFAULT NULL, \
		`line_thick` varchar(8) NULL DEFAULT NULL, \
		`line_color` varchar(8) NULL DEFAULT NULL, \
		`line_endpoint_style` varchar(8) NULL DEFAULT NULL, \
		`line_endpoint_color` varchar(8) NULL DEFAULT NULL, \
		PRIMARY KEY(`id`));");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_level_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `level` \
		( \
		`pic_id` varchar(64)  NOT NULL, \
		`min_zoom` varchar(16) NULL DEFAULT NULL, \
		`max_zoom` varchar(16) NULL DEFAULT NULL, \
		`level_id` varchar(64) NULL DEFAULT NULL,\
		`more1` varchar(32)  NULL DEFAULT NULL, \
		`more2` varchar(64)  NULL DEFAULT NULL);");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_levelshow_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `levelshow` \
		( \
		`icon_id` varchar(64)  NOT NULL, \
		`level_id` varchar(64) NULL DEFAULT NULL,\
		`pic_id` varchar(64) NULL DEFAULT NULL,\
		`more1` varchar(32)  NULL DEFAULT NULL,\
		`more2` varchar(64)  NULL DEFAULT NULL);");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_group_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `group` \
		( \
		`pic_id` varchar(64)  NOT NULL, \
		`group_name` varchar(64)  NOT NULL, \
		`isGroupShowInPic` varchar(4) NULL DEFAULT NULL, \
		`isPerGroupSeeAngle` varchar(4) NULL DEFAULT NULL, \
		`group_rotate_x` varchar(8) NULL DEFAULT NULL, \
		`group_rotate_y` varchar(8) NULL DEFAULT NULL, \
		`gruop_zoom` varchar(8) NULL DEFAULT NULL, \
		`isToGroupZoom` varchar(4) NULL DEFAULT NULL, \
		`isdefOpen` varchar(4) NULL DEFAULT NULL, \
		`isIncludeAll` varchar(4) NULL DEFAULT NULL, \
		`GroupID` varchar(64) NULL DEFAULT NULL, \
		`more1` varchar(32)  NULL DEFAULT NULL, \
		`more2` varchar(64)  NULL DEFAULT NULL);");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_groupshow_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `groupshow` \
		( \
		`icon_id` varchar(64)  NOT NULL, \
		`GroupID` varchar(64) NULL DEFAULT NULL,\
		`pic_id` varchar(64) NULL DEFAULT NULL,\
		`more1` varchar(32)  NULL DEFAULT NULL, \
		`more2` varchar(64)  NULL DEFAULT NULL);");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}

void PanoramicImageReader::create_mask_sqlite()
{
	QString sql = QString(
		"CREATE TABLE IF NOT EXISTS `mask` \
		( \
		`pic_id` varchar(64)  NOT NULL, \
		`pic_path` varchar(64)  NOT NULL, \
		`style` varchar(8) NULL DEFAULT NULL,\
		`area` varchar(8) NULL DEFAULT NULL,\
		`more1` varchar(32)  NULL DEFAULT NULL, \
		`more2` varchar(64)  NULL DEFAULT NULL);");
	QSqlQuery query;
	if (!query.exec(sql))
	{
		qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
	}
}
