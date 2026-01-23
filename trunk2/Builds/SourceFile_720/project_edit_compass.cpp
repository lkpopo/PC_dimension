#include "project_edit_compass.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include "panoramawidget.h"  
#include <QSqlQuery>
#include <QFileDialog>
#include <QPixmap>
#include <QDir>
#include <QSqlError>
#include <QMessageBox>


project_edit_compass::project_edit_compass(QString projUUID, QString picID, QWidget *parent)
	:m_projUUID(projUUID), m_picID(picID), QWidget(parent)
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

	selectedFile = QApplication::applicationDirPath() + "/Resource/compass/compass1.png";

	connect(ui.pushButton_selPic, SIGNAL(clicked()), this, SLOT(slotSetCompassStyle())); //选择指北针样式
	connect(ui.compass_position, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetPosition()));   //设置指北针位置
	connect(ui.pushButton_CtoOtherPic, SIGNAL(clicked()), this, SLOT(slotSetCampass()));
	
	ui.pushButton_selPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_CtoOtherPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
}

project_edit_compass::~project_edit_compass()
{}

void project_edit_compass::setCompassPic(QString compPic)
{
	if(compPic == "")
		compPic = QApplication::applicationDirPath() + "/Resource/compass/compass1.png";
	QPixmap pixmap(compPic);
	QPixmap fitpixmap = pixmap.scaled(115, 115, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_compassN_pic->setPixmap(fitpixmap);
}

void project_edit_compass::slotSetPixmap(QPixmap& pix)
{
	QPixmap fitpixmap = pix.scaled(210, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_compassN->setPixmap(fitpixmap);
}

void project_edit_compass::slotSetAngle(float angle)
{
	m_CAngle = angle;
}

void project_edit_compass::slotSetPosition()
{
	QString text = ui.compass_position->currentText();
	if (text == u8"左上角") 
		compassPosition = "LeftTop";
	else if (text == u8"底部居中") 
		compassPosition = "BottomCenter";
	else if (text == u8"右上角") 
		compassPosition = "RightTop";		
	
	emit sig_setCompassPos(compassPosition);
}

void project_edit_compass::slotSetCompassStyle()
{
	// 设置默认打开的文件夹路径（可选）
	QString defaultDir = QApplication::applicationDirPath() + "/Resource/compass";
	QDir dir(defaultDir);
	if (!dir.exists()) {
		defaultDir = QDir::homePath(); // 如果资源目录不存在，回退到用户主目录
	}

	// 弹出文件选择对话框，只允许选择图片
	selectedFile = QFileDialog::getOpenFileName(
		this,
		u8"选择指北针样式",          // 对话框标题
		defaultDir,                  // 默认打开路径
		u8"图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*)"  // 文件过滤器
	);

	if (!selectedFile.isEmpty()) 
	{
		// 验证是否为有效图片
		if (QPixmap(selectedFile).isNull()) {
			noticeWin.Notice(this, u8"所选文件不是有效的图片格式！", 2000);
			return;
		}
		setCompassPic(selectedFile);  // 更新指北针样式窗口的预览
		//
		emit sig_setCompassPic(selectedFile);	
	}
}

void project_edit_compass::slotSetCampass()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"选择要应用到的场景(指北针)");
	connect(m_delPics, SIGNAL(sig_sel_setCompassPic(std::vector<QString>&)), this, SLOT(slotSetPicCompass(std::vector<QString>&)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit_compass::slotSetPicCompass(std::vector<QString>& vecPicID)
{
	emit sig_setPicCompass(vecPicID, m_CAngle, selectedFile, compassPosition);
}
