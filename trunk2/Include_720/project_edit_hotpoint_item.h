#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_item.h"
#include "PanoramaWidget.h"
#include <QEnterEvent>
#include "HoverMenuButton.h"

class project_edit_hotpoint_item : public QWidget
{
	Q_OBJECT
public:
	project_edit_hotpoint_item(Hotspot& hp,QWidget *parent = nullptr);
	~project_edit_hotpoint_item();

public slots:
	void slotCopyHotPoint();
	void slotIconEdit();
	void slotMore(QString);
	void slot_SetMenuItemText(QString, QString, QString);

public:
	void updateItem(Hotspot& hp);

signals:
	void sig_CopyIcon(Hotspot);
	void sig_insertNew(QString, Hotspot);
	void sig_editIcon(QString);
	void sig_operate(QString,QString);
private:
	QString m_iconUUID;
	QString m_iconPath;


	Ui::project_edit_hotpoint_itemClass ui;
};

