#pragma once

#include <QWidget>
#include "ui_project_edit_globle.h"
#include "project_edit_globle_frontCover.h"
#include "NoticeWidget.h"


class project_edit_globle : public QWidget
{
	Q_OBJECT

public:
	project_edit_globle(QString projUUID, QString cutPicPath,QWidget *parent = nullptr);
	void updateCharCount(QTextEdit* edit, QLabel* label, int maxChars);
	~project_edit_globle();

public slots:
	void slotClose();
	void slotSetFrontCover();
	void slotDone();
	void slotSelPic();

	void slotReSeeFrontCover();

signals:
	void sig_Msg(QString);


private:

	project_edit_globle_frontCover* m_frontCover;
	QString m_projUUID;
	QString m_cutPicPath;
	NoticeWidget noticeWin;


	Ui::project_edit_globleClass ui;
};

