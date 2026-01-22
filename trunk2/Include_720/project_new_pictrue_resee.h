#pragma once

#include <QWidget>
#include "ui_project_new_pictrue_resee.h"
#include "PanoramaWidget.h"



class project_new_pictrue_resee : public QWidget
{
	Q_OBJECT

public:
	project_new_pictrue_resee(QString picPath, QWidget *parent = nullptr);
	~project_new_pictrue_resee();

public slots:
	void slotClose();


private:

	QString m_picPath;
	PanoramaWidget* m_panoramaWidget;

	Ui::project_new_pictrue_reseeClass ui;
};

