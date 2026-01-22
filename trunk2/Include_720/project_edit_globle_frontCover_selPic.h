#pragma once

#include <QWidget>
#include "ui_project_edit_globle_frontCover_selPic.h"
#include "project_see_item.h"


class project_edit_globle_frontCover_selPic : public QWidget
{
	Q_OBJECT

public:
	project_edit_globle_frontCover_selPic(QStringList listPicPath,QWidget *parent = nullptr);
	~project_edit_globle_frontCover_selPic();

	void processNextPicItem();
public slots:
	void slotClose();
	void slotOK();
	void slotNewPic(QString picPath);

signals:
	void sig_pic(QString);

private:
	QStringList m_listPicPath;
	QString m_picPath;
	int m_currentItemIndex;
	Ui::project_edit_globle_frontCover_selPicClass ui;
};

