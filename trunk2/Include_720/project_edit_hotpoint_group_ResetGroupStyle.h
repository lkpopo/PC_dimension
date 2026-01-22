#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_group_ResetGroupStyle.h"

class project_edit_hotpoint_group_ResetGroupStyle : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_group_ResetGroupStyle(QString projUUID, QString picID, QWidget *parent = nullptr);
	~project_edit_hotpoint_group_ResetGroupStyle();

public slots:
	void slot_unsel_bg_color();
	void slot_unsel_text_color();
	void slot_sel_bg_color();
	void slot_sel_text_color();
	void slotDone();

private:
	QString m_projUUID;
	QString m_picID;

	QColor m_group_unsel_bg_color;
	QColor m_group_unsel_text_color;
	QColor m_group_sel_bg_color;
	QColor m_group_sel_text_color;

	Ui::project_edit_hotpoint_group_ResetGroupStyleClass ui;
};

