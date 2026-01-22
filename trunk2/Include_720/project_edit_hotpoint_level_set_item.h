#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_level_set_item.h"
#include "PanoramaWidget.h"
#include "project_edit_hotpoint_level_set.h"


class project_edit_hotpoint_level_set_item : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_level_set_item(Hotspot& hp, QWidget *parent = nullptr);
	~project_edit_hotpoint_level_set_item();

	//Ñ¡Ôñ×´Ì¬
	void setCheckStatus(bool bl) { ui.checkBox->setChecked(bl); };
	bool getCheckStatus() { return ui.checkBox->checkState(); };


public slots:
	void slotSelOperation();

signals:
	void sig_Msg_Set(QString);
	void sig_SelStatus(int);
	void sig_edit_hp_batch_item_RotateCutHotPoint(QString);

private:
	project_edit_hotpoint_level_set* m_levelSet;

	QString m_iconUUID;
	QString m_iconPath;

	Ui::project_edit_hotpoint_level_set_itemClass ui;
};

