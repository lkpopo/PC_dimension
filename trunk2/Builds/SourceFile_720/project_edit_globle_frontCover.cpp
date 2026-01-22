#include "project_edit_globle_frontCover.h"
#include <QFileDialog>
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>

project_edit_globle_frontCover::project_edit_globle_frontCover(QString projUUID, QStringList listPicPath,QWidget *parent)
	: m_projUUID(projUUID), m_listPicPath(listPicPath), QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	//m_panWidget = new PanoramaWidget(m_listPicPath[0], false, false, "LeftTop", this);
	m_panWidget = new PanoramaWidget(m_listPicPath[0], false, false, this);
	{
		connect(m_panWidget, SIGNAL(sig_transEnd()), this, SLOT(slotSetFirstAngle()));
		{
			std::map<QString, WindowParameter> mapPicAngle;
			std::map<QString, CompassParameter> mapPicCompass;
			QString data = QString("select * from picture where project_name_id = '%1'").arg(m_projUUID);
			QSqlQuery query(data);
			while (query.next())
			{
				QString id = query.value(0).toString();
				QString picPath = query.value(1).toString();
				WindowParameter pam;
				pam.start_angle = query.value(4).toString();
				pam.fov = query.value(5).toString();
				pam.zoom = query.value(6).toString();
				pam.zoom_range = query.value(7).toString();
				pam.h_range = query.value(8).toString();
				pam.v_range = query.value(9).toString();

				CompassParameter comp;
				comp.compassAngle = query.value(10).toFloat();
				comp.picPath = query.value(11).toString();
				comp.location = query.value(12).toString();

				mapPicAngle[picPath] = pam;
				mapPicCompass[picPath] = comp;

				m_mapUUidPicPath.insert(std::make_pair(id, picPath));
				m_mapPicPathUUid.insert(std::make_pair(picPath, id));
			}
			m_panWidget->SetAllPicAngle(mapPicAngle);
			m_panWidget->SetAllPicCompass(mapPicCompass);
			m_panWidget->initWindow(m_listPicPath[0]);
		}
	}
	ui.openGLWidget = m_panWidget;
	ui.openGLWidget->setGeometry(60, 110, 300, 300);
	ui.pushButton_screenshot->raise();

	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + projUUID;
	QFileInfo fileInfo(m_listPicPath[0]);
	QString suf = fileInfo.suffix();
	QString frontcover = dirName + "/FrontCover." + suf;
	QPixmap pixmap(frontcover); // 图片路径
	ui.label_frontcover->setPixmap(pixmap);
	//
	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.pushButton_selPic, SIGNAL(clicked()), this, SLOT(slotSelPic()));
	connect(ui.pushButton_screenshot, SIGNAL(clicked()), this, SLOT(slotScreenshort()));
	connect(ui.pushButton_localUp, SIGNAL(clicked()), this, SLOT(slotLocalUp()));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotOK()));

	ui.pushButton_screenshot->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_localUp->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_ok->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
}

project_edit_globle_frontCover::~project_edit_globle_frontCover()
{}

void project_edit_globle_frontCover::slotClose()
{

	this->close();
	this->destroy(true);
}

void project_edit_globle_frontCover::slotSelPic()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	if (m_selPic)
	{
		delete m_selPic;
	}
	QStringList listPicPath;
	m_selPic = new project_edit_globle_frontCover_selPic(m_listPicPath);
	m_selPic->setWindowModality(Qt::ApplicationModal);
	m_selPic->move((scrWidth - m_selPic->width()) / 2, (scrHeight - m_selPic->height()) / 2);
	connect(m_selPic, SIGNAL(sig_pic(QString)), this, SLOT(slotChangePic(QString)));
	m_selPic->show();
}

void project_edit_globle_frontCover::slotScreenshort()
{
	QImage img = ui.openGLWidget->grabFramebuffer(); // 获取当前帧缓冲区的内容
	QPixmap pixmap = QPixmap::fromImage(img);
	ui.label_frontcover->setPixmap(pixmap);
}

void project_edit_globle_frontCover::slotLocalUp()
{
	QString filePath = QFileDialog::getOpenFileName(this, tr(u8"选择图片"), tr(""), tr(u8"图片文件(*.jpg *.jpeg *.png);所有文件（*.*);"));
	if (filePath == "") return;
	QPixmap pixmap(filePath); 
	QPixmap fitpixmap = pixmap.scaled(300, 300, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_frontcover->setPixmap(fitpixmap);
}

void project_edit_globle_frontCover::slotOK()
{
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;

	QFileInfo fileInfo(m_listPicPath[0]);
	QString suf = fileInfo.suffix();
	QString frontcover = dirName + "/FrontCover_tmp." + suf;
	const QPixmap* pixmap = ui.label_frontcover->pixmap();
	bool success = pixmap->save(frontcover);
	if (success)
	{
		noticeWin.Notice(this, u8"保存成功！", 2000);
		emit sig_change();
	}	
	else
		noticeWin.Notice(this, u8"保存失败！", 2000);
}

void project_edit_globle_frontCover::slotChangePic(QString path)
{
	m_panWidget->changePic(path, m_mapPicPathUUid[path],false);
}

