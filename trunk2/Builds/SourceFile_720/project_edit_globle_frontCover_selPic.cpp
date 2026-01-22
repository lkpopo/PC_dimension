#include "project_edit_globle_frontCover_selPic.h"
#include <QTimer>
project_edit_globle_frontCover_selPic::project_edit_globle_frontCover_selPic(QStringList listPicPath, QWidget *parent)
	: m_listPicPath(listPicPath),QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_picPath = "";

	ui.listWidget->setViewMode(QListView::IconMode);
	ui.listWidget->setSpacing(10);
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
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotOK()));

	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
	ui.pushButton_ok->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	// 使用定时器开始处理
	QTimer::singleShot(0, this, &project_edit_globle_frontCover_selPic::processNextPicItem);

}

project_edit_globle_frontCover_selPic::~project_edit_globle_frontCover_selPic()
{}

void project_edit_globle_frontCover_selPic::slotClose()
{
	this->close();
	this->destroy(true);
}

void project_edit_globle_frontCover_selPic::processNextPicItem()
{
	if (m_currentItemIndex >= m_listPicPath.size())
	{
		return;
	}
	QString tmpPic = m_listPicPath[m_currentItemIndex];
	//QString picPath = tmpPic.replace("/Pictures/","/Pictures_Thumb/");
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(80, 80));
	item->setData(Qt::UserRole, tmpPic);
	ui.listWidget->addItem(item);
	project_see_item* item2 = new project_see_item(m_listPicPath[m_currentItemIndex]);
	item2->resize(QSize(80, 80));
	connect(item2, SIGNAL(sig_picpath(QString)), this, SLOT(slotNewPic(QString)));
	ui.listWidget->setItemWidget(item, item2);

	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit_globle_frontCover_selPic::processNextPicItem);
}

void project_edit_globle_frontCover_selPic::slotOK()
{
	if (m_picPath == "")
	{
		slotClose();
		return;
	}
	emit sig_pic(m_picPath);
	slotClose();
}

void project_edit_globle_frontCover_selPic::slotNewPic(QString picPath)
{
	if (picPath == m_picPath)
	{
		return;
	}
	m_picPath = picPath;
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString tmpPic = listItem->data(Qt::UserRole).toString();
		if (tmpPic != picPath) continue;
		project_see_item* customItem = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
		if (customItem)
		{
			customItem->setFocus();
			break;
		}
	}
}

