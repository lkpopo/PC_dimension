#pragma once

#include <QWidget>
#include "ui_project_edit_mask.h"

#include "MultiPosSlider_Longitude.h"
#include "MultiPosSlider_Latitude.h"

class project_edit_mask : public QWidget
{
	Q_OBJECT

public:
	project_edit_mask(QString projUUID, QString picID, QString picPath, QWidget *parent = nullptr);
	~project_edit_mask();

public slots:
	void slotDone();
	void slotSlideValueChanged_Long(HandlePos_Long pos, double value);
	void slotSlideValueChanged_Lat(HandlePos_Lat pos, double value);

signals:
	void sig_angle(double, double, double, double);
	void sig_Msg(QString);

private:
	QString m_projUUID;
	QString m_picID;
	QString m_picPath;

	bool m_isHave = false;

	Ui::project_edit_maskClass ui;
};

