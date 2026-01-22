#pragma once

#include <QWidget>
#include "ui_project_see_item.h"
#include "MarqueeLabel.h"


class project_see_item : public QWidget
{
	Q_OBJECT

public:
	project_see_item(QString picPath, QWidget *parent = nullptr);
	~project_see_item();

	void setFrontCover();
	void setFrontCover_thumb();
	void setNameHide() 
	{ 
		ui.pushButton_baseName->hide(); 
		ui.label_baseName->hide();
	};

public slots:
	void slotPicPath();
signals:
	void sig_picpath(QString);


public:
	QString m_picPath;
private:

	Ui::project_see_itemClass ui;
};

