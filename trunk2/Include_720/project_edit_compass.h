#pragma once

#include <QWidget>
#include "NoticeWidget.h"
#include "ui_project_edit_compass.h"

class PanoramaWidget; //前置声明，避免循环包含头文件。前置声明足够（因为只用指针）
class project_edit_compass : public QWidget
{
	Q_OBJECT

public:
	project_edit_compass(QString projUUID, QWidget *parent = nullptr);
	~project_edit_compass();
	void setPanoramaWidget(PanoramaWidget* widget); // 设置全景指针
	void setCompassPic(QString compPic);

	NoticeWidget noticeWin;

public slots:
	void slotSetPixmap(QPixmap& pix);
	void slotDone();
	void slotSetPosition();
	void slotSetCompassStyle();

signals:
	void sig_save();

private:

	QString m_projUUID;
	float m_CAngle;
	PanoramaWidget* m_panWidget = nullptr;
	QString selectedFile;     // 指北针图片路径
	QString compassPosition;  // 指北针位置
	Ui::project_edit_compassClass ui;
};
