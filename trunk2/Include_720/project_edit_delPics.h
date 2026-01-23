#pragma once

#include <QWidget>
#include "ui_project_edit_delPics.h"
#include <QProgressDialog>
#include <vector>

class project_edit_delPics : public QWidget
{
	Q_OBJECT

public:
	project_edit_delPics(QString projUUID, QWidget *parent = nullptr);
	~project_edit_delPics();


	void processNextPicItem();
	void setTitle(QString title);
signals:
	void sig_delList(std::vector<QString>);
	void sig_selList(std::vector<QString>, std::vector<QString>);
	void sig_Msg(QString);
	void sig_sel_setFirstAnglePic(std::vector<QString>&);
	void sig_sel_setFOVPic(std::vector<QString>&);
	void sig_sel_setHVPic(std::vector<QString>&);
	void sig_sel_setAllPic(std::vector<QString>&);
	void sig_sel_setCompassPic(std::vector<QString>&);
public slots:
	void slotClose();
	void slotSelStatus(int);
	void SlotSelList(int);
	void slotOK();

private:
	QProgressDialog* m_progressDialog;
	int m_currentItemIndex;
	QString m_projUUID;
	std::vector<QString> m_vecpicID;
	std::map<QString, QString> m_mapIDPicPath;


	Ui::project_edit_delPicsClass ui;
};

