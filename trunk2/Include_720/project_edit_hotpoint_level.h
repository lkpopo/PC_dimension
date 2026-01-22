#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_level.h"
#include "project_edit_hotpoint_level_set.h"
#include <set>


class project_edit_hotpoint_level : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_level(QString projUUID, QString picPath, QString picID, QWidget *parent = nullptr);
	~project_edit_hotpoint_level();

	void addLevel2List(QString);
	void processNextLevelItem();


public slots:
	void slotClose();
	void slotAddLevel();
	void slotEditLevel(QString);
	void slotAddLevelItem(std::vector<HotspotLevelShow>);
	void slotUpdateLevelItem(std::vector<HotspotLevelShow>);
	void slotDelLevelItem(QString levelID);

signals:
	void sig_edit_hp_level_addLevel(int);
	void sig_update_see_angle_level(float);
	void sig_cut_zoom(float);
	void sig_Msg_Set(QString);
	void sig_edit_hp_lv_RelaodLevel();
	void sig_closeSet();

private:
	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	project_edit_hotpoint_level_set* m_levelSet;
	int m_curInsertNum;
	std::map<QString, std::vector<HotspotLevelShow>> m_mapVecLevel;
	std::vector<QString> m_vecLevelID;
	Ui::project_edit_hotpoint_levelClass ui;
};

