#pragma once

#include <QWidget>
#include "ui_project_edit_ruler.h"
#include "project_edit_ruler_set.h"


class project_edit_ruler : public QWidget
{
	Q_OBJECT

public:
	project_edit_ruler(QString projUUID, QString picPath, QString picID, QWidget *parent = nullptr);
	~project_edit_ruler();

	void processNextRulerItem();
	void clearListWidget();
	void search(QString);


public slots:
	void slotAddRuler();


private:

	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;
	project_edit_ruler_set* m_rulerSet;

	int m_curInsertNum;
	//std::vector<Ruler> m_vecCurPicRuler;

	Ui::project_edit_rulerClass ui;
};

