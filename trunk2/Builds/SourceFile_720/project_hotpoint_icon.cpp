#include "project_hotpoint_icon.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QMouseEvent>


project_hotpoint_icon::project_hotpoint_icon(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_selIcon = "";

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	m_rectMovableArea = QRect(0, 0, width(), 60);

	ui.listWidget->setViewMode(QListView::IconMode);
	ui.listWidget->setSpacing(19);
	ui.listWidget->setMovement(QListView::Static);
	ui.listWidget->setFocusPolicy(Qt::ClickFocus);
	ui.listWidget->setStyleSheet(
		"QListWidget{"
		"    background-color: rgba(255, 255, 255, 0);"
		"    border: 0px;"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item {"
		"    border: 0px solid #dee2e6;"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item:selected {"
		"    border: 3px solid #fa6400;"
		"}"
		"QListWidget::item:hover {"
		"    border: 3px solid #fa6400;"
		"}"
	);

	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotDone()));


	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
	ui.pushButton_ok->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	// 使用定时器开始处理
	m_currentItemIndex = 0;
	QString sysIcon = QApplication::applicationDirPath() + "/Resource/hotpoint/system";
	m_sysIconPathList = searchImages(sysIcon);
	QTimer::singleShot(0, this, &project_hotpoint_icon::processNexIconItem);
}

project_hotpoint_icon::~project_hotpoint_icon()
{}

void project_hotpoint_icon::slotClose()
{
	this->close();
	this->destroy(true);
}


void project_hotpoint_icon::processNexIconItem()
{
	if (m_currentItemIndex >= m_sysIconPathList.size())
	{
		return;
	}
	QString picPath = m_sysIconPathList[m_currentItemIndex];
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(80, 80));
	item->setData(Qt::UserRole, picPath);
	ui.listWidget->addItem(item);
	project_see_item* item2 = new project_see_item(picPath);
	item2->resize(QSize(80, 80));
	item2->setNameHide();
	connect(item2, SIGNAL(sig_picpath(QString)), this, SLOT(slotNewIconClick(QString)));

	ui.listWidget->setItemWidget(item, item2);
	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_hotpoint_icon::processNexIconItem);
}

void project_hotpoint_icon::slotDone()
{
	if (m_selIcon == "")
	{
		return;
	}
	emit sig_iconPath(m_selIcon);
	slotClose();
}

void project_hotpoint_icon::slotNewIconClick(QString iconPath)
{
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString tmpPic = listItem->data(Qt::UserRole).toString();
		project_see_item* customItem = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
		if (customItem && (tmpPic == iconPath))
		{
			m_selIcon = iconPath;
			customItem->setFocus();
			break;
		}
	}
}

void project_hotpoint_icon::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_mousePressFlag = true;
		this->setCursor(Qt::OpenHandCursor);
		m_mouseSrcPos = event->pos();
	}
}

void project_hotpoint_icon::mouseReleaseEvent(QMouseEvent* event)
{
	this->setCursor(Qt::ArrowCursor);
	m_mousePressFlag = false;
}

void project_hotpoint_icon::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mousePressFlag)
	{
		m_mouseDstPos = event->pos();
		if (PointInArea(m_rectMovableArea, m_mouseSrcPos))
		{
			this->move(this->pos() + m_mouseDstPos - m_mouseSrcPos);
		}
	}
	event->ignore();
}
