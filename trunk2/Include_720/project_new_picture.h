#pragma once

#include <QWidget>
#include "ui_project_new_picture.h"

#include "project_new_pictrue_resee.h"


class project_new_picture : public QWidget
{
	Q_OBJECT

public:
	project_new_picture(QString picPath,QWidget *parent = nullptr);
	~project_new_picture();


public slots:
	void slotEdit();
	void slotReSee();
	void slotDel();

	void slotEndEdit();

signals:
	void sigPic(QString);

private:

	QString m_picPath;

	project_new_pictrue_resee* m_reseeItem;

	Ui::project_new_pictureClass ui;
};

