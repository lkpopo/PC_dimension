#pragma once

#include <QWidget>
#include "ui_project_hotpoint_icon.h"
#include "project_see_item.h"

class project_hotpoint_icon : public QWidget
{
	Q_OBJECT

public:
	project_hotpoint_icon(QWidget *parent = nullptr);
	~project_hotpoint_icon();

	void processNexIconItem();

public slots:
	void slotClose();
	void slotDone();
	void slotNewIconClick(QString iconPath);

signals:
	void sig_iconPath(QString);
protected:
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	QRect m_rectMovableArea;//窗口可以拖动的区域

private:
	QPoint m_mouseSrcPos;//鼠标点 
	QPoint m_mouseDstPos;//鼠标点
	bool m_mousePressFlag;//鼠标按下状态
	bool m_mouseMoveWindowFlag;//鼠标移动窗口状态


	QStringList m_sysIconPathList;
	QString m_selIcon;
	int m_currentItemIndex;
	Ui::project_hotpoint_iconClass ui;
};

