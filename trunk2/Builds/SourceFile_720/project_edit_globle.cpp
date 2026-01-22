#include "project_edit_globle.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>


project_edit_globle::project_edit_globle(QString projUUID, QString cutPicPath, QWidget *parent)
	: m_projUUID(projUUID), m_cutPicPath(cutPicPath), QWidget(parent)
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
	//ui.pushButton_done->move(10, scrHeight - 60 - 10 -25);

	//初始化
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + projUUID;
	QFileInfo fileInfo(m_cutPicPath);
	QString suf = fileInfo.suffix();
	QString frontcover = dirName + "/FrontCover." + suf;
	QPixmap pixmap(frontcover); // 图片路径
	QPixmap fitpixmap = pixmap.scaled(131, 131, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_coverPic->setPixmap(fitpixmap);

	QString sql = QString("SELECT * from project where id = '%1'").arg(m_projUUID);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		ui.textEdit_tilte->setText(query.value(1).toString());
		ui.textEdit_keyword->setText(query.value(3).toString());
		ui.textEdit_introduction->setText(query.value(4).toString());
		ui.textEdit_classifyType->setText(query.value(9).toString());  //分类

		//更新字数显示
		updateCharCount(ui.textEdit_tilte, ui.label_titleCount, 50);
		updateCharCount(ui.textEdit_keyword, ui.label_keyCount, 100);
		updateCharCount(ui.textEdit_introduction, ui.label_itdCount, 500);
	}



	//connect
	connect(ui.pushButton_setcover, SIGNAL(clicked()), this, SLOT(slotSetFrontCover()));
	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));

	ui.pushButton_setcover->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_done->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	
	connect(ui.textEdit_tilte, &QTextEdit::textChanged, this, [this]() {
		updateCharCount(ui.textEdit_tilte, ui.label_titleCount, 100);
		});
	connect(ui.textEdit_keyword, &QTextEdit::textChanged, this, [this]() {
		updateCharCount(ui.textEdit_keyword, ui.label_keyCount, 64);
		});
	connect(ui.textEdit_introduction, &QTextEdit::textChanged, this, [this]() {
		updateCharCount(ui.textEdit_introduction, ui.label_itdCount, 250);
		});



}

project_edit_globle::~project_edit_globle()
{}

// 辅助函数：更新 QTextEdit 对应的字数标签
void project_edit_globle::updateCharCount(QTextEdit* edit, QLabel* label, int maxChars)
{
	if (!edit || !label) return;
	int current = edit->toPlainText().length(); // 注意：使用 length() 是字符数（中文=1）
	label->setText(QString("%1/%2").arg(current).arg(maxChars));
}



void project_edit_globle::slotClose()
{
	this->close();
	this->destroy(true);
}

void project_edit_globle::slotSetFrontCover()
{
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();

	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	QStringList listPicPath;
	QString sql = QString("SELECT picture_path from picture where project_name_id = '%1'").arg(m_projUUID);
	QSqlQuery query(sql);
	QString lineName;
	while (query.next())
	{
		listPicPath.append(query.value(0).toString());
	}
	if (m_frontCover)
	{
		delete m_frontCover;
	}
	m_frontCover = new project_edit_globle_frontCover(m_projUUID, listPicPath);
	m_frontCover->setWindowModality(Qt::ApplicationModal);
	m_frontCover->move((scrWidth - m_frontCover->width()) / 2, (scrHeight - m_frontCover->height()) / 2);
	connect(m_frontCover, SIGNAL(sig_change()), this, SLOT(slotReSeeFrontCover()));
	m_frontCover->show();
}

void project_edit_globle::slotDone()
{
	QString title = ui.textEdit_tilte->toPlainText();
	QString keyword = ui.textEdit_keyword->toPlainText();
	QString introduction = ui.textEdit_introduction->toPlainText();
	if (title == "")
	{
		emit sig_Msg(u8"标题不能为空！");
		return;
	}
	//封面
	QFileInfo fileInfo(m_cutPicPath);
	QString suf = fileInfo.suffix();
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;
	QString frontcover = dirName + "/FrontCover." + suf;
	QString frontcover_tmp = dirName + "/FrontCover_tmp." + suf;
	
	QFile file(frontcover_tmp);
	if (file.exists())
	{
		bool bl1 = QFile::remove(frontcover);
	}
	bool bl2 = QFile::rename(frontcover_tmp,frontcover);
	//更新数据库
	QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	QString updata = QString("update project set project_name = '%1', keyword = '%2',introduction = '%3', update_time = '%4' where id = '%5'").arg(title).arg(keyword).arg(introduction).arg(update_time).arg(m_projUUID);
	QSqlQuery query;
	if (!query.exec(updata))
	{
		emit sig_Msg(u8"保存失败！");
	}
	else
	{
		emit sig_Msg(u8"保存成功！");
	}
}

void project_edit_globle::slotSelPic()
{
}

void project_edit_globle::slotReSeeFrontCover()
{
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projUUID;

	QFileInfo fileInfo(m_cutPicPath);
	QString suf = fileInfo.suffix();
	QString frontcover = dirName + "/FrontCover_tmp." + suf;
	QPixmap pixmap(frontcover); 
	QPixmap fitpixmap = pixmap.scaled(131, 131, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_coverPic->setPixmap(fitpixmap);
}


