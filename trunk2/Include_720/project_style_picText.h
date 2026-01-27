#pragma once

#include <QWidget>
#include "ui_project_style_picText.h"
#include <QProgressDialog>


class project_style_picText : public QWidget
{
	Q_OBJECT

public:
	project_style_picText(QStringList picList, QString title, QWidget *parent = nullptr);
	~project_style_picText();

	void processNextPicItem();


public slots:
	void slotOpenPicture(QListWidgetItem* item);


private:
	QStringList m_picList;
	QString m_title;

	int m_currentItemIndex;
	QProgressDialog* m_progressDialog;

	Ui::project_style_picTextClass ui;
};

