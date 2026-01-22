#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_set.h"
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QParallelAnimationGroup>
#include "project_hotpoint_icon.h"
#include "PanoramaWidget.h"

class project_edit_hotpoint_set : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_set(QString iconID, QWidget *parent = nullptr);
	~project_edit_hotpoint_set();

	
	void slideOut(QWidget*);
	void slideInHide();
public slots:
	void slotClose();
	void slotDone();
	void slotSelPic();
	void slotSetIcon(QString);

	void setHorValueW(int);
	void setHorValueH(int);
	void setHorValueHW(int);
	void setIconName();
	void setShowTitle(int);

	void slotSetTitleBgColor();
	void slotSetTitleTextColor();
	void slotTextSizeChanged(int);

signals:
	void sig_Msg_Set(QString);
	void sig_addIconSet(Hotspot);
	void sig_updateIconSet(QString);
	void sig_setScaleX(float);
	void sig_setScaleY(float);
	void sig_setIconName(QString);
	void sig_IconNameShow(bool);
	void sig_AddIconEnd();
	void sig_EditIconEnd();
	void sig_AddIcon(Hotspot);
	void sig_UpdateIcon(Hotspot);
	void sig_DelTmpIcon();

	void sig_setTitleBgColor(QColor);
	void sig_setTitleTextColor(QColor);
	void sig_setTextSize(int);
	void sig_clearFocus();
	void sig_edit_hp_set_SetHPLocation(bool);
private:
	QParallelAnimationGroup* m_AnimationGroup; // 动画组管理
	QString m_iconUUID;
	QString m_iconPath;

	QColor m_titleBgColor;
	QColor m_titleTextColor;
	int    m_titleTextSize;

	bool m_isExist;//是否是以前存在的
	Hotspot m_oldHpPam;//传入的旧参数

	project_hotpoint_icon* m_iconWin;


	Ui::project_edit_hotpoint_setClass ui;
};

