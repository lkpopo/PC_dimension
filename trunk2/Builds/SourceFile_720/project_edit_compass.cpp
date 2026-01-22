#include "project_edit_compass.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include "panoramawidget.h"  
#include <QSqlQuery>
#include <QFileDialog>
#include <QPixmap>
#include <QDir>
#include <QSqlError>
#include <QMessageBox>


project_edit_compass::project_edit_compass(QString projUUID, QWidget *parent)
	:m_projUUID(projUUID), QWidget(parent)
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

	connect(ui.pushButton_selPic, SIGNAL(clicked()), this, SLOT(slotSetCompassStyle())); //选择指北针样式
	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));
	connect(ui.compass_position, SIGNAL(currentIndexChanged(QString)), this, SLOT(slotSetPosition()));   //设置指北针位置

	ui.pushButton_done->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
}

project_edit_compass::~project_edit_compass()
{}

void project_edit_compass::setCompassPic(QString compPic)
{
	if(compPic == "")
		compPic = QApplication::applicationDirPath() + "/Resource/compass/compass1.png";
	QPixmap pixmap(compPic);
	QPixmap fitpixmap = pixmap.scaled(115, 115, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_compassN_pic->setPixmap(fitpixmap);
}

void project_edit_compass::slotSetPixmap(QPixmap& pix)
{
	QPixmap fitpixmap = pix.scaled(210, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_compassN->setPixmap(fitpixmap);
}

//void project_edit_compass::slotDone()
//{
//	emit sig_save(); //保存
//
//}

void project_edit_compass::slotDone()
{
	if (m_projUUID.isEmpty()) {
		qDebug() << "No current project UUID!";
		return;
	}

	QString compassPicPath = selectedFile /* 你的图片路径，例如 m_selectedCompassPath */;

	//使用参数化查询，防止 SQL 注入
	QSqlQuery query;
	query.prepare("UPDATE picture SET compass_pic_path = ?, compass_location = ? WHERE project_name_id = ?");
	query.addBindValue(compassPicPath);
	query.addBindValue(compassPosition);
	query.addBindValue(m_projUUID);  

	if (!query.exec()) {
		qDebug() << "SQL exec failed:" << query.lastError();
		QMessageBox::warning(this, u8"保存失败", query.lastError().text());
		return;
	}

	qDebug() << "Compass style updated for project:" << m_projUUID;

	noticeWin.Notice(this, u8"保存成功！", 2000);

	//emit sig_save(); // 通知主窗口保存成功，但是不知道为什么会导致更新数据库失败
}



void project_edit_compass::setPanoramaWidget(PanoramaWidget* widget)
{
	//将已存在的全景控件实例传入
	m_panWidget = widget;
}
void project_edit_compass::slotSetPosition()
{
	if (!m_panWidget) {
		qWarning() << "m_panWidget is null!";
		return;
	}

	QString text = ui.compass_position->currentText();
	if (text == u8"左上角") {
		compassPosition = "LeftTop";
		m_panWidget->setCompassLocation("LeftTop");

	}
	else if (text == u8"底部居中") {
		compassPosition = "BottomCenter";
		m_panWidget->setCompassLocation("BottomCenter");
	}
	else if (text == u8"右上角") {
		compassPosition = "RightTop";
		m_panWidget->setCompassLocation("RightTop");
	}

}


void project_edit_compass::slotSetCompassStyle()
{
	// 设置默认打开的文件夹路径（可选）
	QString defaultDir = QApplication::applicationDirPath() + "/Resource/compass";
	QDir dir(defaultDir);
	if (!dir.exists()) {
		defaultDir = QDir::homePath(); // 如果资源目录不存在，回退到用户主目录
	}

	// 弹出文件选择对话框，只允许选择图片
	selectedFile = QFileDialog::getOpenFileName(
		this,
		u8"选择指北针样式",          // 对话框标题
		defaultDir,                  // 默认打开路径
		u8"图片文件 (*.png *.jpg *.jpeg *.bmp *.gif);;所有文件 (*)"  // 文件过滤器
	);

	if (!selectedFile.isEmpty()) {
		// 验证是否为有效图片
		if (QPixmap(selectedFile).isNull()) {
			noticeWin.Notice(this, u8"所选文件不是有效的图片格式！", 2000);
			return;
		}

		// 将选中的路径传递给 PanoramaWidget
		if (m_panWidget) {
			m_panWidget->setCompassImagePath(selectedFile); 
		}
		setCompassPic(selectedFile);  // 更新指北针样式窗口的预览
	}

}
