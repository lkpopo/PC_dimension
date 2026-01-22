#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_group_item.h"
#include "PanoramaWidget.h"


class project_edit_hotpoint_group_item : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_group_item(std::vector<HotspotGroupShow> lvShow, QWidget *parent = nullptr);
	~project_edit_hotpoint_group_item();

	void updateItem(std::vector<HotspotGroupShow> vecGroupShow);
	bool getDefOpen() { return m_blDefOpen; };
	void setDefOpen(bool bl) { m_blDefOpen = bl; };
signals:
	void sig_EditGroup(QString);
	void sig_edit_hp_group_item_rotatezoom(GroupSeeAngleZoom);
	void sig_edit_hp_group_set_SetSelStyle(QString);
	void sig_edit_hp_group_item_GroupAngleWin(int);
	void sig_edit_hp_gp_item_RelaodGroup(QString);
	void sig_del_group_list(QString,bool);
	void sig_setFirstOpen(QString);

public slots:
	void slotGroupEdit();
	void slotDeflOpen();//Ä¬ÈÏ´ò¿ª
	void slotDelGroup();
	void slotDefGroup();
private:
	QString m_groupID;
	bool m_isPerGroupSeeAngle;
	float m_group_rotate_x;
	float m_group_rotate_y;
	float m_gruop_zoom;
	bool m_isToGroupZoom;

	bool m_blDefOpen = false;

	Ui::project_edit_hotpoint_group_itemClass ui;
};

