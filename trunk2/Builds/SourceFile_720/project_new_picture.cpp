#include "project_new_picture.h"
#include <QFileInfo>
#include <QIcon>
#include "styleSheet.h"
#include <QLineEdit>


project_new_picture::project_new_picture(QString picPath, QWidget *parent)
	: m_picPath(picPath)
	,QWidget(parent)
{
	ui.setupUi(this);

	QFileInfo fileinfo = QFileInfo(m_picPath);
	QString  file_name = m_picPath.section("/", -1).section(".", 0,-2);

	QPixmap currentImage = QPixmap(m_picPath);
	int x = (currentImage.width() - 512) / 2;
	int y = (currentImage.height() - 512) / 2;
	QPixmap pixmap = currentImage.copy(x, y, 512, 512);

	QPixmap fitpixmap = pixmap.scaled(84, 84, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.pushButton_pic->setIcon(QIcon(fitpixmap));
	ui.pushButton_pic->setIconSize(QSize(84, 84));
	ui.pushButton_pic->setFlat(true);
	ui.pushButton_pic->setStyleSheet("border: 0px");
	ui.lineEdit_name->setText(file_name);

	connect(ui.pushButton_edit, SIGNAL(clicked()), this, SLOT(slotEdit()));
	connect(ui.pushButton_resee, SIGNAL(clicked()), this, SLOT(slotReSee()));
	connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(slotDel()));

	ui.pushButton_edit->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/new/edit_0.png);}"
		"QPushButton::hover{border-image:url(%1/Resource/new/edit_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_resee->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/new/resee_0.png);}"
		"QPushButton::hover{border-image:url(%1/Resource/new/resee_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_del->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/new/del_0.png);}"
		"QPushButton::hover{border-image:url(%1/Resource/new/del_1.png);}").arg(QApplication::applicationDirPath()));
}

project_new_picture::~project_new_picture()
{
	if (m_reseeItem)
	{
		delete m_reseeItem;
	}
}

void project_new_picture::slotEdit()
{
	ui.lineEdit_name->setReadOnly(false);
	ui.lineEdit_name->setFocus();
}

void project_new_picture::slotEndEdit()
{
	ui.lineEdit_name->setReadOnly(true);
}


void project_new_picture::slotReSee()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	if (m_reseeItem)
	{
		delete m_reseeItem;
	}
	m_reseeItem = new project_new_pictrue_resee(m_picPath);
	m_reseeItem->setWindowModality(Qt::ApplicationModal);
	m_reseeItem->move((scrWidth - m_reseeItem->width()) / 2, (scrHeight - m_reseeItem->height()) / 2);
	m_reseeItem->showNormal();
}

void project_new_picture::slotDel()
{
	emit sigPic(m_picPath);
}

