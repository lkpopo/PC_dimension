#include "project_edit_mask.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>

project_edit_mask::project_edit_mask(QString projUUID, QString picID, QString picPath, QWidget *parent)
	:m_projUUID(projUUID), m_picID(picID), m_picPath(picPath), QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	resize(250, scrHeight - 60);
	ui.label_bg->resize(250, scrHeight - 60);
	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));
	ui.pushButton_done->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	QString style;
	QString pamLong = "0;360;0;180";
	{
		QString data = QString("select * from mask where pic_id = '%1'").arg(m_picID);
		QSqlQuery query(data);
		while (query.next())
		{
			style = query.value(2).toString();
			pamLong = query.value(3).toString();
			
			m_isHave = true;
		}
	}
	QStringList range = pamLong.split(";");

	ui.horizontalSlide_Long->setStyleSheet("QSlider::handle { background: transparent; }"
		"QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");
	ui.horizontalSlide_Long->setRange(0.0, 360.0);
	ui.horizontalSlide_Long->setTickInterval(0.1);
	ui.horizontalSlide_Long->setHandleValue(Handle_Bottom_Left_Long, range[0].toFloat());
	ui.horizontalSlide_Long->setHandleValue(Handle_Bottom_Right_Long, range[1].toFloat());
	ui.label_Long_min->setText(QString::number(range[0].toFloat(), 10, 0));
	ui.label_Long_max->setText(QString::number(range[1].toFloat(), 10, 0));
	connect(ui.horizontalSlide_Long, SIGNAL(handleValueChangedLong(HandlePos_Long, double)), this, SLOT(slotSlideValueChanged_Long(HandlePos_Long, double)));

	ui.horizontalSlide_Lat->setStyleSheet("QSlider::handle { background: transparent; }"
		"QSlider::groove:horizontal {background: transparent; border: none;margin: 0px; }");
	ui.horizontalSlide_Lat->setRange(0.0, 180.0);
	ui.horizontalSlide_Lat->setTickInterval(0.1);
	ui.horizontalSlide_Lat->setHandleValue(Handle_Bottom_Left_Lat, range[2].toFloat());
	ui.horizontalSlide_Lat->setHandleValue(Handle_Bottom_Right_Lat, range[3].toFloat());
	ui.label_Lat_min->setText(QString::number(range[2].toFloat(), 10, 0));
	ui.label_Lat_max->setText(QString::number(range[3].toFloat(), 10, 0));
	connect(ui.horizontalSlide_Lat, SIGNAL(handleValueChangedLat(HandlePos_Lat, double)), this, SLOT(slotSlideValueChanged_Lat(HandlePos_Lat, double)));
}

project_edit_mask::~project_edit_mask()
{}

void project_edit_mask::slotSlideValueChanged_Long(HandlePos_Long pos, double value)
{
	if (pos == Handle_Bottom_Left_Long)
		ui.label_Long_min->setText(QString::number(value));
	else
		ui.label_Long_max->setText(QString::number(value));

	QString style;
	if (pos == Handle_Bottom_Left_Long)	
		style = "Region_thetaMin";
	else 
		style = "Region_thetaMax";
	//
	double thetaMin = ui.label_Long_min->text().toDouble();
	double thetaMax = ui.label_Long_max->text().toDouble();
	double phiMin = ui.label_Lat_min->text().toDouble();
	double phiMax = ui.label_Lat_max->text().toDouble();
	emit sig_angle(thetaMin, thetaMax, phiMin, phiMax);
}

void project_edit_mask::slotSlideValueChanged_Lat(HandlePos_Lat pos, double value)
{
	if (pos == Handle_Bottom_Left_Lat)
		ui.label_Lat_min->setText(QString::number(value));
	else
		ui.label_Lat_max->setText(QString::number(value));

	QString style;
	if (pos == Handle_Bottom_Left_Lat)
		style = "Region_phiMin";
	else
		style = "Region_phiMax";
	//
	double thetaMin = ui.label_Long_min->text().toDouble();
	double thetaMax = ui.label_Long_max->text().toDouble();
	double phiMin = ui.label_Lat_min->text().toDouble();
	double phiMax = ui.label_Lat_max->text().toDouble();
	emit sig_angle(thetaMin, thetaMax, phiMin, phiMax);
}

void project_edit_mask::slotDone()
{
	QString thetaMin = ui.label_Long_min->text();
	QString thetaMax = ui.label_Long_max->text();
	QString phiMin = ui.label_Lat_min->text();
	QString phiMax = ui.label_Lat_max->text();

	QString style = "LongLat";
	QString area = thetaMin + ";" + thetaMax + ";" + phiMin + ";" + phiMax;
	if(m_isHave == false)
	{
		//遮罩
		QString sql = QString("INSERT INTO mask( \
				pic_id, \
				pic_path, \
				style, \
				area\
			) VALUES (\
				'%1', \
				'%2', \
				'%3', \
				'%4'  \
			); ")
			.arg(m_picID)
			.arg(m_picPath)
			.arg(style)
			.arg(area);
		//执行语句
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
		emit sig_Msg(u8"添加成功!");
	}
	else
	{
		QString updata = QString("update mask set style = '%1',area = '%2' where pic_id = '%3'").arg(style).arg(area).arg(m_picID);
		QSqlQuery query;
		query.exec(updata);
		emit sig_Msg(u8"更新成功!");
		//更新修改时间
		{
			QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
			QString updata = QString("update project set update_time = '%1' where id = '%2'").arg(update_time).arg(m_projUUID);
			QSqlQuery query;
			query.exec(updata);
		}
	}
}
