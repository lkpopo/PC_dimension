#pragma once

#include <QWidget>
#include "ui_project_new_todo.h"

class project_new_todo : public QWidget
{
	Q_OBJECT

public:
	project_new_todo(QString Project_UUID, QWidget *parent = nullptr);
	~project_new_todo();


public slots:
	void slotEdit();
	void slotSee();
	void slotNew();


signals:
	void sig_edit(QString);
	void sig_see(QString);
	void sig_new();




private:

	QString m_project_UUID;


	Ui::project_new_todoClass ui;
};

