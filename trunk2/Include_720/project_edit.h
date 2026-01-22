#pragma once

#include <QWidget>
#include "ui_project_edit.h"
#include "MsgBox.h"
#include "NoticeWidget.h"
#include "project_new.h"
#include <QProgressDialog>
#include <QSqlQuery>

#include "project_edit_globle.h"
#include "project_edit_compass.h"
#include "project_edit_hotpoint.h"
#include "project_edit_mask.h"
#include "project_edit_seeAngle.h"
#include "project_see.h"
#include "project_edit_hotpoint_set.h"
#include "project_edit_resee.h"
#include "project_edit_delPics.h"

class project_edit : public QWidget
{
	Q_OBJECT

public:
	project_edit(QString projUUID, QString project_name, QStringList listPicPath, QWidget *parent = nullptr);
	~project_edit();

	void processNextPicItem();
	void processNextPicItem_Add();
	void initSheet();
	void processNextIconItem();
	void processNextGroupItem();
	void clearListWidgetGroup();

	void LoadPic_Globle(const QString&);
	void LoadPic_SeeAngle(const QString&);
	void LoadPic_Compass(const QString&);
	void LoadPic_Mask(const QString&);
	void LoadPic_HotPoint(const QString&);



public slots:
	void slotClose();
	void slotDelPic(std::vector<QString>);
	void SlotSetLevelShow(int);
	void slotMsg(QString);
	void slotSeeAngleSave();
	void slotCompassSave();
	void slotNewPicClick(const QString& picPath);

	void slotGloble();
	void slotSeeAngle();
	void slotCompass();
	void slotMask();
	void slotHotPoint();
	void slotSetFirstAngle();
	void slotSetSeeAngle(QString);
	void slotSetFirstAngle_FrontCover();
	void slotSetGroupAngle();
	void slotSetCompassN();

	void slotAddPic();
	void slotDelPic();
	void slotReSee();

	void slotAddIcon(Hotspot);//添加热点
	void slotUpdateIcon(QString);//更新热点
	void slotSetScaleX(float);//设置缩放X
	void slotSetScaleY(float);//设置缩放Y
	void slotSetName(QString);//设置图标名称
	void slotNameShow(bool);//是否显示标题
	void slotAddIconEnd();
	void slotEditIconEnd();
	void slotDelTmpIcon();
	void slotCopyIcon(Hotspot);
	void slotEditCopyToTmpIcon(QString);
	void slotRotateCutHotPoint(QString);
	void slotSetTitleBgColor(QColor);
	void slotSetTitleTextColor(QColor);
	void slotSetTitleTextSize(int);
	void slotSetCutZoom(float);
	void slotReloadLevel();
	void slotReloadGroup(QString);
	void slotAddGroupItemStyle(GroupItemStyle);
	void slotAddLevel(int num);
	void slotGroupNameChange(QString, const QString&);
	void slotSetGroupItemHide(QString, bool);
	void slotShowGroupAngleWin(int);
	void slotRotataZoom(GroupSeeAngleZoom);
	void slotSetHPLocation(bool bl);
	void slotDelTmpGroup(QString);
	void slotSetGroupOtherItemStyle(QString);
	void slotSetSelStyle(QString);
	void slotDelIcon(QString);
	void slotclearStackedWidget(QWidget* delWgt);
	void slotSetItemShow_Lock(QString, QString);

signals:
	void sig_updateSeeAngle(float);
	void sig_Pixmap(QPixmap&);
	void sig_update();
	void sig_Close();
	void sig_edit_GroupSeeAngleZoom(GroupSeeAngleZoom);
	void sig_clearHpList();
	void sig_roration_zoom(QVector3D, float);


private:
	QString m_projUUID;
	QString m_project_name;
	std::map<QString, QString> m_mapUUidPicPath;
	std::map<QString, QString> m_mapPicPathUUid;
	QString m_picPath;

	PanoramaWidget* m_panWidget;
	MsgBox* m_msgBox;
	NoticeWidget noticeWin;

	int m_currentItemIndex;

	project_edit_globle* m_globle;
	project_edit_seeAngle* m_seeAngle;
	project_edit_compass* m_compass;
	project_edit_mask* m_mask;
	project_edit_hotpoint* m_hotpoint;
	project_edit_delPics* m_delPics;
	QVector3D m_rotation = QVector3D(-9999,-9999,-9999);
	float m_CAngle = -9999.0;
	float m_zoom = -9999.0;
	std::vector<Hotspot> m_vecCurPicHp;
	int m_curInsertNum;
	
	QString m_cutPage;//当前界面的编辑功能
	project_edit_resee* m_projSee;

	int m_curInsertGroupNum;

	std::map<QString, std::vector<HotspotGroupShow>> m_mapVecGroup;//当前图片的组信息
	std::vector<QString> m_vecGroupID;//当前图片的组ID
	std::vector<GroupItemStyle> m_vecGroupStyle;//当前图片的组的样式，现在每个组都是一样

	//添加场景
	QProgressDialog* m_progressDialog;
	QStringList m_tmpPathList;
	QStringList m_listPicPath;

	Ui::project_editClass ui;
};

