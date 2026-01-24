#pragma once

#include <QWidget>
#include "ui_project_edit_delPics_item.h"

class project_edit_delPics_item : public QWidget
{
	Q_OBJECT

public:
	project_edit_delPics_item(QString projID, QString picID, QString picPath,QWidget *parent = nullptr);
	~project_edit_delPics_item();

	//Ñ¡Ôñ×´Ì¬
	void setCheckStatus(bool bl) { ui.checkBox->setChecked(bl); };
	bool getCheckStatus() { return ui.checkBox->checkState(); };

signals:
	void sig_SelStatus(int);

public slots:
	void slotSelOperation();

private:
	QString m_projID;
	QString m_picID;
	QString m_picPath;


	Ui::project_edit_delPics_itemClass ui;
};

