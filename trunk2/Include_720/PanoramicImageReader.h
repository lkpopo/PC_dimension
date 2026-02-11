#pragma once

#include <QtWidgets/QWidget>
#include "ui_PanoramicImageReader.h"
#include "MsgBox.h"
#include "NoticeWidget.h"
#include "project_new.h"
#include <QProgressDialog>
#include <QSqlQuery>


class PanoramicImageReader : public QWidget
{
    Q_OBJECT

public:
    PanoramicImageReader(QWidget *parent = nullptr);
    ~PanoramicImageReader();


public:
    void initWindows();
    void UpdateList();
    void initListWidget();
    void initListWidgetPure();
    void create_linename_sqlite();
    void create_project_sqlite();
    void create_picture_sqlite();
    void create_hotpoint_sqlite();
    void create_ruler_sqlite();
    void create_level_sqlite();
    void create_group_sqlite();
    void create_levelshow_sqlite();
    void create_groupshow_sqlite();
    void create_mask_sqlite();
public slots:
    void slotClose();
    void slotEdit(QString);
    void slotSee(QString);
    void slotNew();
    void slotBatchDel();
	void slotSort();

    void slotUpdateProjList();
    void slotDelProj(QString projUUID);


private:

	MsgBox* m_msgBox;  //批量删除确认框
    NoticeWidget noticeWin;
    QProgressDialog* m_progressDialog;

    QSqlQuery m_allQuery;//全部项目
    int m_currentProjIndex;
    int m_ProjCount;
private:
    Ui::PanoramicImageReaderClass ui;
};

