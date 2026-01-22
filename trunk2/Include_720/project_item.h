#pragma once

#include <QWidget>
#include "ui_project_item.h"
#include "project_see.h"
#include "project_edit.h"
#include "MsgBox.h"

class project_item : public QWidget
{
	Q_OBJECT

public:
	project_item(QString front_cover, QString m_project_uuid,QString project_name, QString line_name, int picture_count,QString create_time, QString update_time,QString classifyType, QWidget *parent = nullptr);
	~project_item();

	QPushButton* getEditButton() const {
		QPushButton* rrr = ui.pushButton_edit;
		return ui.pushButton_edit; 
	}
	QPushButton* getSeeButton() const { return ui.pushButton_see; }
	bool isCheckBoxChecked() const { return ui.checkBox_sel->isChecked();}; // ∑µªÿ «∑Òπ¥—°
	QSize sizeHint() const;

public slots:
	void slotSee();
	void slotEdit();
	void slotDel();
	void slotUpdate();
signals:
	void sigDel(QString);


private:
	QString m_front_cover;
	QString m_project_uuid;
	QString m_project_name;
	QString m_line_name;
	int m_picture_count;
	QString m_create_time;
	QString m_update_time;
	QString m_classifyType;

	project_see* m_projSee;
	project_edit* m_projEdit;

	MsgBox* m_msgBox;
	Ui::project_itemClass ui;
};

