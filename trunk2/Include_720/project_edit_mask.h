#pragma once

#include <QWidget>
#include "ui_project_edit_mask.h"

class project_edit_mask : public QWidget
{
	Q_OBJECT

public:
	project_edit_mask(QString projUUID, QWidget *parent = nullptr);
	~project_edit_mask();

private:
	Ui::project_edit_maskClass ui;
};

