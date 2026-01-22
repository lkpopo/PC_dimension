#include "project_edit_hotpoint_item.h"
#include "CommonTool.h"
#include <QSqlQuery>
#include <QSqlError>


project_edit_hotpoint_item::project_edit_hotpoint_item(Hotspot& hp, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);


	m_iconUUID = hp.iconID;
	m_iconPath = hp.iconPath;

	QPixmap pixmap(hp.iconPath);
	QPixmap fitpixmap = pixmap.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_icon->setPixmap(fitpixmap);
	ui.pushButton_title->setText(hp.name);

	if (hp.lock == true)
		slot_SetMenuItemText(m_iconUUID, u8"锁定", u8"解锁");			
	else
		slot_SetMenuItemText(m_iconUUID, u8"解锁", u8"锁定");
	
	if (hp.icon_visible == true)
		slot_SetMenuItemText(m_iconUUID, u8"显示", u8"隐藏");	
	else
		slot_SetMenuItemText(m_iconUUID, u8"隐藏", u8"显示");
	
	connect(ui.pushButton_title, SIGNAL(clicked()), this, SLOT(slotIconEdit()));
	connect(ui.pushButton_changPic, SIGNAL(clicked()), this, SLOT(slotChangePic()));
	connect(ui.pushButton_copy, SIGNAL(clicked()), this, SLOT(slotCopyHotPoint()));
	connect(ui.pushButton_changPic, SIGNAL(clicked()), this, SLOT(slotChangePic()));

	connect(ui.pushButton_more, SIGNAL(sig_Txt(QString)), this, SLOT(slotMore(QString)));

	ui.pushButton_copy->setToolTip(u8"复制");

	ui.pushButton_copy->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/copy_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/copy_1.png);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_more->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/hotpoint/item/more_0.png);border:0px;background-color: rgba(52, 52, 52,0);}"
		"QPushButton::hover{border-image:url(%1/Resource/hotpoint/item/more_1.png);}").arg(QApplication::applicationDirPath()));

	ui.label_lock->setStyleSheet(QString("QLabel{border-image:url(%1/Resource/hotpoint/item/lock.png);background-color: rgba(52, 52, 52,0);}").arg(QApplication::applicationDirPath()));

}

project_edit_hotpoint_item::~project_edit_hotpoint_item()
{}

void project_edit_hotpoint_item::slotIconEdit()
{
	emit sig_editIcon(m_iconUUID);
}

void project_edit_hotpoint_item::slotMore(QString opera)
{
	emit sig_operate(m_iconUUID, opera);
}

void project_edit_hotpoint_item::slot_SetMenuItemText(QString iconID, QString oldText, QString newText)
{
	if (m_iconUUID == iconID)
	{
		if (oldText == u8"锁定")
			ui.label_lock->show();
		else if (oldText == u8"解锁")
			ui.label_lock->hide();
		else if (oldText == u8"隐藏")
			ui.label_bg->show();
		else if (oldText == u8"显示")
			ui.label_bg->hide();
		//修改文字
		ui.pushButton_more->setMenuItemText(oldText, newText);
	}
}

void project_edit_hotpoint_item::updateItem(Hotspot& hp)
{
	m_iconPath = hp.iconPath;
	QPixmap pixmap(hp.iconPath);
	QPixmap fitpixmap = pixmap.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_icon->setPixmap(fitpixmap);
	ui.pushButton_title->setText(hp.name);
}

void project_edit_hotpoint_item::slotCopyHotPoint()
{
	//复制热点到数据库
	Hotspot hp;
	QString data = QString("select * from hotpoint where id = '%1' LIMIT 1").arg(m_iconUUID);
	QSqlQuery query(data);//查询表的内容
	while (query.next())
	{
		hp.iconID = getUUID();
		hp.name = query.value(1).toString();
		QString picID = query.value(2).toString();
		hp.iconPath = query.value(3).toString();
		hp.style = query.value(4).toString();
		//
		QString positonStr = query.value(5).toString();
		QStringList iconPosList = positonStr.split(",");
		double icon_x = 0.0;
		double icon_y = 0.0;
		double icon_z = -1.0;

		double icon_x2 = 0.0;
		double icon_y2 = 0.0;
		double icon_z2 = -1.0;
		if (iconPosList.size() == 3)
		{
			icon_x = iconPosList[0].toDouble();
			icon_y = iconPosList[1].toDouble();
			icon_z = iconPosList[2].toDouble();
		}
		QVector3D tmpPos = getNearPointOnUnitSphere(QVector3D(icon_x, icon_y, icon_z),0.1);
		icon_x2 = tmpPos.x();
		icon_y2 = tmpPos.y();
		icon_z2 = tmpPos.z();
		QString newPosition = QString::number(icon_x2, 10, 4) + "," + QString::number(icon_y2, 10, 4) + "," + QString::number(icon_z2, 10, 4);
		hp.position = QVector3D(icon_x2, icon_y2, icon_z2);
		//
		hp.scale_x = query.value(6).toDouble();
		hp.scale_y = query.value(7).toDouble();
		hp.lock = query.value(8).toBool();

		hp.icon_visible = query.value(9).toBool();
		hp.title_visible = query.value(10).toBool();

		hp.isFollowSysZoom = query.value(10).toBool();

		QString title_bg_color = query.value(13).toString();
		QString title_text_color = query.value(14).toString();
		QString title_text_size = query.value(15).toString();

		hp.title_bg_color = QColor(30, 30, 30, 255);
		hp.title_text_color = QColor(255, 255, 255, 255);
		hp.title_text_size = 10;

		QStringList bgColorList = query.value(13).toString().split(",");
		QStringList textColorList = query.value(14).toString().split(",");
		int size = query.value(15).toInt();
		if (bgColorList.size() == 3)
			hp.title_bg_color = QColor(bgColorList[0].toInt(), bgColorList[1].toInt(), bgColorList[2].toInt());
		if (textColorList.size() == 3)
			hp.title_text_color = QColor(textColorList[0].toInt(), textColorList[1].toInt(), textColorList[2].toInt());
		hp.title_text_size = size;
		//
		QString sql = QString("INSERT INTO hotpoint( \
				id, \
				hotpoint_name, \
				picture_id, \
				hotpoint_path, \
				style,\
                position,\
				w_zoom,\
				h_zoom,\
				isLock,\
				isIconShow,\
				isTitleShow,\
				isFollowSysZoom,\
				project_id,\
				title_bg_color,\
                title_text_color,\
                title_text_size\
			) VALUES (\
				'%1',\
				'%2', \
				'%3', \
				'%4', \
				'%5', \
				'%6', \
				'%7', \
				'%8', \
				'%9', \
				'%10',\
                '%11',\
				'%12',\
				'%13', \
                '%14', \
                '%15', \
                '%16' \
			); ")
			.arg(hp.iconID)
			.arg(hp.name)
			.arg(picID)
			.arg(hp.iconPath)
			.arg(hp.style)
			.arg(newPosition)
			.arg(hp.scale_x)
			.arg(hp.scale_y)
			.arg(hp.lock)
			.arg(hp.icon_visible)
			.arg(hp.title_visible)
			.arg(hp.isFollowSysZoom)
			.arg(ProjectInfo::Ins().m_projUUID)
			.arg(title_bg_color)
			.arg(title_text_color)
			.arg(title_text_size);
		//执行语句
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	//复制到场景
	emit sig_CopyIcon(hp);
	//复制到list
	emit sig_insertNew(m_iconUUID, hp);
}
