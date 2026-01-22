#pragma once

#include <QDialog>
#include "ui_LoginWindows.h"
#include <QMovie>
#include <QMouseEvent>
#include "MsgBox.h"
#include "NoticeWidget.h"

class LoginWindows : public QDialog
{
	Q_OBJECT

public:
	LoginWindows(QWidget *parent = nullptr);
	~LoginWindows();

protected:
	void mousePressEvent(QMouseEvent*);
	void mouseReleaseEvent(QMouseEvent*);
	void mouseMoveEvent(QMouseEvent*);
	void setStyle();
public slots:
	void slotLoginInfoBox(QString);
	void slotClose();
	void generateFile();
	void loadFile();
	void Login2();
	void loadCode();
	QString generateCode();

signals:
	void sig_loginNum(QString);
private:
	QPoint m_mouseSrcPos;//鼠标点 
	QPoint m_mouseDstPos;//鼠标点
	bool m_mousePressFlag;//鼠标按下状态
	bool m_mouseMoveWindowFlag;//鼠标移动窗口状态
	QRect m_rectMovableArea;//窗口可以拖动的区域
	bool m_see;//明文，暗文
	MsgBox* m_msgBox;
	NoticeWidget noticeWin;

	//授权2
	QString  m_DeviceInfo;
	QString m_code;//设备码
	std::string m_originComputerKey;
	std::string m_deviceId;
	std::string m_startTime;
	std::string m_endTime;

private:
	Ui::LoginWindowsClass ui;
};
