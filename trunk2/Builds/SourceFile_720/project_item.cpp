#include "project_item.h"
#include "CommonTool.h"
#include <QScreen>
#include <QSqlQuery>
#include <QSqlError>


project_item::project_item(QString front_cover, QString project_uuid, QString project_name, QString line_name, int picture_count, QString create_time, QString update_time, QString classifyType, QWidget *parent):
	m_front_cover(front_cover),
	m_project_uuid(project_uuid),
	m_project_name(project_name),
	m_line_name(line_name),
	m_picture_count(picture_count),
	m_create_time(create_time),
	m_update_time(update_time),
	m_classifyType(classifyType),
	QWidget(parent)
{
	ui.setupUi(this);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	setFixedSize(scrWidth - 80, 100);

	QPixmap pixmap(m_front_cover);
	QPixmap fitpixmap = pixmap.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.pushButton_frontCover->setIcon(QIcon(fitpixmap));
	ui.pushButton_frontCover->setIconSize(QSize(64, 64));
	ui.pushButton_frontCover->setFlat(true);
	ui.pushButton_frontCover->setStyleSheet("border: 0px");

	ui.label_projectName->setText(QString("[%1] %2").arg(m_classifyType).arg(m_project_name));
	//ui.label_projectName->setText(QString("%1").arg(m_project_name));
	ui.label_lineName->setText(m_line_name);
	ui.label_picCount->setText(QString::number(m_picture_count));
	ui.label_createTime->setText(m_create_time);
	ui.label_lastChange->setText(m_update_time);

	ui.pushButton_see->move(scrWidth - 150 - 150, 34);
	ui.pushButton_edit->move(scrWidth - 150 - 100, 34);
	ui.pushButton_del->move(scrWidth - 150 - 50, 34);

	//ÐÞ¸Ä×ÖÌåÑÕÉ«
	ui.pushButton_see->setStyleSheet("QPushButton{color: rgb(0, 85, 255);background-color: rgba(255, 255, 255,0);border:0px;font: 12pt \"Î¢ÈíÑÅºÚ\";}"
		"QPushButton::hover{color: rgb(0, 120, 255);}");
	ui.pushButton_edit->setStyleSheet("QPushButton{color: rgb(0, 85, 255);background-color: rgba(255, 255, 255,0);border:0px;font: 12pt \"Î¢ÈíÑÅºÚ\";}"
		"QPushButton::hover{color: rgb(0, 120, 255);}");
	ui.pushButton_del->setStyleSheet("QPushButton{color: rgb(255, 0, 0);background-color: rgba(255, 255, 255,0);border:0px;font: 12pt \"Î¢ÈíÑÅºÚ\";}"
		"QPushButton::hover{color: rgb(255, 50, 50);}");


	ui.pushButton_bg->setFixedSize(scrWidth - 30, 100);

	connect(ui.pushButton_bg, SIGNAL(clicked()), this, SLOT(slotSee()));
	connect(ui.pushButton_see, SIGNAL(clicked()), this, SLOT(slotSee()));
	connect(ui.pushButton_edit, SIGNAL(clicked()), this, SLOT(slotEdit()));
	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDel()));

}

project_item::~project_item()
{}

void project_item::slotSee()
{
	QStringList listPicPath;
	QString sql = QString("SELECT picture_path from picture where project_name_id = '%1'").arg(m_project_uuid);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		listPicPath.append(query.value(0).toString());
	}

	if (m_projSee)
	{
		delete m_projSee;
	}
	m_projSee = new project_see(m_project_uuid, listPicPath);
	m_projSee->setWindowModality(Qt::ApplicationModal);
	m_projSee->show();
}

void project_item::slotEdit()
{
	ProjectInfo::Ins().m_projUUID = m_project_uuid;

	QStringList listPicPath;
	QString sql = QString("SELECT picture_path from picture where project_name_id = '%1'").arg(m_project_uuid);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		listPicPath.append(query.value(0).toString());
	}

	if (m_projEdit)
	{
		delete m_projEdit;
	}
	m_projEdit = new project_edit(m_project_uuid, m_project_name, listPicPath);
	connect(m_projEdit, SIGNAL(sig_update()), this, SLOT(slotUpdate()));
	m_projEdit->setWindowModality(Qt::ApplicationModal);
	m_projEdit->show();
}

void project_item::slotDel()
{
	emit sigDel(m_project_uuid);
}

void project_item::slotUpdate()
{
	//·âÃæ
	QPixmap pixmap(m_front_cover);
	QPixmap fitpixmap = pixmap.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.pushButton_frontCover->setIcon(QIcon(fitpixmap));

	QString sql = QString("SELECT * from project where id = '%1'").arg(m_project_uuid);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		ui.label_picCount->setText(query.value(5).toString());
		ui.label_lastChange->setText(query.value(8).toString());
	}
}



