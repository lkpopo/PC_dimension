#pragma once

#include <QWidget>
#include "ui_project_edit_group_item.h"
#include "PanoramaWidget.h"

class project_edit_group_item : public QWidget
{
	Q_OBJECT

public:
	project_edit_group_item(GroupItemStyle gpStyle,QWidget *parent = nullptr);
	~project_edit_group_item();

	void setText(QString);
	void clickEvent(); //µã»÷
	void clickButton();




public slots:
	void slotHPGroup();
	void slotSelHPGroup();
	void slotUnSelHPGroup();
	void setSelStatus(bool);

signals:
	void itemClicked(QString);
	void sigDisconnect();
	void sigConnect();



private:
	GroupItemStyle m_itemStyle;
	bool m_isSelStatus = false;

	Ui::project_edit_group_itemClass ui;
};

