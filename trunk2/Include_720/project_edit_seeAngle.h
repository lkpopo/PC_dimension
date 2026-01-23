#pragma once

#include <QWidget>
#include "ui_project_edit_seeAngle.h"
#include "project_edit_delPics.h"
#include <QVector3D>
#include "MultiPosSlider_FOV.h"
#include "MultiPosSlider_H.h"
#include "MultiPosSlider_V.h"

class project_edit_seeAngle : public QWidget
{
	Q_OBJECT

public:
	project_edit_seeAngle(QString projUUID, QString picID, QWidget *parent = nullptr);
	~project_edit_seeAngle();

public slots:
	void slotSetPixmap(QPixmap& pix);
	void slotDone();
	
	void slotSlideValueChanged_FOV(HandlePos_FOV pos, double value);
	void slotSlideValueChanged_H(HandlePos_H pos, int value);
	void slotSlideValueChanged_V(HandlePos_V pos, int value);
	
	void slotSetOrgSeeAngle();
	void slotSetFOVAngle();
	void slotSetHVAngle();
	void slotSetAllAngle();

	void slotSetPicFOV(std::vector<QString>&);
	void slotSetPicHV(std::vector<QString>&);
	void slotSetPicAll(std::vector<QString>&);

signals:
	void sig_save();
	void sig_setFirstAnglePic(std::vector<QString>&);

	void sig_setPicFOV(std::vector<QString>&, float, float, float);
	void sig_setPicHV(std::vector<QString>&, int, int, int, int);

	void sig_cut_zoom_angle(float);
	void sig_H_angle(int);
	void sig_V_angle(int);

private:
	project_edit_delPics* m_delPics;
	QString m_projUUID;
	QString m_picID;

	QVector3D m_rotation = QVector3D(-9999, -9999, -9999);//旋转
	float m_zoom = -9999.0;//缩放，与初始视角相关




	Ui::project_edit_seeAngleClass ui;
};

