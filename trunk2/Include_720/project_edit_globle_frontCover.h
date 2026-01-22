#pragma once

#include <QWidget>
#include "ui_project_edit_globle_frontCover.h"
#include "NoticeWidget.h"
#include "PanoramaWidget.h"
#include "project_edit_globle_frontCover_selPic.h"

class project_edit_globle_frontCover : public QWidget
{
	Q_OBJECT

public:
	project_edit_globle_frontCover(QString projUUID, QStringList listPicPath, QWidget *parent = nullptr);
	~project_edit_globle_frontCover();

public slots:
	void slotClose();
	void slotSelPic();
	void slotScreenshort();
	void slotLocalUp();
	void slotOK();
	void slotChangePic(QString);
signals:
	void sig_change();

private:
	NoticeWidget noticeWin;
	PanoramaWidget* m_panWidget;
	QString m_projUUID;
	QStringList m_listPicPath;
	std::map<QString, QString> m_mapUUidPicPath;
	std::map<QString, QString> m_mapPicPathUUid;
	project_edit_globle_frontCover_selPic* m_selPic;

	Ui::project_edit_globle_frontCoverClass ui;
};

