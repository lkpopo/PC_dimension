#pragma once

#include <QWidget>
#include "ui_project_edit_ruler_set.h"
#include "PanoramaWidget.h"


class project_edit_ruler_set : public QWidget
{
	Q_OBJECT

public:
	project_edit_ruler_set(QString projUUID, QString picID, QString rulerID, QWidget *parent = nullptr);
	~project_edit_ruler_set();

private:

	QString m_projUUID;
	QString m_picID;

	QString m_rulerID;

	bool m_isExist;//是否是以前存在的
	Ruler m_oldRulerPam;//传入的旧参数

	int m_currentItemIndex;

	Ui::project_edit_ruler_setClass ui;
};

