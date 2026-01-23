#pragma once

#include <QWidget>
#include "NoticeWidget.h"
#include "ui_project_edit_compass.h"
#include "project_edit_delPics.h"

class PanoramaWidget; //前置声明，避免循环包含头文件。前置声明足够（因为只用指针）
class project_edit_compass : public QWidget
{
	Q_OBJECT

public:
	project_edit_compass(QString projUUID, QString picID, QWidget *parent = nullptr);
	~project_edit_compass();
	void setCompassPic(QString compPic);

	NoticeWidget noticeWin;

public slots:
	void slotSetAngle(float);
	void slotSetPixmap(QPixmap& pix);
	void slotSetPosition();
	void slotSetCompassStyle();
	void slotSetCampass();
	void slotSetPicCompass(std::vector<QString>&);
signals:
	void sig_save();
	void sig_setPicCompass(std::vector<QString>&, float, QString, QString);
	void sig_setCompassPic(QString);
	void sig_setCompassPos(QString);
private:
	QString m_projUUID;
	QString m_picID;

	project_edit_delPics* m_delPics;

	float m_CAngle;//角度
	QString selectedFile;     // 指北针图片路径
	QString compassPosition;  // 指北针位置

	Ui::project_edit_compassClass ui;
};
