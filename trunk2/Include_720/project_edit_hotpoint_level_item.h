#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_level_item.h"
#include "PanoramaWidget.h"

class project_edit_hotpoint_level_item : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_level_item(std::vector<HotspotLevelShow>, QWidget *parent = nullptr);
	~project_edit_hotpoint_level_item();

	void updateItem(std::vector<HotspotLevelShow> vecLevelShow);
signals:
	void sig_EditLevel(QString);
	void sig_del_level_list(QString);
	void sig_edit_hp_lv_item_RelaodLevel();
public slots:
	void slotLevelEdit();
	void slotChangePic();
	void slotDelLevel();

private:

	std::vector<HotspotLevelShow> m_vecLevel;

	QString m_levelID;
	float m_min_zoom;
	float m_max_zoom;
	int m_IconMum;



	Ui::project_edit_hotpoint_level_itemClass ui;
};

