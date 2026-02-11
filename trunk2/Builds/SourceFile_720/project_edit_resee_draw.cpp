#include "project_edit_resee_draw.h"

project_edit_resee_draw::project_edit_resee_draw(QString projUUID, QWidget *parent)
	: m_projID(projUUID), QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(
		Qt::Window                // 基础窗口标识
		| Qt::WindowCloseButtonHint // 只显示关闭按钮
		| Qt::MSWindowsFixedSizeDialogHint // 可选：固定窗口大小，防止用户拉伸
	);
	setWindowTitle(u8"施工图");

	resize(1200,800);
	ui.label_draw->resize(1182, 782);

	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projID;
	QString picPath = dirName + "/Engineering_Drawings/Engineering_Drawings.jpg";

	QPixmap pixmap(picPath);
	if (pixmap.isNull()) {
		ui.label_draw->setText("图片加载失败");
		return;
	}
	// 2. 关键设置：小图居中，大图按比例填满QLabel（不拉伸）
	ui.label_draw->setAlignment(Qt::AlignCenter);  // 小图居中
	ui.label_draw->setScaledContents(false);       // 关闭直接拉伸，改用手动缩放

	// 3. 按QLabel尺寸缩放图片（保持宽高比）
	QPixmap scaledPixmap = pixmap.scaled(
		ui.label_draw->size(),        // 目标尺寸（QLabel的大小）
		Qt::KeepAspectRatio,  // 保持宽高比，避免变形
		Qt::SmoothTransformation  // 平滑缩放，图片更清晰
	);

	// 4. 设置缩放后的图片到QLabel
	ui.label_draw->setPixmap(scaledPixmap);
}

project_edit_resee_draw::~project_edit_resee_draw()
{}

