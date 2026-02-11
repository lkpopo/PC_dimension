#include "project_edit_ruler.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>

project_edit_ruler::project_edit_ruler(QString projUUID, QString picPath, QString picID, QWidget *parent)
	:m_projUUID(projUUID), m_cutPicPath(picPath), m_picID(picID), QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_rulerSet = nullptr;

	//隐藏
	{
		ui.label->hide();
		ui.pushButton_filter->hide();
		ui.pushButton_style->hide();
	}

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	//动态长度
	{
		resize(250, scrHeight - 60);
		ui.label_bg->resize(250, scrHeight - 60);
		ui.listWidget->resize(241, scrHeight - 60 - 130 - 10);
	}
	ui.listWidget->setViewMode(QListView::ListMode);
	ui.listWidget->setSpacing(5);
	ui.listWidget->setFocusPolicy(Qt::NoFocus);
	ui.listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.listWidget->setStyleSheet(
		"QListWidget{"
		"background-color: rgb(26, 26, 26 );"
		"border: 0px;"
		"border-radius: 5px;"
		"}"
		"QListWidget::item {"
		"background-color: rgb(60, 60, 60);"
		"border-radius: 5px;"
		"}"
		"QListWidget::item:hover {"
		"background-color: rgb(100, 100, 100);"
		"border-radius: 5px;"
		"}"
	);
	connect(this, SIGNAL(sig_UpdateHotpointList(QVector<Hotspot>&)), this, SLOT(slotUpdateHotpointList(QVector<Hotspot>&)));

	connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(slotAddRuler()));

	ui.pushButton_filter->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/filter_0.png);border:0px;background-color: rgb(52, 52, 52);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/filter_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_style->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/style_0.png);border:0px;background-color: rgb(52, 52, 52);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/style_1.png);}").arg(QApplication::applicationDirPath()));

	ui.pushButton_ceng->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/ceng_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/ceng_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_group->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/group_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/group_1.png);}").arg(QApplication::applicationDirPath()));



	//加载标尺
	//addIcon2List(m_picID);
}

project_edit_ruler::~project_edit_ruler()
{}


void project_edit_ruler::processNextRulerItem()
{
}

void project_edit_ruler::clearListWidget()
{
}

void project_edit_ruler::search(QString)
{
}

void project_edit_ruler::slotAddRuler()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	if (m_rulerSet != nullptr)
	{
		delete m_rulerSet;
	}
	m_rulerSet = new project_edit_ruler_set(m_projUUID, m_picID, "", this);
	m_rulerSet->move(scrWidth - 250, 60);
	m_rulerSet->showNormal();
}

