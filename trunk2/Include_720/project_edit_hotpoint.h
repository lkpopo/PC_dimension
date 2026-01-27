#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint.h"
#include "project_edit_hotpoint_set.h"
#include "project_edit_hotpoint_item.h"

#include "project_edit_hotpoint_level.h"
#include "project_edit_hotpoint_group.h"
#include "project_edit_hotpoint_batch.h"


class project_edit_hotpoint : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint(QString projUUID, QString picPath, QString picID, QWidget *parent = nullptr);
	~project_edit_hotpoint();

	
	void processNextIconItem();
	void clearListWidget();
	void search(QString);



public slots:
	void slotAddHotPoint();
	void slot_add_icon_list(Hotspot);
	void slot_update_icon(Hotspot);
	void slot_insert_new(QString, Hotspot);
	void slot_edit_icon(QString);

	void slotAddLevel();
	void slotAddGroup();
	void slotPiSetPic();
	void slot_Operation(QString, QString); //
	void slotDelIcon(QString);
	void slotclearHpList();
	void slotTest(QString);
	void addIcon2List(QString picID);
	void slotSearch(const QString&);
	void slotUpdateHotpointList(QVector<Hotspot>& filterHpList);
	void slotClearSearchLine();


signals:
	void sig_addIcon(Hotspot);
	void sig_updateIcon(QString);
	void sig_Msg(QString);
	void sig_scale_x(float);
	void sig_scale_y(float);
	void sig_name(QString);
	void sig_name_show(bool);
	void sig_add_icon_end();
	void sig_edit_icon_end();
	void sig_del_tmp_icon();
	void sig_edit_copyTo_tmp_icon(QString);
	void sig_copy_icon(Hotspot);
	void sig_close();
	void sig_set_titleBg_color(QColor);
	void sig_set_titleText_color(QColor);
	void sig_set_titleText_Size(int);
	void sig_update_see_angle(float);
	void sig_cut_zoom_hp(float);
	void sig_edit_hp_RelaodLevel(); 
	void sig_edit_hp_addLevel(int);
	void sig_edit_hp_addGroupItem(GroupItemStyle);
	void sig_edit_hp_groupName(QString, const QString&);
	void sig_edit_hp_setGroupItemHide(QString, bool);
	void sig_edit_hp_GroupAngleWin(int);
	void sig_edit_hp_GroupSeeAngleZoom(GroupSeeAngleZoom);
	void sig_edit_hp_rotatezoom(GroupSeeAngleZoom);
	void sig_edit_hp_SetHPLocation(bool);
	void sig_edit_hp_DelTmpGroup(QString);
	void sig_edit_hp_SetSelStyle(QString);
	void sig_edit_hp_DelIcon(QString);
	void sig_setMenuItemText(QString, QString, QString);
	void sig_setItemShow_Lock(QString, QString);
	void sig_edit_hp_RotateCutHotPoint(QString);

	void sig_Test(QString);

	void sig_UpdateHotpointList(QVector<Hotspot>&);
	void sig_style(QString);
private:

	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	project_edit_hotpoint_set* m_hpSet;
	project_edit_hotpoint_level* m_level;
	project_edit_hotpoint_group* m_group;
	project_edit_hotpoint_batch* m_batch;

	int m_curInsertNum;
	std::vector<Hotspot> m_vecCurPicHp;
	Ui::project_edit_hotpointClass ui;
};

