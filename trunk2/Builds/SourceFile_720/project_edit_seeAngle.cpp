#include "project_edit_seeAngle.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include "PanoramaWidget.h"


project_edit_seeAngle::project_edit_seeAngle(QString projUUID, QString picID, QWidget *parent)
	: m_projUUID(projUUID), m_picID(picID), QWidget(parent)
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
	
	connect(ui.pushButton_CtoOtherPic, SIGNAL(clicked()), this, SLOT(slotSetOrgSeeAngle()));
	connect(ui.pushButton_FovOtherPic, SIGNAL(clicked()), this, SLOT(slotSetFOVAngle()));
	connect(ui.pushButton_HVOtherPic, SIGNAL(clicked()), this, SLOT(slotSetHVAngle()));
	connect(ui.pushButton_AlltoOtherPic, SIGNAL(clicked()), this, SLOT(slotSetAllAngle()));

	//初始化
	WindowParameter pam;
	{
		QString data = QString("select * from picture where id = '%1' LIMIT 1").arg(picID);
		QSqlQuery query(data);
		while (query.next())
		{
			pam.start_angle = query.value(4).toString();
			pam.fov = query.value(5).toString();
			pam.zoom = query.value(6).toString();
			pam.zoom_range = query.value(7).toString();
			pam.h_range = query.value(8).toString();
			pam.v_range = query.value(9).toString();
		}
	}

	QStringList zoom_range = pam.zoom_range.split(";");
	QStringList h_range = pam.h_range.split(";");
	QStringList v_range = pam.v_range.split(";");

	ui.horizontalSlide_FOV->setStyleSheet("QSlider::handle { background: transparent; }"
		"QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");
	ui.horizontalSlide_FOV->setRange(0.0, 180.0);
	ui.horizontalSlide_FOV->setHandleValue(Handle_Bottom_Left_FOV, 60.0 / zoom_range[1].toFloat());
	ui.horizontalSlide_FOV->setHandleValue(Handle_Bottom_Right_FOV, 60.0 / zoom_range[0].toFloat());
	ui.horizontalSlide_FOV->setTopValue(pam.fov.toFloat());
	ui.horizontalSlide_FOV->setTickInterval(0.1);
	ui.label_fov_min->setText(QString::number(60.0 / zoom_range[1].toFloat(),10,1));
	ui.label_fov_max->setText(QString::number(60.0 / zoom_range[0].toFloat(), 10, 1));
	ui.label_fov_min->setText(QString::number(pam.fov.toFloat(), 10, 1));
	connect(ui.horizontalSlide_FOV, SIGNAL(handleValueChangedFOV(HandlePos_FOV, double)), this, SLOT(slotSlideValueChanged_FOV(HandlePos_FOV, double)));
	
	ui.horizontalSlide_H->setStyleSheet("QSlider::handle { background: transparent; }"
		"QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");
	ui.horizontalSlide_H->setRange(-360, 360);
	ui.horizontalSlide_H->setTickInterval(1);
	ui.horizontalSlide_H->setHandleValue(Handle_Bottom_Left_H, h_range[0].toFloat());
	ui.horizontalSlide_H->setHandleValue(Handle_Bottom_Right_H, h_range[1].toFloat());
	ui.label_H_min->setText(QString::number(h_range[0].toFloat(), 10, 0));
	ui.label_H_max->setText(QString::number(h_range[1].toFloat(), 10, 0));
	connect(ui.horizontalSlide_H, SIGNAL(handleValueChangedH(HandlePos_H, int)), this, SLOT(slotSlideValueChanged_H(HandlePos_H, int)));
	
	ui.horizontalSlide_V->setStyleSheet("QSlider::handle { background: transparent; }"
		"QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");
	ui.horizontalSlide_V->setRange(-180, 180);
	ui.horizontalSlide_V->setTickInterval(1);
	ui.horizontalSlide_V->setHandleValue(Handle_Bottom_Left_V, -v_range[1].toFloat());
	ui.horizontalSlide_V->setHandleValue(Handle_Bottom_Right_V, -v_range[0].toFloat());
	ui.label_V_min->setText(QString::number(-v_range[1].toFloat(), 10, 0));
	ui.label_V_max->setText(QString::number(-v_range[0].toFloat(), 10, 0));
	connect(ui.horizontalSlide_V, SIGNAL(handleValueChangedV(HandlePos_V, int)), this, SLOT(slotSlideValueChanged_V(HandlePos_V, int)));

	ui.pushButton_CtoOtherPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_FovOtherPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_HVOtherPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_AlltoOtherPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
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

void project_edit_seeAngle::slotSlideValueChanged_FOV(HandlePos_FOV pos, double value)
{
	float zoom = 60.0 / value;

	if (pos == Handle_Top_FOV)
		ui.label_fov_cut->setText(QString::number(value, 10, 1));
	else if (pos == Handle_Bottom_Left_FOV)
		ui.label_fov_min->setText(QString::number(value, 10, 1));
	else
		ui.label_fov_max->setText(QString::number(value, 10, 1));

	emit sig_cut_zoom_angle(zoom);
}

void project_edit_seeAngle::slotSlideValueChanged_H(HandlePos_H pos, int value)
{
	if (pos == Handle_Bottom_Left_H)
		ui.label_H_min->setText(QString::number(value));
	else 
		ui.label_H_max->setText(QString::number(value));

	emit sig_H_angle(value);
}

void project_edit_seeAngle::slotSlideValueChanged_V(HandlePos_V pos, int value)
{
	if (pos == Handle_Bottom_Left_V)
		ui.label_V_min->setText(QString::number(value));
	else
		ui.label_V_max->setText(QString::number(value));

	emit sig_V_angle(-value);
}

void project_edit_seeAngle::slotSetOrgSeeAngle()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景(初始视角)");
	connect(m_delPics, SIGNAL(sig_sel_setFirstAnglePic(std::vector<QString>&)), this, SIGNAL(sig_setFirstAnglePic(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_seeAngle::slotSetFOVAngle()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景(FOV视角范围)");
	connect(m_delPics, SIGNAL(sig_sel_setFOVPic(std::vector<QString>&)), this, SLOT(slotSetPicFOV(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_seeAngle::slotSetHVAngle()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景(水平垂直视角范围)");
	connect(m_delPics, SIGNAL(sig_sel_setHVPic(std::vector<QString>&)), this, SLOT(slotSetPicHV(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_seeAngle::slotSetAllAngle()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景(全部视角范围)");
	connect(m_delPics, SIGNAL(sig_sel_setFirstAnglePic(std::vector<QString>&)), this, SIGNAL(sig_setFirstAnglePic(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_sel_setAllPic(std::vector<QString>&)), this, SLOT(slotSetPicAll(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_seeAngle::slotSetPicFOV(std::vector<QString>& vecPicID)
{
	float zoom_org = 60.0 / ui.label_fov_cut->text().toFloat();
	float zoom_max = 60.0 / ui.label_fov_min->text().toFloat();
	float zoom_min = 60.0 / ui.label_fov_max->text().toFloat();

	emit sig_setPicFOV(vecPicID, zoom_org, zoom_min, zoom_max);
}

void project_edit_seeAngle::slotSetPicHV(std::vector<QString>& vecPicID)
{
	int H_min = ui.label_H_min->text().toInt();
	int H_max = ui.label_H_max->text().toInt();

	int V_min = ui.label_V_min->text().toInt();
	int V_max = ui.label_V_max->text().toInt();

	emit sig_setPicHV(vecPicID, H_min, H_max, V_min, V_max);
}

void project_edit_seeAngle::slotSetPicAll(std::vector<QString>& vecPicID)
{
	//org
	emit sig_setFirstAnglePic(vecPicID);
	//FOV
	{
		float zoom_org = 60.0 / ui.label_fov_cut->text().toFloat();
		float zoom_max = 60.0 / ui.label_fov_min->text().toFloat();
		float zoom_min = 60.0 / ui.label_fov_max->text().toFloat();

		emit sig_setPicFOV(vecPicID, zoom_org, zoom_min, zoom_max);
	}
	//HV
	{
		int H_min = ui.label_H_min->text().toInt();
		int H_max = ui.label_H_max->text().toInt();

		int V_min = ui.label_V_min->text().toInt();
		int V_max = ui.label_V_max->text().toInt();

		emit sig_setPicHV(vecPicID, H_min, H_max, V_min, V_max);
	}
}

