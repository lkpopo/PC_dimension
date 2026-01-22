#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_group.h"
#include "project_edit_hotpoint_group_set.h"
#include "project_edit_hotpoint_group_ResetGroupStyle.h"

class project_edit_hotpoint_group : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_group(QString projUUID, QString picPath, QString picID, QWidget *parent = nullptr);
	~project_edit_hotpoint_group();

	void addGroup2List(QString);
	void processNextGroupItem();

public slots:
	void slotClose();
	void slotAddGroup();
	void slotGroupStyle();
	void slotEditGroup(QString);
	void SlotAddGroupItem(std::vector<HotspotGroupShow>);
	void SlotUpdateGroupItem(std::vector<HotspotGroupShow>);
	void slotDelGroupItem(QString, bool);
	void slotSetFirstOpen(QString);
	void SlotReturnDefSel();
signals:
	void sig_edit_hp_group_addGroupItem(GroupItemStyle);
	void sig_edit_hp_group_groupName(QString, const QString&);
	void sig_edit_hp_group_setGroupItemHide(QString, bool);
	void sig_edit_hp_group_GroupAngleWin(int);
	void sig_edit_hp_group_GroupSeeAngleZoom(GroupSeeAngleZoom);
	void sig_Msg_Set(QString);
	void sig_edit_hp_group_rotatezoom(GroupSeeAngleZoom);
	void sig_edit_hp_group_DelTmpGroup(QString);
	void sig_edit_hp_group_SetSelStyle(QString);
	void sig_edit_hp_gp_RelaodGroup(QString);
	void sig_closeSet();


private:
	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	project_edit_hotpoint_group_set* m_groupSet;
	project_edit_hotpoint_group_ResetGroupStyle* m_style;
	int m_curInsertNum;
	std::map<QString, std::vector<HotspotGroupShow>> m_mapVecGroup;
	std::vector<QString> m_vecGroupID;
	QString m_DefopenGroupID = "";//默认打开的分组

	Ui::project_edit_hotpoint_groupClass ui;
};

