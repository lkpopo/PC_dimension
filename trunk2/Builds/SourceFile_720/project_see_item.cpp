#include "project_see_item.h"
#include <QFileDialog>

project_see_item::project_see_item(QString picPath, bool delHide, QWidget *parent)
	:m_picPath(picPath), m_delHide(delHide)
	,QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);
	
	if (m_delHide == true)
	{	
		ui.pushButton_del->hide();
		setFrontCover();
	}
	else
	{
		ui.pushButton_del->show();
		setNormalPic();
	}
	
	connect(ui.pushButton_baseName, SIGNAL(clicked()), this, SLOT(slotPicPath()));
	connect(ui.pushButton_pic, SIGNAL(clicked()), this, SLOT(slotPicPath()));
	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDelPic()));

	ui.pushButton_del->setStyleSheet(QString("QPushButton{background-color: rgba(255, 255, 255,0);border-image:url(%1/Resource/new/del_1.png);}"
		"QPushButton::hover{background-color: rgba(255, 255, 255,0);border-image:url(%1/Resource/new/del_0.png);}").arg(QApplication::applicationDirPath()));
}

project_see_item::~project_see_item()
{}

void project_see_item::setFrontCover()
{
	QFileInfo fileinfo = QFileInfo(m_picPath);
	QString  file_name = m_picPath.section("/", -1).section(".", 0, -2);
	QString pic = m_picPath;
	QString picPath_thumb = pic.replace("/Pictures/", "/Pictures_Thumb/");
	QPixmap pixmap(picPath_thumb);
	QPixmap fitpixmap = pixmap.scaled(74, 74, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	ui.pushButton_pic->setIcon(QIcon(fitpixmap));
	ui.pushButton_pic->setIconSize(QSize(74, 74));
	ui.pushButton_pic->setFlat(true);
	ui.label_baseName->setText(file_name);
}

void project_see_item::setFrontCover_thumb()
{
	QString tmp = m_picPath;
	QString picPath_thumb = tmp.replace("/Pictures/", "/Pictures_Thumb_tmp/");
	QPixmap pixmap(picPath_thumb);
	QPixmap fitpixmap = pixmap.scaled(74, 74, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.pushButton_pic->setIcon(QIcon(fitpixmap));
}

void project_see_item::setNormalPic()
{
	QFileInfo fileinfo = QFileInfo(m_picPath);
	QString  file_name = m_picPath.section("/", -1).section(".", 0, -2);
	QPixmap pixmap(m_picPath);
	QPixmap fitpixmap = pixmap.scaled(74, 74, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	ui.pushButton_pic->setIcon(QIcon(fitpixmap));
	ui.pushButton_pic->setIconSize(QSize(74, 74));
	ui.pushButton_pic->setFlat(true);
	ui.label_baseName->setText(file_name);
}

void project_see_item::slotDelPic()
{
	emit sig_delPic(m_picPath);
}

void project_see_item::slotPicPath()
{
	emit sig_picpath(m_picPath);
}



