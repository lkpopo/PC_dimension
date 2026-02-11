#include "project_edit_resee.h"
#include <QSqlQuery>
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include "project_see_item.h"

int item_size2 = 80;

project_edit_resee::project_edit_resee(QString projUUID, QStringList listPicPath, std::map<QString, QString> mapPicPathUUid, QWidget* parent) :
	m_projUUID(projUUID), m_listPicPath(listPicPath), m_mapPicPathUUid(mapPicPathUUid),QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(
		Qt::Window                // 基础窗口标识
		| Qt::WindowCloseButtonHint // 只显示关闭按钮
		| Qt::MSWindowsFixedSizeDialogHint // 可选：固定窗口大小，防止用户拉伸
	);

	ui.pushButton_info->hide();
	ui.pushButton_selPic->hide();

	ui.pushButton_draw->hide();

	//施工图
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + projUUID;
	QString Engineering_Drawings = dirName + "/Engineering_Drawings/Engineering_Drawings.jpg";
	QFileInfo fileInfo(Engineering_Drawings);
	if (fileInfo.exists())
	{
		ui.pushButton_draw->show();		
		QString styleSheet = QString("QPushButton{"
			"border-image: url(%1);"  // 背景图片路径
			"border: none;"  // 去除按钮边框
			"}").arg(Engineering_Drawings);
		ui.pushButton_draw->setStyleSheet(styleSheet);
	}
	
	setWindowTitle(u8"作品预览");
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	scrWidth -= 100;
	scrHeight -= 100;
	resize(scrWidth, scrHeight);
	move((screenRect.width() - width()) / 2, (screenRect.height() - getBottomPix() - height()) / 2);	

	ui.listWidget_group->move(70, 300);
	
	m_panWidget = new PanoramaWidget(m_listPicPath[0], true, true, this);
	connect(m_panWidget, SIGNAL(sigShowElement(bool)), this, SLOT(SlotShowElement(bool)));
	connect(m_panWidget, SIGNAL(sig_style_picText(QString)), this, SLOT(slotStyle_picText(QString)));
	connect(m_panWidget, SIGNAL(sig_style_picChange(QString)), this, SLOT(slotStyle_picChange(QString)));

	ui.checkBox_autoRotate->setChecked(true);
	m_panWidget->setHotspotShow(true);
	m_panWidget->setGroupStatus(u8"查看");
	ui.openGLWidget = m_panWidget;
	ui.openGLWidget->setGeometry(0, 0, scrWidth, scrHeight);
	ui.pushButton_close->setGeometry(scrWidth - 60, 20, 32, 32);
	ui.pushButton_full->setGeometry(scrWidth - 60, 60, 32, 32);
	ui.checkBox_autoRotate->setGeometry(scrWidth - 60 - 100, 80, 100, 20);
	ui.pushButton_selPic->move(20, scrHeight - 20 - 64);
	ui.pushButton_info->move(scrWidth - 20 - 64, scrHeight - 20 - 64);
	ui.label_pic_bg->setGeometry(0, scrHeight - 20 - 64 - 30 - 120, scrWidth, 100);

	ui.pushButton_draw->move(10, 120);

	connect(ui.checkBox_autoRotate, SIGNAL(stateChanged(int)), this, SLOT(Slot_autoRotate(int)));
	connect(ui.pushButton_draw, SIGNAL(clicked()), this, SLOT(SlotDraw()));

	int start = (scrWidth - (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2)) / 2;
	int lenth = (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2);
	if (lenth > scrWidth)
	{
		lenth = scrWidth;
		start = 0;
	}
	ui.listWidget->setGeometry(start, scrHeight - 20 - 64 - 30 - 120, lenth, 120);
	
	// 2. 核心设置：开启横向排列 + 关闭自动换行
	ui.listWidget->setFlow(QListView::LeftToRight); // 布局方向：从左到右（横向）
	ui.listWidget->setWrapping(false); // 关闭自动换行（关键，避免超出宽度后换行）

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
	ui.listWidget_group->setViewMode(QListView::ListMode);
	ui.listWidget_group->setSpacing(5);
	ui.listWidget_group->setFocusPolicy(Qt::NoFocus);
	ui.listWidget_group->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	ui.listWidget_group->setStyleSheet(
		"QListWidget{"
		"    background-color: rgba(255, 255, 255, 0);"
		"    border: 0px;"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item {"
		"    background-color: rgba(255, 255, 255, 0);"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item:hover {"
		"    border-radius: 5px;"
		"    background-color: rgba(255, 255, 255, 0);"
		"}"
	);

	ui.pushButton_close->hide();
	ui.pushButton_full->hide();
	ui.pushButton_selPic->raise();
	ui.pushButton_info->raise();
	ui.label_pic_bg->raise();
	ui.listWidget->raise();
	ui.listWidget_group->raise();
	ui.checkBox_autoRotate->raise();

	ui.pushButton_draw->raise();

	ui.pushButton_close->setStyleSheet(QString("QPushButton{image:url(%1/Resource/common/close_255_24.png);border-radius:16px;background-color: rgba(40, 40, 40, 80);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_full->setStyleSheet(QString("QPushButton{image:url(%1/Resource/common/max_1.png);border-radius:16px;background-color: rgba(40, 40, 40, 80);}").arg(QApplication::applicationDirPath()));

	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.pushButton_full, SIGNAL(clicked()), this, SLOT(slotFullScreen()));

	m_picPath = m_listPicPath[0];
	//加载
	//设置第一视角
	{
		m_rotation = QVector3D(0.0, 0.0, 0.0);
		QString data = QString("select start_angle, zoom from picture where id = '%1' LIMIT 1").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);
		while (query.next())
		{
			QString start_angle = query.value(0).toString();
			QStringList angleList = start_angle.split(';');
			if (angleList.size() == 3)
				m_rotation = QVector3D(angleList[0].toFloat(), angleList[1].toFloat(), angleList[2].toFloat());
			m_zoom = query.value(1).toFloat();
		}
		WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_picPath];
		tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
		tmpPam.zoom = QString::number(m_zoom, 'f', 1);
		m_panWidget->m_mapPicAngle[m_picPath] = tmpPam;
	}
	//加载场景
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

			CompassParameter comp;  //指北针
			comp.compassAngle = query.value(10).toFloat();
			comp.picPath = query.value(11).toString();
			comp.location = query.value(12).toString();

			mapPicAngle[picPath] = pam;
			mapPicCompass[picPath] = comp;
		}
		m_panWidget->SetAllPicAngle(mapPicAngle);
		m_panWidget->SetAllPicCompass(mapPicCompass);
		m_panWidget->initWindow(m_picPath);

		m_currentPicItemIndex = 0;
	}
	//加载热点
	{
		m_vecCurPicHp.clear();
		QString data = QString("select * from hotpoint where picture_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			Hotspot hp;
			hp.iconID = query.value(0).toString();
			hp.position = QVector3D(0, 0, -1);
			QString strPos = query.value(5).toString();
			QStringList posList = strPos.split(',');
			if (posList.size() == 3)
				hp.position = QVector3D(posList[0].toFloat(), posList[1].toFloat(), posList[2].toFloat());
			hp.iconPath = query.value(3).toString();
			hp.name = query.value(1).toString();
			hp.description = "";
			hp.style = query.value(4).toString();
			hp.scale_x = query.value(6).toFloat();
			hp.scale_y = query.value(7).toFloat();
			hp.lock = query.value(8).toBool();
			hp.icon_visible = query.value(9).toBool();
			hp.title_visible = query.value(10).toBool();
			hp.isFollowSysZoom = query.value(11).toBool();
			hp.title_bg_color = QColor(30, 30, 30, 255);
			hp.title_text_color = QColor(255, 255, 255, 255);
			QStringList bgColorList = query.value(13).toString().split(",");
			QStringList textColorList = query.value(14).toString().split(",");
			int size = query.value(15).toInt();
			if (bgColorList.size() == 3)
				hp.title_bg_color = QColor(bgColorList[0].toInt(), bgColorList[1].toInt(), bgColorList[2].toInt());

			if (textColorList.size() == 3)
				hp.title_text_color = QColor(textColorList[0].toInt(), textColorList[1].toInt(), textColorList[2].toInt());
			hp.title_text_size = size;
			//
			m_vecCurPicHp.push_back(hp);
			m_currentHpItemIndex = 0;
		}
	}
	//载入层次
	{
		std::map<QString, std::vector<HotspotLevelShow>> mapVecIconID_Zoom;
		QString data = QString("select d.icon_id, u.*from level u LEFT JOIN levelshow d ON u.level_id = d.level_id where u.pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			HotspotLevelShow level;
			level.iconID = query.value(0).toString();
			level.picID = query.value(1).toString();
			level.min_zoom = query.value(2).toFloat();
			level.max_zoom = query.value(3).toFloat();
			level.LevelID = query.value(4).toString();

			mapVecIconID_Zoom[level.iconID].push_back(level);
		}
		m_panWidget->setIconShowZooms(mapVecIconID_Zoom);
		//是否按层次显示
		{
			QString data = QString("select isLevelShow from picture where id = '%1' LIMIT 1").arg(m_mapPicPathUUid[m_picPath]);
			QSqlQuery query(data);//查询表的内容
			while (query.next())
			{
				QString islevelShow = query.value(0).toString();
				if (islevelShow == '2')
					m_panWidget->setLevelShow(true);
				else
					m_panWidget->setLevelShow(false);
			}
		}
	}
	//载入分组
	{
		ui.listWidget_group->hide();
		m_DefGroup = "";
		m_curInsertGroupNum = 0;
		m_mapVecGroup.clear();
		m_vecGroupID.clear();
		//加入场景
		std::map<QString, std::vector<HotspotGroupShow>> mapVecIconID_Group;
		QString data = QString("select d.icon_id, u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			HotspotGroupShow group;
			group.iconID = query.value(0).toString();
			group.picID = query.value(1).toString();
			group.groupName = query.value(2).toString();
			group.isGroupShowInPic = query.value(3).toBool();
			group.isPerGroupSeeAngle = query.value(4).toBool();

			group.group_rotate_x = query.value(5).toFloat();
			group.group_rotate_y = query.value(6).toFloat();
			group.gruop_zoom = query.value(7).toFloat();

			group.isToGroupZoom = query.value(8).toBool();
			group.isdefOpen = query.value(9).toBool();
			group.isIncludeAll = query.value(10).toBool();
			group.GroupID = query.value(11).toString();

			//场景中不显示
			if (group.isGroupShowInPic == 0) continue;
			//默认分组
			if (group.isdefOpen == true)
			{
				m_DefGroup = group.GroupID;
				m_panWidget->set_SelGroupID(m_cutGroupID);
			}
					
			mapVecIconID_Group[group.iconID].push_back(group);
			m_mapVecGroup[group.GroupID].push_back(group);
			//
			{
				bool isExist = false;
				for (const QString& str : m_vecGroupID) { // 遍历容器中所有元素
					if (str == group.GroupID) {        // 逐元素对比
						isExist = true;
						break;                   // 找到后立即退出循环，优化性能
					}
				}
				if (isExist == false)
				{
					m_vecGroupID.push_back(group.GroupID);
				}
			}
			//存在分组就显示
			ui.listWidget_group->show();
		}
		m_panWidget->setIconShowGroups(mapVecIconID_Group);
	}
	//加载遮罩
	{
		m_panWidget->setRegion_valid(true);
		QString style;
		QString pamLong = "0;360;0;180";
		{
			QString data = QString("select * from mask where pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
			QSqlQuery query(data);
			while (query.next())
			{
				style = query.value(2).toString();
				pamLong = query.value(3).toString();
			}
		}
		QStringList range = pamLong.split(";");
		m_panWidget->setRegion_theta_phi(range[0].toDouble(), range[1].toDouble(), range[2].toDouble(), range[3].toDouble());
	}
	// 场景
	QTimer::singleShot(0, this, &project_edit_resee::processNextPicItem);
}

project_edit_resee::~project_edit_resee()
{}

void project_edit_resee::slotClose()
{
	this->close();
	this->destroy(true);
}

void project_edit_resee::slotFullScreen()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	if (m_isFull == true)
	{
		ui.openGLWidget->setGeometry(0, 0, screenRect.width(), screenRect.height() - getBottomPix());

		ui.pushButton_selPic->move(20, scrHeight - 20 - 64);
		ui.pushButton_info->move(scrWidth - 20 - 64, scrHeight - 20 - 64);

		ui.label_pic_bg->setGeometry(0, scrHeight - 20 - 64 - 30 - 80, scrWidth, 100);
		ui.listWidget->setGeometry((scrWidth - (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2)) / 2, scrHeight - 20 - 64 - 30 - 80 + 5, (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2), 100);

		m_isFull = false;

		ui.pushButton_full->setStyleSheet(QString("QPushButton{image:url(%1/Resource/common/max_1.png);border-radius:16px;background-color: rgba(40, 40, 40, 80);}").arg(QApplication::applicationDirPath()));

		showNormal();
	}
	else
	{
		ui.openGLWidget->setGeometry(0, 0, screenRect.width(), screenRect.height());

		ui.pushButton_selPic->move(20, scrHeight - 20 - 64 + getBottomPix());
		ui.pushButton_info->move(scrWidth - 20 - 64, scrHeight - 20 - 64 + getBottomPix());

		ui.label_pic_bg->setGeometry(0, scrHeight - 20 - 64 - 30 - 80 + getBottomPix(), scrWidth, 100);
		ui.listWidget->setGeometry((scrWidth - (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2)) / 2, scrHeight - 20 - 64 - 30 - 80 + 5 + getBottomPix(), (item_size2 * m_listPicPath.size() + 10 * (m_listPicPath.size() - 1) + 20 * 2), 100);

		m_isFull = true;

		ui.pushButton_full->setStyleSheet(QString("QPushButton{image:url(%1/Resource/common/max_0.png);border-radius:16px;background-color: rgba(40, 40, 40, 80);}").arg(QApplication::applicationDirPath()));

		showFullScreen();
	}
}

void project_edit_resee::slotNewPicClick(const QString& picPath)
{
	//变换场景
	m_picPath = picPath;
	m_panWidget->changePic(picPath, m_mapPicPathUUid[picPath]);
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
	//刷新
	Reload();
}

void project_edit_resee::slotSetAngleZoom(QString groupID)
{
	//旋转到指定视角
	if (groupID != "")
	{
		HotspotGroupShow hpGpS = m_mapVecGroup[groupID][0];
		if (hpGpS.isPerGroupSeeAngle == true)
		{
			m_panWidget->setRotate(hpGpS.group_rotate_x, hpGpS.group_rotate_y);
			if (hpGpS.isToGroupZoom == true)
				m_panWidget->setCutZoom(hpGpS.gruop_zoom);
			//刷新
			m_panWidget->Update();
		}
	}
}

void project_edit_resee::slotSetSelStyle(QString groupID)
{
	//设置当前选择分组
	if (m_cutGroupID == "")
	{
		m_cutGroupID = groupID;
		m_panWidget->set_SelGroupID(m_cutGroupID);
		for (size_t i = 0; i < ui.listWidget_group->count(); i++)
		{
			QListWidgetItem* listItem = ui.listWidget_group->item(i);
			QString tmpID = listItem->data(Qt::UserRole).toString();
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (tmpID != groupID)
			{
				if (item)
				{
					item->setSelStatus(false);
					item->slotUnSelHPGroup();
				}
			}
			else
			{
				if (item)
					item->setSelStatus(true);
			}
		}
	}
	else
	{
		if (m_cutGroupID == groupID)
		{
			m_cutGroupID = "";
			m_panWidget->set_SelGroupID(m_cutGroupID);
			for (size_t i = 0; i < ui.listWidget_group->count(); i++)
			{
				QListWidgetItem* listItem = ui.listWidget_group->item(i);
				QString tmpID = listItem->data(Qt::UserRole).toString();
				project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
				{
					if (item)
					{
						item->setSelStatus(false);
						item->slotUnSelHPGroup();
					}
				}
			}
		}
		else
		{
			m_cutGroupID = groupID;
			m_panWidget->set_SelGroupID(m_cutGroupID);
			for (size_t i = 0; i < ui.listWidget_group->count(); i++)
			{
				QListWidgetItem* listItem = ui.listWidget_group->item(i);
				QString tmpID = listItem->data(Qt::UserRole).toString();
				project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
				if (tmpID != groupID)
				{
					if (item)
					{
						item->setSelStatus(false);
						item->slotUnSelHPGroup();
					}
				}
				else
				{
					if (item)
					{
						item->setSelStatus(true);
						item->slotSelHPGroup();
					}
				}
			}
		}
	}
}

void project_edit_resee::Slot_autoRotate(int status)
{
	if(status == 0)
		m_panWidget->SetAutoRorate(false);
	else
		m_panWidget->SetAutoRorate(true);
}

void project_edit_resee::SlotShowElement(bool bl)
{
	if (bl == false)
	{
		ui.checkBox_autoRotate->hide();
		ui.listWidget->hide();
		ui.label_pic_bg->hide();
		ui.pushButton_close->hide();
		ui.pushButton_full->hide();
		ui.pushButton_info->hide();
		ui.pushButton_selPic->hide();
	}
	else
	{
		ui.checkBox_autoRotate->show();
		ui.listWidget->show();
		ui.label_pic_bg->show();
		ui.pushButton_close->show();
		ui.pushButton_full->show();

		ui.pushButton_info->hide();
		ui.pushButton_selPic->hide();
	}
}

void project_edit_resee::slotStyle_picText(QString style)
{
	QStringList styleList = style.split(';');
	if (styleList.size() == 2)
	{
		QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
		QString dirName_hp = dirName + "/HotPoints";
		QString dirName_hp_icon = dirName_hp + "/" + styleList[0];
		QStringList picList = searchImages(dirName_hp_icon);

		if (m_picText)
		{
			delete m_picText;
		}
		m_picText = new project_style_picText(picList, styleList[1]);
		m_picText->setWindowModality(Qt::ApplicationModal);
		m_picText->showNormal();
	}
}

void project_edit_resee::slotStyle_picChange(QString style)
{
	QStringList styleList = style.split(';');
	if (styleList.size() == 2)
	{
		QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
		QString dirName_pic = dirName + "/Pictures";
		QString dirName_picPath = dirName_pic + "/" + styleList[1];
		slotNewPicClick(dirName_picPath);
	}
}

void project_edit_resee::SlotDraw()
{
	if (m_draw)
	{
		delete m_draw;
	}
	m_draw = new project_edit_resee_draw(m_projUUID);
	m_draw->setWindowModality(Qt::ApplicationModal);
	m_draw->showNormal();
}

void project_edit_resee::Reload()
{
	//设置第一视角
	{
		m_rotation = QVector3D(0.0, 0.0, 0.0);
		QString data = QString("select start_angle, zoom from picture where id = '%1' LIMIT 1").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);
		while (query.next())
		{
			QString start_angle = query.value(0).toString();
			QStringList angleList = start_angle.split(';');
			if (angleList.size() == 3)
				m_rotation = QVector3D(angleList[0].toFloat(), angleList[1].toFloat(), angleList[2].toFloat());
			m_zoom = query.value(1).toFloat();
		}
		WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_picPath];
		tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
		tmpPam.zoom = QString::number(m_zoom, 'f', 1);
		m_panWidget->m_mapPicAngle[m_picPath] = tmpPam;
	}
	//加载热点
	{
		m_vecCurPicHp.clear();
		QString data = QString("select * from hotpoint where picture_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			Hotspot hp;
			hp.iconID = query.value(0).toString();
			hp.position = QVector3D(0, 0, -1);
			QString strPos = query.value(5).toString();
			QStringList posList = strPos.split(',');
			if (posList.size() == 3)
				hp.position = QVector3D(posList[0].toFloat(), posList[1].toFloat(), posList[2].toFloat());
			hp.iconPath = query.value(3).toString();
			hp.name = query.value(1).toString();
			hp.description = "";
			hp.style = query.value(4).toString();
			hp.scale_x = query.value(6).toFloat();
			hp.scale_y = query.value(7).toFloat();
			hp.lock = query.value(8).toBool();
			hp.icon_visible = query.value(9).toBool();
			hp.title_visible = query.value(10).toBool();
			hp.isFollowSysZoom = query.value(11).toBool();
			hp.title_bg_color = QColor(30, 30, 30, 255);
			hp.title_text_color = QColor(255, 255, 255, 255);
			QStringList bgColorList = query.value(13).toString().split(",");
			QStringList textColorList = query.value(14).toString().split(",");
			int size = query.value(15).toInt();
			if (bgColorList.size() == 3)
				hp.title_bg_color = QColor(bgColorList[0].toInt(), bgColorList[1].toInt(), bgColorList[2].toInt());

			if (textColorList.size() == 3)
				hp.title_text_color = QColor(textColorList[0].toInt(), textColorList[1].toInt(), textColorList[2].toInt());
			hp.title_text_size = size;
			//
			m_vecCurPicHp.push_back(hp);
			m_currentHpItemIndex = 0;
		}
	}
	//载入层次
	{
		std::map<QString, std::vector<HotspotLevelShow>> mapVecIconID_Zoom;
		QString data = QString("select d.icon_id, u.*from level u LEFT JOIN levelshow d ON u.level_id = d.level_id where u.pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			HotspotLevelShow level;
			level.iconID = query.value(0).toString();
			level.picID = query.value(1).toString();
			level.min_zoom = query.value(2).toFloat();
			level.max_zoom = query.value(3).toFloat();
			level.LevelID = query.value(4).toString();

			mapVecIconID_Zoom[level.iconID].push_back(level);
		}
		m_panWidget->setIconShowZooms(mapVecIconID_Zoom);
		//是否按层次显示
		{
			QString data = QString("select isLevelShow from picture where id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
			QSqlQuery query(data);//查询表的内容
			while (query.next())
			{
				QString islevelShow = query.value(0).toString();
				if (islevelShow == '2')
					m_panWidget->setLevelShow(true);
				else
					m_panWidget->setLevelShow(false);
			}
		}
	}
	//载入分组
	{
		ui.listWidget_group->hide();
		m_DefGroup = "";
		clearListWidgetGroup();
		m_curInsertGroupNum = 0;
		m_mapVecGroup.clear();
		m_vecGroupID.clear();
		//加入场景
		std::map<QString, std::vector<HotspotGroupShow>> mapVecIconID_Group;
		QString data = QString("select d.icon_id, u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			HotspotGroupShow group;
			group.iconID = query.value(0).toString();
			group.picID = query.value(1).toString();
			group.groupName = query.value(2).toString();
			group.isGroupShowInPic = query.value(3).toBool();
			group.isPerGroupSeeAngle = query.value(4).toBool();

			group.group_rotate_x = query.value(5).toFloat();
			group.group_rotate_y = query.value(6).toFloat();
			group.gruop_zoom = query.value(7).toFloat();

			group.isToGroupZoom = query.value(8).toBool();
			group.isdefOpen = query.value(9).toBool();
			group.isIncludeAll = query.value(10).toBool();
			group.GroupID = query.value(11).toString();

			//场景中不显示
			if (group.isGroupShowInPic == 0) continue;
			//默认分组
			if (group.isdefOpen == true)
				m_DefGroup = group.GroupID;

			mapVecIconID_Group[group.iconID].push_back(group);
			m_mapVecGroup[group.GroupID].push_back(group);
			//
			{
				bool isExist = false;
				for (const QString& str : m_vecGroupID) { // 遍历容器中所有元素
					if (str == group.GroupID) {        // 逐元素对比
						isExist = true;
						break;                   // 找到后立即退出循环，优化性能
					}
				}
				if (isExist == false)
				{
					m_vecGroupID.push_back(group.GroupID);
				}
			}
			//存在分组就显示
			ui.listWidget_group->show();
		}
		m_panWidget->setIconShowGroups(mapVecIconID_Group);
	}
	//加载遮罩
	{
		m_panWidget->setRegion_valid(true);
		QString style;
		QString pamLong = "0;360;0;180";
		{
			QString data = QString("select * from mask where pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
			QSqlQuery query(data);
			while (query.next())
			{
				style = query.value(2).toString();
				pamLong = query.value(3).toString();
			}
		}
		QStringList range = pamLong.split(";");
		m_panWidget->setRegion_theta_phi(range[0].toDouble(), range[1].toDouble(), range[2].toDouble(), range[3].toDouble());
	}
	//加载热点
	m_panWidget->clearCutPicHp();
	QTimer::singleShot(0, this, &project_edit_resee::processNextHpItem);
}


void project_edit_resee::clearListWidgetGroup()
{
	// 1. 遍历所有项（从后往前删，避免索引错乱）
	for (int i = ui.listWidget_group->count() - 1; i >= 0; --i) {
		// 步骤1：取出当前项（takeItem会将项从列表中移除，但不会删除内存）
		QListWidgetItem* item = ui.listWidget_group->takeItem(i);
		if (item == nullptr) {
			continue; // 极端情况：项为空，跳过
		}

		// 步骤2：获取绑定的自定义控件并清理
		project_edit_group_item* customItem =
			qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(item));
		if (customItem) {
			customItem->disconnect(); // 断开所有信号槽，避免野指针触发信号
			delete customItem; // 显式删除自定义控件（核心：释放内存）
			customItem = nullptr;
		}

		// 步骤3：删除QListWidgetItem本身
		delete item;
		item = nullptr;
	}
	// 2. 清空列表的内部缓存（兜底，确保无残留）
	ui.listWidget_group->clear();
	
}


void project_edit_resee::processNextPicItem()
{
	if (m_currentPicItemIndex >= m_listPicPath.size())
	{
		return;
	}
	QString picPath = m_listPicPath[m_currentPicItemIndex];
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(80, 80));
	item->setData(Qt::UserRole, picPath);
	ui.listWidget->addItem(item);
	project_see_item* item2 = new project_see_item(picPath);
	item2->resize(QSize(80, 80));
	connect(item2, SIGNAL(sig_picpath(QString)), this, SLOT(slotNewPicClick(QString)));
	ui.listWidget->setItemWidget(item, item2);
	if (m_currentPicItemIndex == 0)
	{
		item2->setFocus();
		//加载热点
		m_panWidget->clearCutPicHp();
		QTimer::singleShot(0, this, &project_edit_resee::processNextHpItem);
	}

	m_currentPicItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(0, this, &project_edit_resee::processNextPicItem);
}

void project_edit_resee::processNextHpItem()
{
	//加载热点
	if (m_currentHpItemIndex >= m_vecCurPicHp.size())
	{
		//加载界面分组
		QTimer::singleShot(0, this, &project_edit_resee::processNextGroupItem);
		return;
	}
	//添加到场景
	m_panWidget->addHotspot2List(m_vecCurPicHp[m_currentHpItemIndex]);
	m_currentHpItemIndex++;

	QTimer::singleShot(0, this, &project_edit_resee::processNextHpItem);
}

void project_edit_resee::processNextGroupItem()
{
	//加载分组
	if (m_curInsertGroupNum >= m_mapVecGroup.size())
	{
		if (m_DefGroup != "")
		{
			for (size_t i = 0; i < ui.listWidget_group->count(); i++)
			{
				QListWidgetItem* listItem = ui.listWidget_group->item(i);
				QString tmpID = listItem->data(Qt::UserRole).toString();
				project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
				if (tmpID == m_DefGroup)
				{
					item->clickButton();
					break;
				}
			}
		}
		return;
	}
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	scrWidth -= 100;
	scrHeight -= 100;

	int size = ui.listWidget_group->count() + 1;

	int sizeH = size * 46;
	if (size * 46 > scrHeight - 300)
		sizeH = scrHeight - 300;
	ui.listWidget_group->resize(110, sizeH);
	ui.listWidget_group->move(10, (scrHeight - 6 - 140 - sizeH) / 2);

	//添加到List
	std::vector<HotspotGroupShow> lvShow = m_mapVecGroup[m_vecGroupID[m_curInsertGroupNum]];
	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(230, 36));
	item->setData(Qt::UserRole, m_vecGroupID[m_curInsertGroupNum]);
	ui.listWidget_group->addItem(item);

	GroupItemStyle gpStyle;
	gpStyle.groupName = lvShow[0].groupName;
	gpStyle.groupID = lvShow[0].GroupID;
	QString picID = lvShow[0].picID;
	{
		QString data = QString("select group_unsel_bg_color,group_unsel_text_color,group_sel_bg_color,group_sel_text_color \
			from picture where pic_id = '%1' LIMIT 1").arg(picID);
		QSqlQuery query(data);//查询表的内容
		while (query.next())
		{
			gpStyle.unsel_bgColor = query.value(0).toString();
			gpStyle.unsel_textColor = query.value(1).toString();
			gpStyle.sel_bgColor = query.value(2).toString();
			gpStyle.sel_textColor = query.value(3).toString();
		}
	}
	//分组样式
	m_vecGroupStyle.push_back(gpStyle);
	//
	project_edit_group_item* item2 = new project_edit_group_item(gpStyle);

	connect(item2, SIGNAL(sigDisconnect()), this, SLOT(slotDisconnect()));
	connect(item2, SIGNAL(sigConnect()), this, SLOT(slotConnect()));

	connect(item2, SIGNAL(itemClicked(QString)), this, SLOT(slotSetAngleZoom(QString)));
	connect(item2, SIGNAL(itemClicked(QString)), this, SLOT(slotSetSelStyle(QString)));

	//点击相应
	item2->clickEvent();
	item2->setFocusPolicy(Qt::NoFocus);
	item2->resize(QSize(230, 36));
	ui.listWidget_group->setItemWidget(item, item2);
	//
	m_curInsertGroupNum++;
	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit_resee::processNextGroupItem);
}
