#include "project_style_picText.h"
#include <QSqlQuery>
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QTimer>

project_style_picText::project_style_picText(QStringList picList, QString title, QWidget *parent)
	: m_picList(picList), m_title(title), QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(
		Qt::Window                // 基础窗口标识
		| Qt::WindowCloseButtonHint // 只显示关闭按钮
		| Qt::MSWindowsFixedSizeDialogHint // 可选：固定窗口大小，防止用户拉伸
	);
	setStyleSheet("#MainWindow { background-color: transparent; }"); // 精准指定窗口透明
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_NoSystemBackground, false);

	setWindowTitle(u8"图册");
	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->availableGeometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height();
	scrWidth -= 100;
	scrHeight -= 100;
	resize(scrWidth, scrHeight);
	move((screenRect.width() - width()) / 2, (screenRect.height() - getBottomPix() - height()) / 2);
	ui.listWidget_pics->setGeometry( 20, scrHeight - 160 - 20, scrWidth - 20 * 2, 160);

	int h = scrHeight - 160 - 20 * 2 - 100;

	ui.label_picShow->resize(h * 1.5, h);
	ui.label_picShow->move((scrWidth - h * 1.5) / 2, (scrHeight - 200 - h) / 2);

	ui.label_title->setText(title);
	ui.label_title->setGeometry(20, 10, scrWidth - 20 * 2, 40);

	ui.listWidget_pics->setViewMode(QListView::IconMode);   //设置显示图标模式
	ui.listWidget_pics->setIconSize(QSize(220, 140));         //设置图标大小
	ui.listWidget_pics->setGridSize(QSize(240, 160));       //设置item大小
	ui.listWidget_pics->setMovement(QListView::Static);
	ui.listWidget_pics->setFrameStyle(QFrame::NoFrame);
	ui.listWidget_pics->setSpacing(5);
	ui.listWidget_pics->setFocusPolicy(Qt::NoFocus);
	connect(ui.listWidget_pics, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotOpenPicture(QListWidgetItem*)));

	QTimer::singleShot(0, this, &project_style_picText::processNextPicItem);
}

project_style_picText::~project_style_picText()
{}

void project_style_picText::processNextPicItem()
{
	if (m_currentItemIndex >= m_picList.size() || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}
	QListWidgetItem* item = new QListWidgetItem(QIcon(m_picList[m_currentItemIndex]), "",ui.listWidget_pics);
	item->setData(Qt::UserRole, m_picList[m_currentItemIndex]);
	ui.listWidget_pics->addItem(item);

	if (m_currentItemIndex == 0)
	{
		ui.listWidget_pics->setCurrentItem(item);
		slotOpenPicture(item);
	}
	//
	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_style_picText::processNextPicItem);
}

void project_style_picText::slotOpenPicture(QListWidgetItem* item)
{
	QString picPath = item->data(Qt::UserRole).toString();

	QPixmap pixmap(picPath);
	if (pixmap.isNull()) {
		ui.label_picShow->setText("图片加载失败");
		return;
	}
	// 2. 关键设置：小图居中，大图按比例填满QLabel（不拉伸）
	ui.label_picShow->setAlignment(Qt::AlignCenter);  // 小图居中
	ui.label_picShow->setScaledContents(false);       // 关闭直接拉伸，改用手动缩放

	// 3. 按QLabel尺寸缩放图片（保持宽高比）
	QPixmap scaledPixmap = pixmap.scaled(
		ui.label_picShow->size(),        // 目标尺寸（QLabel的大小）
		Qt::KeepAspectRatio,  // 保持宽高比，避免变形
		Qt::SmoothTransformation  // 平滑缩放，图片更清晰
	);

	// 4. 设置缩放后的图片到QLabel
	ui.label_picShow->setPixmap(scaledPixmap);
}

