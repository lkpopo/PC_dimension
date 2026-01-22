#include "LoginWindows.h"
//#include "interface.h"
#include <QtConcurrent/QtConcurrentRun>
#include "CommonTool.h"
#include <QProcess>
#include <QStandardPaths>
#include <QtNetwork/QHostInfo>
#include <QFileDialog>
#include "OpeateDB.h"
#include "QClipboard"
#include "json/reader.h"
#include "json/value.h"
#include "json/writer.h"
#include "styleSheet.h"
#include "md5.h"
#include "..\..\Include_720\LoginWindows.h"

LoginWindows::LoginWindows(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	//
	setAttribute(Qt::WA_QuitOnClose, false);
	//
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	m_rectMovableArea = QRect(0, 0, width(), 60);
	//
	m_msgBox = nullptr;
	m_see = false;
	setStyle();

	connect(this, SIGNAL(sig_loginNum(QString)), this, SLOT(slotLoginInfoBox(QString)));
	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.m_loadFile, &QPushButton::clicked, this, &LoginWindows::loadFile);
	connect(ui.generateFile, &QPushButton::clicked, this, &LoginWindows::generateFile);
	connect(ui.m_loadInfo, &QPushButton::clicked, this, &LoginWindows::loadCode);
	connect(ui.m_copyButton, &QPushButton::clicked, [&]() {
		QClipboard* clipboard = QApplication::clipboard();   //获取系统剪贴板指针
		clipboard->setText(ui.textEdit->toPlainText());		     //设置剪贴板内容</span>
		noticeWin.Notice(this, u8"已复制到系统剪贴板！", 1000);
		});

	//机器码转成MD5
	std::string machine = GetSerialFromSys();
	std::string machineID = "machineID:" + machine;
	qDebug() << machineID.c_str();
	std::string machineMd5 = MD5(machine).toStr().substr(8, 16);
	machineMd5 = machineMd5.insert(4, "-");
	machineMd5 = machineMd5.insert(9, "-");
	machineMd5 = machineMd5.insert(14, "-");
	transform(machineMd5.begin(), machineMd5.end(), machineMd5.begin(), ::toupper);

	m_originComputerKey = machine;
	m_deviceId = machineMd5;
	m_startTime = "0";
	m_endTime = "0";

	ui.lineEdit->setText(QString::fromStdString(machineMd5));
	this->setWindowTitle(u8"登录");

	std::string key = "jdtnsjuesgjueguf"; 
	std::string encryptedode = aes_encrypt(generateCode().toStdString(), key);

	m_code = QString::fromStdString(encryptedode);
	ui.textEdit->setText(m_code);
}

LoginWindows::~LoginWindows()
{}

void LoginWindows::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_mousePressFlag = true;
		this->setCursor(Qt::OpenHandCursor);
		m_mouseSrcPos = event->pos();
	}
}

void LoginWindows::mouseReleaseEvent(QMouseEvent* event)
{
	this->setCursor(Qt::ArrowCursor);
	m_mousePressFlag = false;
}

void LoginWindows::mouseMoveEvent(QMouseEvent* event)
{
	if (m_mousePressFlag)
	{
		m_mouseDstPos = event->pos();
		if (PointInArea(m_rectMovableArea, m_mouseSrcPos))
		{
			this->move(this->pos() + m_mouseDstPos - m_mouseSrcPos);
		}
	}
	event->ignore();
}

void LoginWindows::setStyle()
{
	ui.label_11->setStyleSheet(getStyleSheet("QLable", 20));
	ui.label_9->setStyleSheet(getStyleSheet("QLable", 12));
	ui.lineEdit->setStyleSheet(getStyleSheet("QLineEdit", 12));

	ui.textEdit->setStyleSheet(getStyleSheet("QTextEdit_ReadOnly", 12));
	ui.m_info->setStyleSheet(getStyleSheet("QTextEdit", 12));

	ui.m_copyButton->setStyleSheet(getStyleSheet("QButton", 12));
	ui.generateFile->setStyleSheet(getStyleSheet("QButton", 12));
	ui.m_loadFile->setStyleSheet(getStyleSheet("QButton", 12));
	ui.m_loadInfo->setStyleSheet(getStyleSheet("QButton", 12));
}

void LoginWindows::slotLoginInfoBox(QString msg)
{
	noticeWin.Notice(this, msg, 2000);
}

void LoginWindows::slotClose()
{
	this->close();
	this->destroy(true);
}

void LoginWindows::generateFile()
{
	QString dateTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");
	QString dstFile = QFileDialog::getSaveFileName(this, u8"设备", dateTime, u8"设备文件(*.inf)");
	if (dstFile.isNull() || dstFile.isEmpty())
		return;
	QFile file(dstFile);
	if (!file.open(QFile::WriteOnly | QIODevice::Truncate)) {
		return;
	}
	QTextStream in(&file);
	in << m_code;
	noticeWin.Notice(this, u8"生成设备文件成功！", 2000);
}

void LoginWindows::loadFile()
{
	QString fileName = QFileDialog::getOpenFileName(this, u8"打开注册文件", "", u8"注册文件(*.lic)");
	if (fileName.isNull() || fileName.isEmpty())
	{
		return;
	}
	QString message;
	if (analyiseByFile(message, fileName))
	{
		noticeWin.Notice(this, message, 2000);
		this->accept();
	}
	else
	{
		noticeWin.Notice(this, message, 2000);
	}
}

void LoginWindows::Login2()
{
	QString msg;
	bool bllogin = ReAnalyiseByFile(msg);
	if (bllogin == false)
	{
		noticeWin.Notice(this, msg, 2000);
	}
	else
	{
		accept();
	}
}

void LoginWindows::loadCode()
{
	//校验
	QString txt = ui.m_info->toPlainText();
	if (txt == "")
	{
		noticeWin.Notice(this, u8"激活码为空！", 2000);
		return;
	}
	QString message;
	if (analyiseByCode(txt, message))
	{
		noticeWin.Notice(this, message, 2000);
		this->accept();
	}
	else
	{
		noticeWin.Notice(this, message, 2000);
	}
}

std::string JsonAsString(const Json::Value& json)
{
	std::string result;
	Json::StreamWriterBuilder wbuilder;

	wbuilder["indentation"] = "";       // Optional
	result = Json::writeString(wbuilder, json);
	return result;
}

QString LoginWindows::generateCode()
{
	QUuid id0 = QUuid::createUuid();//生成唯一码
	QString strId = id0.toString();//转换为QString
	QString strId1 = strId.replace("{", "");
	QString strId2 = strId1.replace("}", "");
	QString strId3 = strId2.replace("-", "");
	QString aesKey = strId3;
	//加密设备信息
	std::map<std::string, std::string> mapdataObj;
	mapdataObj.insert(std::make_pair("deviceId", m_deviceId));
	mapdataObj.insert(std::make_pair("startTime", m_startTime));
	mapdataObj.insert(std::make_pair("endTime", m_endTime));

	Json::Value rs;
	for (auto& obj : mapdataObj)
	{
		std::string memberName = obj.first;
		std::string memberObj = obj.second;
		rs[memberName] = memberObj;
	}
	std::string dataObj = JsonAsString(rs);

	
	std::string encryptedText = dataObj;

	std::map<std::string, std::string> mapjson;
	mapjson.insert(std::make_pair("data:", encryptedText));
	Json::Value rs2;
	for (auto& obj : mapjson)
	{
		std::string memberName = obj.first;
		std::string memberObj = obj.second;
		rs2[memberName] = memberObj;
	}
	return QString::fromStdString(JsonAsString(rs2));
}