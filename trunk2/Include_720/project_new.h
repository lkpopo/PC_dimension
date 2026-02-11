#pragma once

#include <QWidget>
#include "ui_project_new.h"
#include <QMouseEvent>
#include "project_new_picture.h"
#include <QListWidgetItem>
#include <QTimer>
#include <QProgressDialog>
#include <NoticeWidget.h>
#include <MsgBox.h>
#include "project_new_todo.h"

class project_new : public QWidget
{
	Q_OBJECT

public:
	project_new(QWidget *parent = nullptr);
	~project_new();
public slots:
	void slotClose();

	void slotAddPic();
	void slotClear();
	void slotDelPic(QString picPath);
	void slotCreate();
	void slotSelChanged(int index);
	void slotOpenPicture(QListWidgetItem* item);
	void slotEditTitleCount();
	void slotEditIntCount();
	void slotEditKeyCount();
	void slotDraw();

signals:
	void sig_Update();
	void sig_Edit(QString);
	void sig_See(QString);
	void sig_New();



public:
	
	void initializePicItem(QListWidgetItem* item, project_new_picture* item2,const QString& picPath);
	void processNextPicItem();
	void processInsertPicItem();

private:
	QProgressDialog* m_progressDialog;
	MsgBox* m_msgBox;
	NoticeWidget noticeWin;
	project_new_todo* m_todoWin;

	QString m_Project_UUID;
	QString m_draw;

	int m_currentItemIndex;
	QStringList m_tmpPathList;
	QStringList m_ListWidgetPathList;
private:
	Ui::project_newClass ui;
};

