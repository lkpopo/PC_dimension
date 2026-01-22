#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_level_set.h"
#include "PanoramaWidget.h"
class project_edit_hotpoint_level_set : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_level_set(QString projUUID, QString picPath, QString picID, QString levelID = "", QWidget * parent = nullptr);
	~project_edit_hotpoint_level_set();

	void addIcon2List(QString picID);
	void processNextIconItem();

signals:
	void sig_cutZoom(float);
	void sig_Msg_Set_lvSet(QString);
	void sig_AddLevel(std::vector<HotspotLevelShow>);
	void sig_UpdateLevel(std::vector<HotspotLevelShow>);
	void sig_edit_hp_lv_set_RelaodLevel();
	void sig_UpdateHotpointList(QVector<Hotspot>&);



public slots:
	void slotClose();
	void slotDone();
	void slotclearHpList();
	void slotSlideValueChanged(HandlePos pos, double value);
	void slotUpdateTopButton(float value);
	void slotSearch(const QString&);
	void slotUpdateHotpointList(QVector<Hotspot>& filterHpList);
	void slotClearSearchLine();

private:
	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	int m_curInsertNum;
	std::vector<Hotspot> m_vecCurPicHp;
	QString m_levelID;//层级id
	bool m_isExist;//是否是以前存在的
	std::vector<QString> m_vecLevelIcon;

	Ui::project_edit_hotpoint_level_setClass ui;
};

