#include "project_edit_seeAngle.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>

project_edit_seeAngle::project_edit_seeAngle(QString projUUID, QWidget *parent)
	: m_projUUID(projUUID),QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	resize(250, scrHeight - 60);
	ui.label_bg->resize(250, scrHeight - 60);
	
	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));
	connect(ui.pushButton_CtoOtherPic, SIGNAL(clicked()), this, SLOT(slotSetOrgSeeAngle()));

	ui.pushButton_done->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
}

project_edit_seeAngle::~project_edit_seeAngle()
{}

void project_edit_seeAngle::slotSetPixmap(QPixmap& pix)
{
	// 关键：启用自动缩放
	ui.label_SeeAngle->setScaledContents(true);
	// 直接设置原始 pixmap，QLabel 会自动拉伸到自身大小
	ui.label_SeeAngle->setPixmap(pix);
}

void project_edit_seeAngle::slotDone()
{
	emit sig_save();
}

void project_edit_seeAngle::slotSetOrgSeeAngle()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景");
	connect(m_delPics, SIGNAL(sig_selList(std::vector<QString>, std::vector<QString>)), this, SLOT(slotSelPic(std::vector<QString>, std::vector<QString>)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_seeAngle::slotSelPic(std::vector<QString> vecPicID, std::vector<QString> vecPicPath)
{
	for (auto picID : vecPicID)
	{
		//数据库
		{
			QString strRotation = QString::number(m_rotation.x(), 10, 1) + ";" + QString::number(m_rotation.y(), 10, 1) + ";"+ QString::number(m_rotation.z(), 10, 1);
			QString strZoom = QString::number(m_zoom, 10, 4);
			QString updata = QString("update picture set start_angle = '%1', zoom ='%2'where id = '%7'").arg(strRotation).arg(strZoom).arg(picID);
			QSqlQuery query;
			query.exec(updata);
		}
		//场景
	/*	tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
		tmpPam.zoom = QString::number(m_zoom, 'f', 1);
		m_panWidget->m_mapPicAngle[m_picPath] = tmpPam;*/

	}
	




}

void project_edit_seeAngle::slotSetRoration_Zoom(QVector3D rotation, float zoom)
{
	m_rotation = rotation;
	m_zoom = zoom;
}



