#pragma once

#include <QWidget>
#include "ui_project_edit_resee_draw.h"

class project_edit_resee_draw : public QWidget
{
	Q_OBJECT

public:
	project_edit_resee_draw(QString projUUID, QWidget *parent = nullptr);
	~project_edit_resee_draw();

private:
	QString m_projID;
	Ui::project_edit_resee_drawClass ui;
};

