#pragma once

#include <QWidget>
#include "ui_project_edit_seeAngle.h"
#include "project_edit_delPics.h"
#include <QVector3D>

class project_edit_seeAngle : public QWidget
{
	Q_OBJECT

public:
	project_edit_seeAngle(QString projUUID, QWidget *parent = nullptr);
	~project_edit_seeAngle();

public slots:
	void slotSetPixmap(QPixmap& pix);
	void slotDone();
	void slotSetOrgSeeAngle();
	void slotSelPic(std::vector<QString>, std::vector<QString>);
	void slotSetRoration_Zoom(QVector3D, float);

signals:
	void sig_save();

private:
	project_edit_delPics* m_delPics;
	QString m_projUUID;
	
	QVector3D m_rotation = QVector3D(-9999, -9999, -9999);//旋转
	float m_zoom = -9999.0;//缩放，与初始视角相关




	Ui::project_edit_seeAngleClass ui;
};

