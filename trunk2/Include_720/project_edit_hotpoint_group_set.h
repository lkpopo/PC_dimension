#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_group_set.h"
#include "PanoramaWidget.h"
#include <wincrypt.h>
class project_edit_hotpoint_group_set : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_group_set(QString projUUID, QString picPath, QString picID, QString groupName, QString groupID, bool isExist,QWidget *parent = nullptr);
	~project_edit_hotpoint_group_set();

	void addIcon2List(QString picID);
	void processNextIconItem();

public slots:
	void slotClose();
	void slotDone();
	void slot_edit_hp_group_set_groupName(const QString&);
	void SlotSelList(int);
	void SlotGroupItemHide(int);
	void SlotGroupSeeAngleZoom(GroupSeeAngleZoom);
	void SlotHPComboBoxShow(int);
	void Slot_GroupAngleWin(int);
	void slotSelStatus(int);
	void slotSearch(const QString&);
	void slotUpdateHotpointList(QVector<Hotspot>& filterHpList);
	void slotClearSearchLine();
	void slotclearHpList();
signals:
	void sig_Msg_Set_lvSet(QString);
	void sig_edit_hp_group_set_groupName(QString , const QString&);
	void sig_edit_hp_group_set_setGroupItemHide(QString, bool);
	void sig_edit_hp_group_set_GroupAngleWin(int);
	void sig_edit_hp_group_set_GroupAngleWin(QString, bool);
	void sig_AddGroup(std::vector<HotspotGroupShow>);
	void sig_UpdateGroup(std::vector<HotspotGroupShow>);
	void sig_edit_hp_group_set_DelTmpGroup(QString);
	void sig_ReturnDefSel();
	void sig_edit_hp_group_set_SetSelStyle(QString);
	void sig_UpdateHotpointList(QVector<Hotspot>&);

private:

	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	QString m_groupName;
	int m_curInsertNum;
	std::vector<Hotspot> m_vecCurPicHp;
	QString m_groupID;//分组id
	bool m_isExist = false;//是否是以前存在的
	std::vector<QString> m_vecGroupIcon;
	bool m_isIncludeAll;
	GroupSeeAngleZoom m_gsAngZm;

	Ui::project_edit_hotpoint_group_setClass ui;
};

