#include "project_edit.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include "project_see_item.h"
#include "project_edit_group_item.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>


project_edit::project_edit(QString projUUID,QString project_name, QStringList listPicPath, QWidget *parent):
	m_projUUID(projUUID)
	, m_project_name(project_name)
	, m_listPicPath(listPicPath)
	,QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_msgBox = nullptr;

	ui.openGLWidget_center->hide();

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->availableGeometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height();
	resize(scrWidth, scrHeight);

	m_picPath = m_listPicPath[0];
	m_panWidget = new PanoramaWidget(m_listPicPath[0], false, false,this);
	m_panWidget->setGroupStatus(u8"编辑");
	connect(m_panWidget, SIGNAL(sig_transEnd()), this, SLOT(slotSetFirstAngle()));
	connect(m_panWidget, SIGNAL(sig_cutSeeAngle(QString)), this, SLOT(slotSetSeeAngle(QString)));
	connect(m_panWidget, SIGNAL(sig_seeAngle(float)), this, SIGNAL(sig_updateSeeAngle(float)));
	connect(m_panWidget, SIGNAL(sig_style_picText(QString)), this, SLOT(slotStyle_picText(QString)));
	connect(m_panWidget, SIGNAL(sig_style_picChange(QString)), this, SLOT(slotStyle_picChange(QString)));
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
		m_panWidget->SetAllPicAngle( mapPicAngle);
		m_panWidget->SetAllPicCompass(mapPicCompass);

		m_panWidget->initWindow(m_listPicPath[0]);
	}
	//
	m_panWidget->SetPicID(m_mapPicPathUUid[m_picPath]);

	ui.openGLWidget_center = m_panWidget;

	ui.label_bg->setGeometry(0, 0, scrWidth, scrHeight);
	ui.label_topTitle->move(20, 20);
	ui.pushButton_close->setGeometry(scrWidth - 36, 4, 32, 32);

	ui.groupBox_left->setGeometry(0,60, 61, scrHeight - 60);
	ui.label_leftLable->setGeometry(0, 0, 61, scrHeight - 60);

	ui.openGLWidget_center->setGeometry(60, 60, scrWidth - 60 - 250, scrHeight - 60 - 140);

	ui.label_seeAngle->move(60 + (scrWidth - 60 - 250 - 700) / 2,60 + (scrHeight - 60 - 140 - 400) / 2);
	ui.label_hpLocation->move(60 + (scrWidth - 60 - 250 - 700) / 2, 60 + (scrHeight - 60 - 140 - 400) / 2);

	ui.checkBox_blFrontCover->move(60 + (scrWidth - 60 - 250 - 700) / 2 + 280, 60 + (scrHeight - 60 - 140 - 400) / 2 + 300);
	ui.pushButton_FirstAngle->move(60 + (scrWidth - 60 - 250 - 700) / 2 + 250, 60 + (scrHeight - 60 - 140 - 400) / 2 + 340);
	ui.pushButton_compassN->move(60 + (scrWidth - 60 - 250 - 700) / 2 + 250, 60 + (scrHeight - 60 - 140 - 400) / 2 + 340);
	ui.pushButton_GroupAngle->move(60 + (scrWidth - 60 - 250 - 700) / 2 + 250, 60 + (scrHeight - 60 - 140 - 400) / 2 + 340);

	ui.groupBox_right->setGeometry(scrWidth - 250, 60, 250, scrHeight - 60);
	ui.label_rightLable->setGeometry(0, 0, 250, scrHeight - 60);	
	ui.stackedWidget_right->setGeometry(0, 0, 250, scrHeight - 60);

	ui.groupBox_bottom->setGeometry(60, scrHeight - 140, scrWidth - 60 - 250, 140);
	ui.listWidget->setGeometry(0, 30, scrWidth - 60 - 250 - 2 * 30 - 75 - 20 - 90 - 10, 140);
	ui.label_bottomLable->setGeometry(0, 0, scrWidth - 60 - 250, 140);
	ui.pushButton_reSee->move(scrWidth - 60 - 250 - 90, 20);

	ui.pushButton_delPic->move(scrWidth - 60 - 250 - 10 - 75 - 90, 20);
	ui.pushButton_addPic->move(scrWidth - 60 - 250 - 10  - 75 - 90,60);
	

	ui.groupBox_level->move(scrWidth - 250 - 140 - 20, 80);
	ui.listWidget_group->move(70, 300);
	//
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载场景...", u8"取消", 0, m_listPicPath.size(), this);
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
	//
	ui.label_seeAngle->raise();
	ui.label_hpLocation->raise();
	ui.checkBox_blFrontCover->raise();

	ui.pushButton_FirstAngle->raise();
	ui.pushButton_compassN->raise();
	ui.pushButton_GroupAngle->raise();

	ui.groupBox_level->raise();
	ui.listWidget_group->raise();
	m_progressDialog->raise();

	ui.label_seeAngle->setAttribute(Qt::WA_TransparentForMouseEvents, true);
	ui.label_hpLocation->setAttribute(Qt::WA_TransparentForMouseEvents, true);

	//connect
	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.checkBox_isLevelShow, SIGNAL(stateChanged(int)), this, SLOT(SlotSetLevelShow(int)));

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

	ui.listWidget->setViewMode(QListView::IconMode);
	ui.listWidget->setSpacing(10);
	ui.listWidget->setMovement(QListView::Static);
	ui.listWidget->setFocusPolicy(Qt::NoFocus);
	ui.listWidget->setSelectionMode(QAbstractItemView::SingleSelection); 
	ui.listWidget->setStyleSheet(
		"QListWidget{"
		"    background-color: rgba(255, 255, 255, 0);"
		"    border: 1px;"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item {"
		"    border: 0px solid #dee2e6;"
		"    border-radius: 5px;"
		"}"
		"QListWidget::item:selected {"
		"    border-radius: 5px;"
		"    border: 3px solid #fa6400;"
		"}"
		"QListWidget::item:hover {"
		"    border-radius: 5px;"
		"    border: 3px solid #fa6400;"
		"}"
	);

	initSheet();
	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));

	connect(ui.pushButton_globle, SIGNAL(clicked()), this, SLOT(slotGloble()));
	connect(ui.pushButton_seeAngle, SIGNAL(clicked()), this, SLOT(slotSeeAngle()));
	connect(ui.pushButton_compass, SIGNAL(clicked()), this, SLOT(slotCompass()));
	connect(ui.pushButton_mask, SIGNAL(clicked()), this, SLOT(slotMask()));
	connect(ui.pushButton_hotPoint, SIGNAL(clicked()), this, SLOT(slotHotPoint()));

	connect(ui.pushButton_FirstAngle, SIGNAL(clicked()), this, SLOT(slotSetFirstAngle_FrontCover()));
	connect(ui.pushButton_GroupAngle, SIGNAL(clicked()), this, SLOT(slotSetGroupAngle()));
	connect(ui.pushButton_compassN, SIGNAL(clicked()), this, SLOT(slotSetCompassN()));

	connect(ui.pushButton_addPic, SIGNAL(clicked()), this, SLOT(slotAddPic()));
	connect(ui.pushButton_delPic, SIGNAL(clicked()), this, SLOT(slotDelPic()));
	connect(ui.pushButton_reSee, SIGNAL(clicked()), this, SLOT(slotReSee()));

	ui.pushButton_globle->click();
	m_cutPage = "globle";

	resize(scrWidth, scrHeight);
	// 使用定时器开始处理
	m_currentItemIndex = 0;
	QTimer::singleShot(0, this, &project_edit::processNextPicItem);
}

project_edit::~project_edit()
{}

void project_edit::slotClose()
{
	emit sig_Close();
	emit sig_update();
	this->close();
	this->destroy(true);
}

void project_edit::processNextPicItem()
{
	if (m_currentItemIndex >= m_listPicPath.size()|| (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}
	QString picPath = m_listPicPath[m_currentItemIndex];
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(80, 80));
	item->setData(Qt::UserRole, picPath);
	item->setData(Qt::UserRole + 1, m_mapPicPathUUid[picPath]);
	ui.listWidget->addItem(item);
	project_see_item* item2 = new project_see_item(picPath);
	item2->resize(QSize(80, 80));
	connect(item2, SIGNAL(sig_picpath(QString)), this, SLOT(slotNewPicClick(QString)));
	ui.listWidget->setItemWidget(item, item2);
	if (m_currentItemIndex == 0)
	{
		item2->setFocus();
		item2->slotPicPath();
	}
	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit::processNextPicItem);
}

void project_edit::processNextPicItem_Add()
{
	if (m_currentItemIndex >= m_tmpPathList.size() || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}
	QString picPath = m_tmpPathList[m_currentItemIndex];
	QString picUUID = getUUID();
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
	QFileInfo fileInfo(picPath);
	QString suf = fileInfo.completeSuffix();
	QString newName = picPath.section("/", -1).section(".", 0, -2);
	QString newPath = dirName + "/Pictures/" + newName + "." + suf;
	m_mapUUidPicPath.insert(std::make_pair(picUUID, newPath));
	m_mapPicPathUUid.insert(std::make_pair(newPath, picUUID));
	//复制图片
	CopyFile(picPath, newPath, true);
	//拇指图
	{
		QPixmap currentImage = QPixmap(newPath);
		int x = (currentImage.width() - 512) / 2;
		int y = (currentImage.height() - 512) / 2;
		QPixmap endImage = currentImage.copy(x, y, 512, 512);
		QString thumbPath = dirName + "/Pictures_Thumb/" + newName + "." + suf;
		endImage.save(thumbPath, suf.toUtf8());
	}
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(80, 80));
	item->setData(Qt::UserRole, newPath);
	item->setData(Qt::UserRole + 1, m_mapPicPathUUid[newPath]);
	ui.listWidget->addItem(item);
	project_see_item* item2 = new project_see_item(newPath);
	item2->resize(QSize(80, 80));
	connect(item2, SIGNAL(sig_picpath(QString)), this, SLOT(slotNewPicClick(QString)));
	ui.listWidget->setItemWidget(item, item2);
	//添加
	m_listPicPath.append(newPath);
	//插入数据库
	QString uuid = picUUID;
	QString front_cover = newPath;
	QString projectName = m_project_name;
	QString start_angle = "0.0;0.0;0.0";
	QString fov = "30.0";
	QString zoom = "1.0";
	QString zoom_range = "0.5;3.0";
	QString h_range = "-360.0;360.0";
	QString v_range = "-90.0;90.0";

	QString compass_s_angle = "0.0";
	QString compass_pic_path = "";
	QString compass_location = "LeftTop"; // LeftTop,BottomCentre

	QString unsel_bgColor = "26,26,26";
	QString unsel_textColor = "255,255,255";
	QString sel_bgColor = "250,100,0";
	QString sel_textColor = "255,255,255";

	//设置视角
	WindowParameter pam;
	pam.start_angle = start_angle;
	pam.fov = fov;
	pam.zoom = zoom;
	pam.zoom_range = zoom_range;
	pam.h_range = h_range;
	pam.v_range = v_range;
	m_panWidget->SetOnePicAngle(newPath,pam);
	//设置指北针
	CompassParameter comp;
	comp.compassAngle = 0.0;
	comp.picPath = compass_pic_path;
	comp.location = compass_location;
	m_panWidget->SetOnePicCompass(newPath, comp);
	{
		//全景图片表
		QString sql = QString("INSERT INTO picture( \
				id, \
				picture_path, \
				project_name_id, \
				project_name, \
				start_angle,\
				fov,\
				zoom,\
				zoom_range,\
				h_range,\
				v_range,\
				compass_s_angle,\
				compass_pic_path,\
				compass_location,\
				group_unsel_bg_color,\
				group_unsel_text_color,\
				group_sel_bg_color,\
				group_sel_text_color\
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
				'%11', \
				'%12', \
				'%13',\
				'%14',\
				'%15',\
				'%16',\
				'%17'\
			); ")
			.arg(picUUID)
			.arg(newPath)
			.arg(m_projUUID)
			.arg(projectName)
			.arg(start_angle)
			.arg(fov)
			.arg(zoom)
			.arg(zoom_range)
			.arg(h_range)
			.arg(v_range)
			.arg(compass_s_angle)
			.arg(compass_pic_path)
			.arg(compass_location)
			.arg(unsel_bgColor)
			.arg(unsel_textColor)
			.arg(sel_bgColor)
			.arg(sel_textColor);
		//执行语句
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	//
	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit::processNextPicItem_Add);
}

void project_edit::initSheet()
{
	ui.pushButton_globle->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/globle_0.png);border-radius:5px;background-color: rgba(255, 255, 255,0);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_seeAngle->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/seeAngle_0.png);border-radius:5px;background-color: rgba(255, 255, 255,0);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_compass->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/compass_0.png);border-radius:5px;background-color: rgba(255, 255, 255,0);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_mask->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/mask_0.png);border-radius:5px;background-color: rgba(255, 255, 255,0);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	ui.pushButton_hotPoint->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/hotpoint_0.png);border-radius:5px;background-color: rgba(255, 255, 255,0);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));

	ui.pushButton_FirstAngle->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_compassN->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	ui.pushButton_addPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 40, 40);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(80, 80, 80);}"));
	ui.pushButton_delPic->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 40, 40);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(80, 80, 80);}"));
	ui.pushButton_reSee->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 40, 40);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(80, 80, 80);}"));
}

void project_edit::slotDelPic(std::vector<QString> vecPicID)
{
	//是否删除当前场景
	bool isHaveCutPic = false;
	for (auto tmp : vecPicID)
	{
		if (tmp == m_mapPicPathUUid[m_picPath])
		{
			isHaveCutPic = true;
			break;
		}
	}
	//
	for (auto tmp : vecPicID)
	{
		//类中删除
		m_listPicPath.removeOne(m_mapUUidPicPath[tmp]);
		m_mapPicPathUUid.erase(tmp);
		for (auto it = m_mapUUidPicPath.begin(); it != m_mapUUidPicPath.end(); ++it)
		{
			if (it->second == tmp) 
			{
				m_mapUUidPicPath.erase(it);  
				break;            
			}
		}
		//当前场景被删除
		if (isHaveCutPic == true && m_listPicPath.size() >= 1)
		{
			//单击剩余的第一个图片
			slotNewPicClick(m_listPicPath[0]);
		}
		//场景中删除
		m_panWidget->DelPic(vecPicID);
		//list中删除
		for (size_t i = 0; i < ui.listWidget->count(); i++)
		{
			QListWidgetItem* listItem = ui.listWidget->item(i);
			QString tmpID = listItem->data(Qt::UserRole + 1).toString();
			if (tmpID == tmp)
			{
				project_see_item* item = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
				if (item)
				{
					delete item;
					QListWidgetItem* removedItem = ui.listWidget->takeItem(i);
					if (removedItem)
						delete removedItem;
					break;
				}
			}
		}
		//数据库中删除
		{
			QString sql = QString("delete from picture where id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
		{
			QString sql = QString("delete from hotpoint where picture_id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
		{
			QString sql = QString("delete from level where pic_id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
		{
			QString sql = QString("delete from levelshow where pic_id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
		{
			QString sql = QString("delete from group where pic_id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
		{
			QString sql = QString("delete from groupshow where pic_id = '%1'").arg(tmp);
			QSqlQuery query;
			query.exec(sql);
		}
	}
}

void project_edit::SlotSetLevelShow(int status)
{
	m_panWidget->setLevelShow(status);
	m_panWidget->Update();
	//修改数据库
	{
		QString updata = QString("update picture set isLevelShow = '%1' where id = '%7'").arg(status).arg(m_mapPicPathUUid[m_picPath]);
		QSqlQuery query;
		query.exec(updata);
	}	
}

void project_edit::slotMsg(QString msg)
{
	noticeWin.Notice(this, msg, 2000);
}

void project_edit::slotSeeAngleSave()
{
	int isuccessNum = 0;
	for (auto tmpPic : m_listPicPath)
	{
		WindowParameter picPam = m_panWidget->m_mapPicAngle[tmpPic];
		//更新数据库
		QString updata = QString("update picture set start_angle = '%1', fov = '%2',zoom = '%3',zoom_range = '%4',h_range = '%5',v_range = '%6' where picture_path = '%7' and  project_name_id = '%8'")
			.arg(picPam.start_angle).arg(picPam.fov).arg(picPam.zoom).arg(picPam.zoom_range).arg(picPam.h_range).arg(picPam.v_range).arg(tmpPic).arg(m_projUUID);
		QSqlQuery query;
		if (!query.exec(updata))
		{
			break;
		}
		else
		{
			isuccessNum++;
		}
	}
	if (isuccessNum == m_listPicPath.size())
		noticeWin.Notice(this, u8"保存成功！", 2000);
	else
		noticeWin.Notice(this, u8"保存失败！", 2000);
}

void project_edit::slotCompassSave()
{
	int isuccessNum = 0;
	for (auto tmpPic : m_listPicPath)
	{
		CompassParameter picPam = m_panWidget->m_mapPicCompass[tmpPic];
		//更新数据库
		QString updata = QString("update picture set compass_s_angle = '%1', compass_pic_path = '%2',compass_location = '%3' where picture_path = '%4' and  project_name_id = '%5'")
			.arg(picPam.compassAngle).arg(picPam.picPath).arg(picPam.location).arg(tmpPic).arg(m_projUUID);
		QSqlQuery query;
		if (!query.exec(updata))
		{
			break;
		}
		else
		{
			isuccessNum++;
		}
	}
	if (isuccessNum == m_listPicPath.size())
		noticeWin.Notice(this, u8"保存成功！", 2000);
	else
		noticeWin.Notice(this, u8"保存失败！", 2000);
}

void project_edit::slotNewPicClick(const QString& picPath)
{
	//if (picPath == m_picPath) return;

	//关闭热点下边的菜单
	emit sig_Close();//热点界面多层界面关闭
	//变换场景
	m_picPath = picPath;
	m_panWidget->changePic(picPath, m_mapPicPathUUid[picPath]);
	//下方场景图选中样式
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString tmpPicPath = listItem->data(Qt::UserRole).toString();		
		if (tmpPicPath != picPath) continue;
		project_see_item* customItem = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
		if (customItem)
		{
			customItem->setFocus();
			break;
		}
	}	
	//
	if (m_cutPage == "Globle")
	{
		LoadPic_Globle(picPath);
	}
	else if (m_cutPage == "SeeAngle")
	{
		LoadPic_SeeAngle(picPath);
	}
	else if (m_cutPage == "Compass")
	{
		LoadPic_Compass(picPath);
	}
	else if (m_cutPage == "Mask")
	{
		LoadPic_Mask(picPath);
	}
	else if (m_cutPage == "HotPoint")
	{
		LoadPic_HotPoint(picPath);
	}
}

void project_edit::processNextIconItem()
{
	//加载热点
	if (m_curInsertNum >= m_vecCurPicHp.size())
	{
		return;
	}
	//添加到场景
	m_panWidget->addHotspot2List(m_vecCurPicHp[m_curInsertNum]);
	m_curInsertNum++;
	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit::processNextIconItem);
}

void project_edit::processNextGroupItem()
{
	//加载分组
	if (m_curInsertGroupNum >= m_mapVecGroup.size())
	{
		return;
	}
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	int size = ui.listWidget_group->count() + 1;

	int sizeH = size * 46;
	if (size * 46 > scrHeight - 300)
		sizeH = scrHeight - 300;
	ui.listWidget_group->resize(110, sizeH);
	ui.listWidget_group->move(70, (scrHeight - 6 - 140 - sizeH) / 2);

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

	project_edit_group_item* item2 = new project_edit_group_item(gpStyle);
	connect(item2, SIGNAL(itemClicked(QString)), this, SLOT(slotSetGroupOtherItemStyle(QString)));
	item2->setFocusPolicy(Qt::NoFocus);
	item2->resize(QSize(230, 36));
	ui.listWidget_group->setItemWidget(item, item2);
	//
	m_curInsertGroupNum++;
	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_edit::processNextGroupItem);
}

void project_edit::slotGloble()
{	
	emit sig_Close();//热点界面多层界面关闭
	m_cutPage = "Globle";
	initSheet();
	//热点，指北针
	m_panWidget->setHotspotShow(false);
	m_panWidget->setCompassShow(false);
	//
	ui.label_seeAngle->hide();
	ui.label_hpLocation->hide();
	ui.checkBox_blFrontCover->hide();
	ui.pushButton_FirstAngle->hide();
	ui.pushButton_compassN->hide();
	m_panWidget->setCompassNShow(false);

	ui.groupBox_level->hide();
	ui.listWidget_group->hide();
	ui.pushButton_GroupAngle->hide();

	ui.pushButton_globle->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/globle_1.png);border-radius:5px;background-color: rgb(40, 110, 250);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));

	if (m_globle)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_globle;
	}
	m_globle = new project_edit_globle(m_projUUID, m_picPath,this);
	connect(m_globle, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	ui.stackedWidget_right->addWidget(m_globle);
	ui.stackedWidget_right->setCurrentWidget(m_globle);
}

void project_edit::slotSeeAngle()
{	
	emit sig_Close();//热点界面多层界面关闭
	m_cutPage = "SeeAngle";
	initSheet();
	//热点，指北针
	m_panWidget->setHotspotShow(false);
	m_panWidget->setCompassShow(false);
	//
	ui.label_seeAngle->show();
	ui.label_hpLocation->hide();
	ui.checkBox_blFrontCover->show();
	ui.pushButton_FirstAngle->show();
	ui.pushButton_compassN->hide();
	m_panWidget->setCompassNShow(false);

	ui.groupBox_level->hide();
	ui.listWidget_group->hide();
	ui.pushButton_GroupAngle->hide();

	ui.pushButton_seeAngle->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/seeAngle_1.png);border-radius:5px;background-color: rgb(40, 110, 250);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	if (m_seeAngle)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_seeAngle;
	}
	m_seeAngle = new project_edit_seeAngle(m_projUUID, m_mapPicPathUUid[m_picPath], this);
	connect(m_seeAngle, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_seeAngle, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_seeAngle, SIGNAL(sig_save()), this, SLOT(slotSeeAngleSave()));

	connect(m_seeAngle, SIGNAL(sig_setFirstAnglePic(std::vector<QString>&)), this, SLOT(slotSetSelPicFirstAngle(std::vector<QString>&)));
	connect(m_seeAngle, SIGNAL(sig_setPicFOV(std::vector<QString>&, float, float, float)), this, SLOT(slotSetSelPicFOV(std::vector<QString>&, float, float, float)));
	connect(m_seeAngle, SIGNAL(sig_setPicHV(std::vector<QString>&, int, int, int,int)), this, SLOT(slotSetSelPicHV(std::vector<QString>&, int, int, int, int)));

	connect(m_seeAngle, SIGNAL(sig_cut_zoom_angle(float)), this, SLOT(slotSetCutZoom(float)));
	connect(m_seeAngle, SIGNAL(sig_H_angle(int)), this, SLOT(slotHAngle(int)));
	connect(m_seeAngle, SIGNAL(sig_V_angle(int)), this, SLOT(slotVAngle(int)));

	ui.stackedWidget_right->addWidget(m_seeAngle);
	ui.stackedWidget_right->setCurrentWidget(m_seeAngle);
	slotSetFirstAngle();
}

void project_edit::slotCompass()
{
	emit sig_Close();//热点界面多层界面关闭
	m_cutPage = "Compass";
	initSheet();
	//热点，指北针
	m_panWidget->setHotspotShow(false);
	m_panWidget->setCompassShow(true);
	//
	ui.label_seeAngle->show();
	ui.label_hpLocation->hide();
	ui.checkBox_blFrontCover->hide();
	ui.pushButton_FirstAngle->hide();
	ui.pushButton_compassN->show();
	m_panWidget->setCompassNShow(true);

	ui.groupBox_level->hide();
	ui.listWidget_group->hide();
	ui.pushButton_GroupAngle->hide();

	ui.pushButton_compass->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/compass_1.png);border-radius:5px;background-color: rgb(40, 110, 250);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	if (m_compass)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_compass;
	}
	m_compass = new project_edit_compass(m_projUUID, m_mapPicPathUUid[m_picPath],this);
	//指北针图片
	m_compass->setCompassPic(m_panWidget->m_mapPicCompass[m_picPath].picPath);
	connect(m_compass, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_compass, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_compass, SIGNAL(sig_setPicCompass(std::vector<QString>&, float, QString, QString)), this, SLOT(slotSetSelPicCompass(std::vector<QString>&, float, QString, QString)));
	connect(m_compass, SIGNAL(sig_setCompassPic(QString)), this, SLOT(slotSetPicCompassPic(QString)));
	connect(m_compass, SIGNAL(sig_setCompassPos(QString)), this, SLOT(slotSetPicCompassPos(QString)));
	connect(this, SIGNAL(sig_Angle(float)), m_compass, SLOT(slotSetAngle(float)));
	
	ui.stackedWidget_right->addWidget(m_compass);
	ui.stackedWidget_right->setCurrentWidget(m_compass);
	slotSetFirstAngle();
}

void project_edit::slotMask()
{
	emit sig_Close();//热点界面多层界面关闭
	m_cutPage = "Mask";
	initSheet();
	//热点，指北针
	m_panWidget->setHotspotShow(false);
	m_panWidget->setCompassShow(false);
	//
	ui.label_seeAngle->hide();
	ui.label_hpLocation->hide();
	ui.checkBox_blFrontCover->hide();
	ui.pushButton_FirstAngle->hide();
	ui.pushButton_compassN->hide();
	m_panWidget->setCompassNShow(false);

	ui.groupBox_level->hide();
	ui.listWidget_group->hide();
	ui.pushButton_GroupAngle->hide();

	ui.pushButton_mask->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/mask_1.png);border-radius:5px;background-color: rgb(40, 110, 250);}"
		"QPushButton::hover{background-color: rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));
	if (m_mask)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_mask;
	}
	m_mask = new project_edit_mask(m_projUUID, this);
	connect(m_mask, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_mask, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_mask, SIGNAL(sig_save()), this, SLOT(slotCompassSave()));
	ui.stackedWidget_right->addWidget(m_mask);
	ui.stackedWidget_right->setCurrentWidget(m_mask);
	slotSetFirstAngle();
}

void project_edit::slotHotPoint()
{
	m_cutPage = "HotPoint";
	initSheet();
	//热点，指北针
	m_panWidget->setHotspotShow(true);
	m_panWidget->setCompassShow(false);
	//
	ui.label_seeAngle->hide();
	ui.label_hpLocation->hide();
	ui.checkBox_blFrontCover->hide();
	ui.pushButton_FirstAngle->hide();
	ui.pushButton_compassN->hide();
	m_panWidget->setCompassNShow(false);

	ui.groupBox_level->hide();
	ui.listWidget_group->hide();

	ui.pushButton_hotPoint->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/edit/hotpoint_1.png);border-radius:5px;background-color: rgb(40, 110, 250);}"
		"QPushButton::hover{background-color:rgb(40, 110, 250);}").arg(QApplication::applicationDirPath()));

	if (m_hotpoint)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_hotpoint;
	}
	m_hotpoint = new project_edit_hotpoint(m_projUUID, m_picPath, m_mapPicPathUUid[m_picPath],this);
	connect(m_hotpoint, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(m_hotpoint, SIGNAL(sig_hp_set(QString)), this, SLOT(slotHotPoint_Set(QString)));
	connect(m_hotpoint, SIGNAL(sig_addIcon(Hotspot)), this, SLOT(slotAddIcon(Hotspot)));
	connect(m_hotpoint, SIGNAL(sig_updateIcon(QString)), this, SLOT(slotUpdateIcon(QString)));
	connect(m_hotpoint, SIGNAL(sig_scale_x(float)), this, SLOT(slotSetScaleX(float)));
	connect(m_hotpoint, SIGNAL(sig_scale_y(float)), this, SLOT(slotSetScaleY(float)));
	connect(m_hotpoint, SIGNAL(sig_name(QString)), this, SLOT(slotSetName(QString)));
	connect(m_hotpoint, SIGNAL(sig_name_show(bool)), this, SLOT(slotNameShow(bool)));
	connect(m_hotpoint, SIGNAL(sig_add_icon_end()), this, SLOT(slotAddIconEnd()));
	connect(m_hotpoint, SIGNAL(sig_edit_icon_end()), this, SLOT(slotEditIconEnd()));
	connect(m_hotpoint, SIGNAL(sig_del_tmp_icon()), this, SLOT(slotDelTmpIcon()));
	connect(m_hotpoint, SIGNAL(sig_copy_icon(Hotspot)), this, SLOT(slotCopyIcon(Hotspot)));
	connect(m_hotpoint, SIGNAL(sig_edit_copyTo_tmp_icon(QString)), this, SLOT(slotEditCopyToTmpIcon(QString)));
	connect(this, SIGNAL(sig_Close()), m_hotpoint, SIGNAL(sig_close()));
	connect(m_hotpoint, SIGNAL(sig_set_titleBg_color(QColor)), this, SLOT(slotSetTitleBgColor(QColor)));
	connect(m_hotpoint, SIGNAL(sig_set_titleText_color(QColor)), this, SLOT(slotSetTitleTextColor(QColor)));
	connect(m_hotpoint, SIGNAL(sig_set_titleText_Size(int)), this, SLOT(slotSetTitleTextSize(int)));
	connect(this, SIGNAL(sig_updateSeeAngle(float)), m_hotpoint, SIGNAL(sig_update_see_angle(float)));
	connect(m_hotpoint, SIGNAL(sig_cut_zoom_hp(float)), this, SLOT(slotSetCutZoom(float)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RelaodLevel()), this, SLOT(slotReloadLevel()));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RelaodGroup(QString)), this, SLOT(slotReloadGroup(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_addLevel(int)), this, SLOT(slotAddLevel(int)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_addGroupItem(GroupItemStyle)), this, SLOT(slotAddGroupItemStyle(GroupItemStyle)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_groupName(QString, const QString&)), this, SLOT(slotGroupNameChange(QString, const QString&)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_setGroupItemHide(QString, bool)), this, SLOT(slotSetGroupItemHide(QString, bool)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_GroupAngleWin(int)), this, SLOT(slotShowGroupAngleWin(int)));
	connect(this, SIGNAL(sig_edit_GroupSeeAngleZoom(GroupSeeAngleZoom)), m_hotpoint, SIGNAL(sig_edit_hp_GroupSeeAngleZoom(GroupSeeAngleZoom)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_rotatezoom(GroupSeeAngleZoom)), this, SLOT(slotRotataZoom(GroupSeeAngleZoom)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_SetHPLocation(bool)), this, SLOT(slotSetHPLocation(bool)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_DelTmpGroup(QString)), this, SLOT(slotDelTmpGroup(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_SetSelStyle(QString)), this, SLOT(slotSetSelStyle(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_DelIcon(QString)), this, SLOT(slotDelIcon(QString)));
	connect(this, SIGNAL(sig_clearHpList()), m_hotpoint, SLOT(slotclearHpList()));
	connect(m_hotpoint, SIGNAL(sig_setItemShow_Lock(QString, QString)), this, SLOT(slotSetItemShow_Lock(QString, QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RotateCutHotPoint(QString)), this, SLOT(slotRotateCutHotPoint(QString)));
	connect(m_hotpoint, SIGNAL(sig_style(QString)), this, SLOT(slotSetStyle(QString)));

	ui.stackedWidget_right->addWidget(m_hotpoint);
	ui.stackedWidget_right->setCurrentWidget(m_hotpoint);

	slotSetFirstAngle();
	//模拟点击，添加热点到场景
	{
		for (int i = 0; i < ui.listWidget->count(); ++i)
		{
			QListWidgetItem* listItem = ui.listWidget->item(i);
			QString tmpPic = listItem->data(Qt::UserRole).toString();
			if (tmpPic != m_picPath) continue;
			project_see_item* customItem = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
			if (customItem)
			{
				customItem->slotPicPath();
				break;
			}
		}
	}
}

void project_edit::slotSetFirstAngle()
{
	m_rotation = m_panWidget->getRotation();
	m_zoom = m_panWidget->getZoom();

	WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_picPath];
	tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
	tmpPam.zoom = QString::number(m_zoom, 'f', 1);
	m_panWidget->m_mapPicAngle[m_picPath] = tmpPam;

	QImage img = ui.openGLWidget_center->grabFramebuffer(); 
	QPixmap pixmap = QPixmap::fromImage(img);
	emit sig_Pixmap(pixmap);
}

void project_edit::slotSetSeeAngle(QString angle)
{
	QString strAngle = u8"当前视角：" + angle;
	ui.label_CutAngle->setText(strAngle);
}

void project_edit::slotSetFirstAngle_FrontCover()
{
	//设置场景视角
	slotSetFirstAngle();
	//设置为场景封面
	if (ui.checkBox_blFrontCover->isChecked() != true) return;
	for (int i = 0; i < ui.listWidget->count(); ++i)
	{
		QListWidgetItem* listItem = ui.listWidget->item(i);
		QString tmpPic = listItem->data(Qt::UserRole).toString();
		if (tmpPic != m_picPath) continue;
		project_see_item* customItem = qobject_cast<project_see_item*>(ui.listWidget->itemWidget(listItem));
		if (customItem)
		{
			//生成封面
			QImage img = ui.openGLWidget_center->grabFramebuffer();
			QPixmap pixmap = QPixmap::fromImage(img);
			QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
			QFileInfo fileInfo(m_picPath);
			QString baseName = fileInfo.completeBaseName();
			QString suf = fileInfo.suffix();
			QString thumbPath = dirName + "/Pictures_Thumb_tmp/" + baseName + "." + suf;
			pixmap.save(thumbPath, suf.toUtf8());

			customItem->setFrontCover_thumb();
			break;
		}
	}
}

void project_edit::slotSetSelPicFirstAngle(std::vector<QString>& vecPicID)
{
	for (auto picID : vecPicID)
	{
		QString strRotation = QString::number(m_rotation.x(), 10, 1) + ";" + QString::number(m_rotation.y(), 10, 1) + ";" + QString::number(m_rotation.z(), 10, 1);
		QString strFOV= QString::number(60.0 / m_zoom, 10, 1);

		//数据库
		{
			QString updata = QString("update picture set start_angle = '%1', fov = '%2' ,zoom = '%3' where id = '%4'").arg(strRotation).arg(strFOV).arg(m_zoom).arg(picID);
			QSqlQuery query;
			query.exec(updata);
		}
		//场景
		WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]];
		tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
		tmpPam.zoom = QString::number(m_zoom, 'f', 1);
		m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]] = tmpPam;
	}
	noticeWin.Notice(this, u8"保存完成！", 2000);
}

void project_edit::slotSetSelPicFOV(std::vector<QString>& vecPicID, float zoom, float zoom_min, float zoom_max)
{
	for (auto picID : vecPicID)
	{
		QString zoom_range = QString::number(zoom_min, 10, 1) + ";" + QString::number(zoom_max, 10, 1);
		QString strFOV = QString::number(60.0 / zoom, 10, 1);
		//数据库
		{
			QString updata = QString("update picture set zoom_range = '%1', fov = '%2' where id = '%3'").arg(zoom_range).arg(strFOV).arg(picID);
			QSqlQuery query;
			query.exec(updata);
		}
		//场景
		WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]];
		tmpPam.start_angle = QString::number(m_rotation.x(), 'f', 1) + ";" + QString::number(m_rotation.y(), 'f', 1) + ";" + QString::number(m_rotation.z(), 'f', 1);
		tmpPam.zoom = QString::number(m_zoom, 'f', 1);
		m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]] = tmpPam;
	}
	noticeWin.Notice(this, u8"保存完成！", 2000);
}

void project_edit::slotSetSelPicHV(std::vector<QString>& vecPicID, int H_min, int H_max, int V_min, int V_max)
{
	for (auto picID : vecPicID)
	{
		QString H_range = QString::number(H_min, 10, 1) + ";" + QString::number(H_max, 10, 1);
		QString V_range = QString::number(-V_max, 10, 1) + ";" + QString::number(-V_min, 10, 1);
		//数据库
		{
			QString updata = QString("update picture set h_range = '%1', v_range = '%2' where id = '%3'").arg(H_range).arg(V_range).arg(picID);
			QSqlQuery query;
			query.exec(updata);
		}
		//场景
		WindowParameter tmpPam = m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]];
		tmpPam.h_range = H_range;
		tmpPam.v_range = V_range;
		m_panWidget->m_mapPicAngle[m_mapUUidPicPath[picID]] = tmpPam;
	}
	noticeWin.Notice(this, u8"保存完成！", 2000);
}

void project_edit::slotSetSelPicCompass(std::vector<QString>& vecPicID, float angle, QString path, QString pos)
{
	for (auto picID : vecPicID)
	{
		QString compass_s_angle = QString::number(angle, 10, 1) ;
		//数据库
		{
			QString updata = QString("update picture set compass_s_angle = '%1', compass_pic_path = '%2', compass_location  = '%3' where id = '%4'")
				.arg(compass_s_angle).arg(path).arg(pos).arg(picID);
			QSqlQuery query;
			query.exec(updata);
		}
		//场景
		CompassParameter tmpPam = m_panWidget->m_mapPicCompass[m_mapUUidPicPath[picID]];
		tmpPam.compassAngle = angle;
		tmpPam.picPath = path;
		tmpPam.location = pos;
		m_panWidget->m_mapPicCompass[m_mapUUidPicPath[picID]] = tmpPam;
	}
	noticeWin.Notice(this, u8"保存完成！", 2000);
}

void project_edit::slotSetPicCompassPic(QString picPath)
{
	//场景
	CompassParameter tmpPam = m_panWidget->m_mapPicCompass[m_picPath];
	tmpPam.picPath = picPath;
	m_panWidget->m_mapPicCompass[m_picPath] = tmpPam;
	m_panWidget->setCompassImagePath(picPath);
}

void project_edit::slotSetPicCompassPos(QString pos)
{
	//场景
	CompassParameter tmpPam = m_panWidget->m_mapPicCompass[m_picPath];
	tmpPam.location = pos;
	m_panWidget->m_mapPicCompass[m_picPath] = tmpPam;
	m_panWidget->setCompassLocation(pos);
}

void project_edit::slotSetGroupAngle()
{
	GroupSeeAngleZoom cutANgZ;
	cutANgZ.rotation = m_panWidget->getRotation();
	cutANgZ.zoom = m_panWidget->getZoom();
	QImage img = ui.openGLWidget_center->grabFramebuffer();
	cutANgZ.pixmap = QPixmap::fromImage(img);

	emit sig_edit_GroupSeeAngleZoom(cutANgZ);
}

void project_edit::slotSetCompassN()
{
	//设置指北针
	float currentYaw = m_panWidget->getRotationY(); 
	m_panWidget->setCompassN(currentYaw); 
	m_CAngle = currentYaw;
	m_panWidget->m_mapPicCompass[m_picPath].compassAngle = m_CAngle;
	//
	QImage img = ui.openGLWidget_center->grabFramebuffer();
	QPixmap pixmap = QPixmap::fromImage(img);
	emit sig_Pixmap(pixmap);
	//角度
	emit sig_Angle(currentYaw);
}

void project_edit::slotAddPic()
{
	QStringList tmpPathList = QFileDialog::getOpenFileNames(this, u8"选择全景图片", "", u8"图片 (*.png *.jpg *.jpeg)");
	//删除路径重复的
	std::map<QString, bool> mapPicHave;
	for (auto tmpI : tmpPathList)
	{
		mapPicHave[tmpI] = false;
	}
	for (auto tmpI : tmpPathList)
	{
		for (auto tmpJ : m_listPicPath)
		{
			QString file_name1 = tmpI.section("/", -1).section(".", 0, -2);
			QString file_name2 = tmpJ.section("/", -1).section(".", 0, -2);
			if (file_name1 == file_name2)
			{
				mapPicHave[tmpI] = true;
				break;
			}
		}
	}
	//添加
	m_tmpPathList.clear();
	for (auto tmp : mapPicHave)
	{
		if (tmp.second == false)
		{
			m_tmpPathList.append(tmp.first);
		}
	}

	if (m_tmpPathList.count() == 0) return;
	m_currentItemIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载场景...", u8"取消", 0, m_tmpPathList.size(), this);
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
	m_progressDialog->raise();

	// 使用定时器开始处理
	QTimer::singleShot(0, this, &project_edit::processNextPicItem_Add);
}

void project_edit::slotDelPic()
{
	if (m_delPics)
	{
		delete m_delPics;
	}
	m_delPics = new project_edit_delPics(m_projUUID);
	m_delPics->setTitle(u8"删除场景");
	connect(m_delPics, SIGNAL(sig_delList(std::vector<QString>)), this, SLOT(slotDelPic(std::vector<QString>)));
	connect(m_delPics, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	m_delPics->setWindowModality(Qt::ApplicationModal);
	m_delPics->showNormal();
}

void project_edit::slotReSee()
{
	QStringList listPicPath;
	QString sql = QString("SELECT picture_path from picture where project_name_id = '%1'").arg(m_projUUID);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		listPicPath.append(query.value(0).toString());
	}

	if (m_projSee)
	{
		delete m_projSee;
	}
	m_projSee = new project_edit_resee(m_projUUID, listPicPath, m_mapPicPathUUid);
	m_projSee->setWindowModality(Qt::ApplicationModal);
	m_projSee->showNormal();

}

void project_edit::slotAddIcon(Hotspot hp)
{
	m_panWidget->addHotspot(hp);
}

void project_edit::slotUpdateIcon(QString newPath)
{
	m_panWidget->updateHotspotPath(newPath);
}

void project_edit::slotSetScaleX( float scaleX)
{
	m_panWidget->setScale_X(scaleX);
}

void project_edit::slotSetScaleY( float scaleY)
{
	m_panWidget->setScale_Y(scaleY);
}

void project_edit::slotSetName( QString name)
{
	m_panWidget->setIconName(name);
}

void project_edit::slotNameShow( bool blShow)
{
	m_panWidget->slotNameShow(blShow);
}

void project_edit::slotSetStyle(QString style)
{
	m_panWidget->slotSetStyle(style);
}

void project_edit::slotAddIconEnd()
{
	m_panWidget->slotAddIconEnd();
}

void project_edit::slotEditIconEnd()
{
	m_panWidget->slotEditTmpIconToList();
}

void project_edit::slotDelTmpIcon()
{
	m_panWidget->slotDemTmpIcon();
}

void project_edit::slotCopyIcon(Hotspot hp)
{
	m_panWidget->slotCopyIcon(hp);
}

void project_edit::slotEditCopyToTmpIcon(QString iconID)
{
	m_panWidget->slotEditCopyToTmpIcon(iconID);
}

void project_edit::slotRotateCutHotPoint(QString iconID)
{
	m_panWidget->rotateCutHotPoint(iconID);
}

void project_edit::slotSetTitleBgColor(QColor cl)
{
	m_panWidget->setTitleBgColor(cl);
}

void project_edit::slotSetTitleTextColor(QColor cl)
{
	m_panWidget->setTitleTextColor(cl);
}

void project_edit::slotSetTitleTextSize(int size)
{
	m_panWidget->setTitleTextSize(size);
}

void project_edit::slotSetCutZoom(float zoom)
{
	m_panWidget->setCutZoom(zoom);
}

void project_edit::slotHAngle(int angle)
{
	m_panWidget->setRotateY(angle);
}

void project_edit::slotVAngle(int angle)
{
	m_panWidget->setRotateX(angle);
}

void project_edit::slotReloadLevel()
{
	//重新载入层次
	{
		std::map<QString, std::vector<HotspotLevelShow>> mapVecIconID_Zoom;
		QString data = QString("select d.icon_id, u.* from level u LEFT JOIN levelshow d ON u.level_id = d.level_id where u.pic_id = '%1' ").arg(m_mapPicPathUUid[m_picPath]);
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
	}
	//刷新
	m_panWidget->Update();
}

void project_edit::slotReloadGroup(QString gpID)
{
	//列表删除
	{
		slotDelTmpGroup(gpID);
	}
	//重新载入分组
	{
		std::map<QString, std::vector<HotspotGroupShow>> mapVecIconID_Group;
		QString data = QString("select d.icon_id,u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.pic_id = '%1'").arg(m_mapPicPathUUid[m_picPath]);
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

			mapVecIconID_Group[group.iconID].push_back(group);

		}
		m_panWidget->setIconShowGroups(mapVecIconID_Group);
	}
	//刷新
	m_panWidget->Update();
}
void project_edit::slotAddGroupItemStyle(GroupItemStyle gpStyle)
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();
	int size = ui.listWidget_group->count() + 1;
	int sizeH = size * 46;
	if (size * 46 > scrHeight - 300)
		sizeH = scrHeight - 300;
	ui.listWidget_group->resize(110, sizeH);
	ui.listWidget_group->move(70, (scrHeight - 6 - 140 - sizeH) / 2);

	//添加到List
	QListWidgetItem* item = new QListWidgetItem();
	item->setSizeHint(QSize(90, 36));
	item->setData(Qt::UserRole, gpStyle.groupID);
	ui.listWidget_group->addItem(item);

	project_edit_group_item* item2 = new project_edit_group_item(gpStyle);
	item2->setFocusPolicy(Qt::NoFocus);
	item2->resize(QSize(90, 36));
	ui.listWidget_group->setItemWidget(item, item2);

	m_vecGroupStyle.push_back(gpStyle);

	//显示与隐藏
	if (size > 0 )
		ui.listWidget_group->show();
	else
		ui.listWidget_group->hide();
}

void project_edit::slotAddLevel(int num)
{
	//显示与隐藏
	if (num > 0)
		ui.groupBox_level->show();
	else
		ui.groupBox_level->hide();
}

void project_edit::slotGroupNameChange(QString groupID, const QString& text)
{
	for (size_t i = 0; i < ui.listWidget_group->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget_group->item(i);
		QString tmpID = listItem->data(Qt::UserRole).toString();
		if (tmpID == groupID)
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				item->setText(text);
				break;
			}
		}
	}
}

void project_edit::slotSetGroupItemHide(QString groupID, bool bl)
{
	for (size_t i = 0; i < ui.listWidget_group->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget_group->item(i);
		QString tmpID = listItem->data(Qt::UserRole).toString();
		if (tmpID == groupID)
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				listItem->setHidden(!bl);
				item->setHidden(!bl);
				break;
			}
		}
	}
}

void project_edit::slotShowGroupAngleWin(int bl)
{
	if (bl == 2)
	{
		ui.label_seeAngle->show();
		ui.pushButton_GroupAngle->show();
	}
	else
	{
		ui.label_seeAngle->hide();
		ui.pushButton_GroupAngle->hide();
	}
}

void project_edit::slotRotataZoom(GroupSeeAngleZoom agZoom)
{
	m_panWidget->setRotate(agZoom.rotation.x(), agZoom.rotation.y());
	if (agZoom.isToGroupZoom == true)
		m_panWidget->setCutZoom(agZoom.zoom);
	//刷新
	m_panWidget->Update();
}

void project_edit::slotSetHPLocation(bool bl)
{
	if (bl == true)
		ui.label_hpLocation->show();
	else
		ui.label_hpLocation->hide();
}

void project_edit::slotDelTmpGroup(QString groupID)
{
	//删除
	for (size_t i = 0; i < ui.listWidget_group->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget_group->item(i);
		QString tmpID = listItem->data(Qt::UserRole).toString();
		if (tmpID == groupID)
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				delete item;
				QListWidgetItem* removedItem = ui.listWidget_group->takeItem(i);
				if (removedItem)
					delete removedItem;

				break;
			}
		}
	}
}

void project_edit::slotSetGroupOtherItemStyle(QString groupID)
{
	//设置当前选择分组
	m_panWidget->set_SelGroupID(groupID);
	//
	for (size_t i = 0; i < ui.listWidget_group->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget_group->item(i);
		QString tmpID = listItem->data(Qt::UserRole).toString();
		if (tmpID != groupID)
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				item->slotUnSelHPGroup();
			}
		}
	}
}

void project_edit::slotSetSelStyle(QString groupID)
{
	//设置当前选择分组
	m_panWidget->set_SelGroupID(groupID);
	for (size_t i = 0; i < ui.listWidget_group->count(); i++)
	{
		QListWidgetItem* listItem = ui.listWidget_group->item(i);
		QString tmpID = listItem->data(Qt::UserRole).toString();
		if (tmpID != groupID)
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				item->slotUnSelHPGroup();
			}
		}
		else
		{
			project_edit_group_item* item = qobject_cast<project_edit_group_item*>(ui.listWidget_group->itemWidget(listItem));
			if (item)
			{
				item->slotSelHPGroup();
			}
		}
	}
}

void project_edit::slotDelIcon(QString iconID)
{
	m_panWidget->DelIcon(iconID);
	//刷新
	m_panWidget->Update();
}

void project_edit::slotclearStackedWidget(QWidget* delWgt)
{
	QStackedWidget* stackedWidget = ui.stackedWidget_right; //你的堆叠窗口
	for (size_t i = 0; i < stackedWidget->count(); i++)
	{	
		QWidget* widget = stackedWidget->widget(i);
		if (widget == delWgt)
			stackedWidget->removeWidget(widget);
		//delete widget; 
	}
}

void project_edit::slotSetItemShow_Lock(QString iconID, QString opera)
{
	m_panWidget->setIconShow_Lock(iconID, opera);
	//刷新
	m_panWidget->Update();
}

void project_edit::slotStyle_picText(QString style)
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

void project_edit::slotStyle_picChange(QString style)
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

void project_edit::clearListWidgetGroup()
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

void project_edit::LoadPic_Globle(const QString& picPath)
{
	//
}

void project_edit::LoadPic_SeeAngle(const QString& picPath)
{
	if (m_seeAngle)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_seeAngle;
	}
	m_seeAngle = new project_edit_seeAngle(m_projUUID, m_mapPicPathUUid[picPath], this);
	connect(m_seeAngle, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_seeAngle, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_seeAngle, SIGNAL(sig_save()), this, SLOT(slotSeeAngleSave()));
	connect(m_seeAngle, SIGNAL(sig_setFirstAnglePic(std::vector<QString>&)), this, SLOT(slotSetSelPicFirstAngle(std::vector<QString>&)));
	connect(m_seeAngle, SIGNAL(sig_setPicFOV(std::vector<QString>&, float, float, float)), this, SLOT(slotSetSelPicFOV(std::vector<QString>&, float, float, float)));
	connect(m_seeAngle, SIGNAL(sig_setPicHV(std::vector<QString>&, int, int, int, int)), this, SLOT(slotSetSelPicHV(std::vector<QString>&, int, int, int, int)));

	connect(m_seeAngle, SIGNAL(sig_cut_zoom_angle(float)), this, SLOT(slotSetCutZoom(float)));
	connect(m_seeAngle, SIGNAL(sig_H_angle(int)), this, SLOT(slotHAngle(int)));
	connect(m_seeAngle, SIGNAL(sig_V_angle(int)), this, SLOT(slotVAngle(int)));
	ui.stackedWidget_right->addWidget(m_seeAngle);
	ui.stackedWidget_right->setCurrentWidget(m_seeAngle);
}

void project_edit::LoadPic_Compass(const QString& picPath)
{
	if (m_compass)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_compass;
	}
	m_compass = new project_edit_compass(m_projUUID, m_mapPicPathUUid[m_picPath], this);
	//指北针图片
	QString compassPic = m_panWidget->m_mapPicCompass[m_picPath].picPath;
	m_compass->setCompassPic(compassPic);
	connect(m_compass, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_compass, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_compass, SIGNAL(sig_setPicCompass(std::vector<QString>&, float, QString, QString)), this, SLOT(slotSetSelPicCompass(std::vector<QString>&, float, QString, QString)));
	connect(m_compass, SIGNAL(sig_setCompassPic(QString)), this, SLOT(slotSetPicCompassPic(QString)));
	connect(m_compass, SIGNAL(sig_setCompassPos(QString)), this, SLOT(slotSetPicCompassPos(QString)));
	connect(this, SIGNAL(sig_Angle(float)), m_compass, SLOT(slotSetAngle(float)));

	ui.stackedWidget_right->addWidget(m_compass);
	ui.stackedWidget_right->setCurrentWidget(m_compass);
}

void project_edit::LoadPic_Mask(const QString& picPath)
{
	if (m_mask)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_mask;
	}
	m_mask = new project_edit_mask(m_projUUID, this);
	connect(m_mask, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(this, SIGNAL(sig_Pixmap(QPixmap&)), m_mask, SLOT(slotSetPixmap(QPixmap&)));
	connect(m_mask, SIGNAL(sig_save()), this, SLOT(slotCompassSave()));
	ui.stackedWidget_right->addWidget(m_mask);
	ui.stackedWidget_right->setCurrentWidget(m_mask);
}

void project_edit::LoadPic_HotPoint(const QString& picPath)
{
	//新的场景热点界面
	if (m_hotpoint)
	{
		slotclearStackedWidget(m_hotpoint);//删除以前的
		delete m_hotpoint;
	}
	m_hotpoint = new project_edit_hotpoint(m_projUUID, m_picPath, m_mapPicPathUUid[m_picPath], this);
	connect(m_hotpoint, SIGNAL(sig_Msg(QString)), this, SLOT(slotMsg(QString)));
	connect(m_hotpoint, SIGNAL(sig_hp_set(QString)), this, SLOT(slotHotPoint_Set(QString)));
	connect(m_hotpoint, SIGNAL(sig_addIcon(Hotspot)), this, SLOT(slotAddIcon(Hotspot)));
	connect(m_hotpoint, SIGNAL(sig_updateIcon(QString)), this, SLOT(slotUpdateIcon(QString)));
	connect(m_hotpoint, SIGNAL(sig_scale_x(float)), this, SLOT(slotSetScaleX(float)));
	connect(m_hotpoint, SIGNAL(sig_scale_y(float)), this, SLOT(slotSetScaleY(float)));
	connect(m_hotpoint, SIGNAL(sig_name(QString)), this, SLOT(slotSetName(QString)));
	connect(m_hotpoint, SIGNAL(sig_name_show(bool)), this, SLOT(slotNameShow(bool)));
	connect(m_hotpoint, SIGNAL(sig_add_icon_end()), this, SLOT(slotAddIconEnd()));
	connect(m_hotpoint, SIGNAL(sig_edit_icon_end()), this, SLOT(slotEditIconEnd()));
	connect(m_hotpoint, SIGNAL(sig_del_tmp_icon()), this, SLOT(slotDelTmpIcon()));
	connect(m_hotpoint, SIGNAL(sig_copy_icon(Hotspot)), this, SLOT(slotCopyIcon(Hotspot)));
	connect(m_hotpoint, SIGNAL(sig_edit_copyTo_tmp_icon(QString)), this, SLOT(slotEditCopyToTmpIcon(QString)));
	connect(this, SIGNAL(sig_Close()), m_hotpoint, SIGNAL(sig_close()));
	connect(m_hotpoint, SIGNAL(sig_set_titleBg_color(QColor)), this, SLOT(slotSetTitleBgColor(QColor)));
	connect(m_hotpoint, SIGNAL(sig_set_titleText_color(QColor)), this, SLOT(slotSetTitleTextColor(QColor)));
	connect(m_hotpoint, SIGNAL(sig_set_titleText_Size(int)), this, SLOT(slotSetTitleTextSize(int)));
	connect(this, SIGNAL(sig_updateSeeAngle(float)), m_hotpoint, SIGNAL(sig_update_see_angle(float)));
	connect(m_hotpoint, SIGNAL(sig_cut_zoom_hp(float)), this, SLOT(slotSetCutZoom(float)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RelaodLevel()), this, SLOT(slotReloadLevel()));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RelaodGroup(QString)), this, SLOT(slotReloadGroup(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_addLevel(int)), this, SLOT(slotAddLevel(int)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_addGroupItem(GroupItemStyle)), this, SLOT(slotAddGroupItemStyle(GroupItemStyle)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_groupName(QString, const QString&)), this, SLOT(slotGroupNameChange(QString, const QString&)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_setGroupItemHide(QString, bool)), this, SLOT(slotSetGroupItemHide(QString, bool)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_GroupAngleWin(int)), this, SLOT(slotShowGroupAngleWin(int)));
	connect(this, SIGNAL(sig_edit_GroupSeeAngleZoom(GroupSeeAngleZoom)), m_hotpoint, SIGNAL(sig_edit_hp_GroupSeeAngleZoom(GroupSeeAngleZoom)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_rotatezoom(GroupSeeAngleZoom)), this, SLOT(slotRotataZoom(GroupSeeAngleZoom)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_SetHPLocation(bool)), this, SLOT(slotSetHPLocation(bool)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_DelTmpGroup(QString)), this, SLOT(slotDelTmpGroup(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_SetSelStyle(QString)), this, SLOT(slotSetSelStyle(QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_DelIcon(QString)), this, SLOT(slotDelIcon(QString)));
	connect(this, SIGNAL(sig_clearHpList()), m_hotpoint, SLOT(slotclearHpList()));
	connect(m_hotpoint, SIGNAL(sig_setItemShow_Lock(QString, QString)), this, SLOT(slotSetItemShow_Lock(QString, QString)));
	connect(m_hotpoint, SIGNAL(sig_edit_hp_RotateCutHotPoint(QString)), this, SLOT(slotRotateCutHotPoint(QString)));
	connect(m_hotpoint, SIGNAL(sig_style(QString)), this, SLOT(slotSetStyle(QString)));

	ui.stackedWidget_right->addWidget(m_hotpoint);
	ui.stackedWidget_right->setCurrentWidget(m_hotpoint);
	slotSetFirstAngle();
	//加载热点
	m_vecCurPicHp.clear();
	QString tmpPicID = m_mapPicPathUUid[picPath];
	QString data = QString("select * from hotpoint where picture_id = '%1'").arg(tmpPicID);
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
		m_curInsertNum = 0;
	}
	//载入层次
	{
		std::map<QString, std::vector<HotspotLevelShow>> mapVecIconID_Zoom;
		QString data = QString("select d.icon_id, u.* from level u LEFT JOIN levelshow d ON u.level_id = d.level_id where u.pic_id = '%1' ").arg(tmpPicID);
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

			//存在层次就显示
			ui.groupBox_level->show();
			//是否勾选
			{
				QString data = QString("select isLevelShow from picture where id = '%1' LIMIT 1").arg(tmpPicID);
				QSqlQuery query(data);//查询表的内容
				while (query.next())
				{
					QString islevelShow = query.value(0).toString();
					if (islevelShow == '2')
						ui.checkBox_isLevelShow->setChecked(true);
					else
						ui.checkBox_isLevelShow->setChecked(false);
				}
			}
		}
		m_panWidget->setIconShowZooms(mapVecIconID_Zoom);
	}
	//载入分组
	{
		m_curInsertGroupNum = 0;
		m_mapVecGroup.clear();
		m_vecGroupID.clear();
		//加入场景
		std::map<QString, std::vector<HotspotGroupShow>> mapVecIconID_Group;
		QString data = QString("select d.icon_id,u.* from `group` u LEFT JOIN groupshow d ON u.GroupID = d.GroupID where u.pic_id = '%1'").arg(tmpPicID);
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
		//加入List
		clearListWidgetGroup();
		m_vecGroupStyle.clear();
		QTimer::singleShot(0, this, &project_edit::processNextGroupItem);

	}
	//
	if (m_hotpoint != nullptr)
	{
		m_hotpoint->addIcon2List(m_mapPicPathUUid[m_picPath]);
	}
	//清空当前场景下的热点
	m_panWidget->clearCutPicHp();
	//清空右侧菜单热点
	emit sig_clearHpList();
	//
	QTimer::singleShot(0, this, &project_edit::processNextIconItem);
}
