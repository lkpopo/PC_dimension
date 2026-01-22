#include "project_edit_delPics.h"
#include <QSqlError>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QSqlQuery>
#include "PanoramaWidget.h"
#include "project_edit_delPics_item.h"

project_edit_delPics::project_edit_delPics(QString projUUID, QWidget *parent)
	: m_projUUID(projUUID),QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	QString data = QString("select id, picture_path from picture where project_name_id = '%1'").arg(m_projUUID);
	QSqlQuery query(data);
	while (query.next())
	{
		QString id = query.value(0).toString();
		QString picPath = query.value(1).toString();

		m_vecpicID.push_back(id);
		m_mapIDPicPath[id] = picPath;
	}
	m_currentItemIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载...", u8"取消", 0, m_vecpicID.size(), this);
	m_progressDialog->setWindowTitle(u8"请稍等");
	m_progressDialog->setStyleSheet("QProgressDialog {"
		"background-color: white;"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"padding: 10px;"
		"}");
	m_progressDialog->move(width() / 2 - 50, height() / 2 - 20);

	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setWindowFlags(Qt::FramelessWindowHint);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimumDuration(0);

	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
	connect(ui.pushButton_ok, SIGNAL(clicked()), this, SLOT(slotOK()));

	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));


	// 使用定时器开始处理
	QTimer::singleShot(30, this, &project_edit_delPics::processNextPicItem);
}

project_edit_delPics::~project_edit_delPics()
{}

void project_edit_delPics::slotClose()
{
	this->close();
	this->destroy(true);
}

void project_edit_delPics::SlotSelList(int sel)
{
	int tmp = 0;
	if (sel == 2)
		tmp = 1;
	for (size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString iconID = listItem->data(Qt::UserRole + 1).toString();
		project_edit_delPics_item* item = qobject_cast<project_edit_delPics_item*>(ui.listWidget->itemWidget(listItem));
		if (item)
		{
			item->setCheckStatus(tmp);
		}
	}
}

void project_edit_delPics::slotOK()
{
	std::vector<QString> vecIconID;
	for (size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString iconID = listItem->data(Qt::UserRole).toString();
		project_edit_delPics_item* item = qobject_cast<project_edit_delPics_item*>(ui.listWidget->itemWidget(listItem));
		if (item)
		{
			bool blCheck = item->getCheckStatus();
			if (blCheck == true)
			{
				vecIconID.push_back(iconID);
			}
		}
	}
	if (ui.label_title->text() == u8"删除场景")
	{
		if (vecIconID.size() == ui.listWidget->count())
		{
			emit sig_Msg(u8"无法删除，作品必须至少包含一个场景");
			return;
		}
		emit sig_delList(vecIconID);
	}
	else if (ui.label_title->text() == u8"选择要应用到的场景")
	{
		//emit sig_selList(vecIconID);
	}
	//
	slotClose();
}

void project_edit_delPics::processNextPicItem()
{
	if (m_currentItemIndex >= m_vecpicID.size() || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}

	QString picPath = m_mapIDPicPath[m_vecpicID[m_currentItemIndex]];
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(300, 40));
	item->setData(Qt::UserRole, m_vecpicID[m_currentItemIndex]);
	ui.listWidget->addItem(item);
	project_edit_delPics_item* item2 = new project_edit_delPics_item(m_projUUID, m_vecpicID[m_currentItemIndex], picPath);
	connect(item2, SIGNAL(sig_SelStatus(int)), this, SLOT(slotSelStatus(int)));
	item2->resize(QSize(300, 40));
	connect(item2, SIGNAL(sigPic(QString)), this, SLOT(slotDelPic(QString)));
	ui.listWidget->setItemWidget(item, item2);

	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit_delPics::processNextPicItem);
}

void project_edit_delPics::setTitle(QString title)
{
	ui.label_title->setText(title);
}

void project_edit_delPics::slotSelStatus(int nl)
{
	int selNum = 0;
	for (size_t i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString iconID = listItem->data(Qt::UserRole + 1).toString();
		project_edit_delPics_item* item = qobject_cast<project_edit_delPics_item*>(ui.listWidget->itemWidget(listItem));
		if (item)
		{
			bool blCheck = item->getCheckStatus();
			if (blCheck == true)
				selNum++;
		}
	}
	if (selNum == ui.listWidget->count())
	{
		disconnect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
		ui.checkBox->setChecked(true);
		connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
	}
	else
	{
		disconnect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
		ui.checkBox->setChecked(false);
		connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SLOT(SlotSelList(int)));
	}
}

