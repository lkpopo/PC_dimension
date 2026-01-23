#include "project_new.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlError>
#include <QFileDialog>
#include <QListWidgetItem>
#include <QSqlQuery>
#include <QPainter>

project_new::project_new(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_msgBox = nullptr;

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	resize(scrWidth, scrHeight);

	ui.label_bg->setGeometry(0, 0, scrWidth, scrHeight);
	ui.label_topTitle->move(20, 20);
	ui.pushButton_close->setGeometry(scrWidth - 36, 4, 32, 32);
	

	ui.groupBox_left->setGeometry(10, 70, scrWidth - 10 * 3 - 300, scrHeight - 70 - 10);
	ui.label_leftcenter_bg->setGeometry(0, 0, scrWidth - 10 * 3 - 300, scrHeight - 70 - 10);

	ui.pushButton_add->move(scrWidth - 10 * 3 - 300 - 2 * 100 - 10 - 30, 20);
	ui.pushButton_clear->move(scrWidth - 10 * 3 - 300 - 100 - 30, 20);

	ui.listWidget->setGeometry(20, 60, scrWidth - 10 * 3 - 300 - 20, scrHeight - 70 - 10 - 60);

	ui.groupBox_topright->move(scrWidth - 10 - 300, 70);

	ui.groupBox_right->setGeometry(scrWidth - 10 - 300, 160, 300, 721);
	ui.label_right_bg->setGeometry(0, 0, 300, 721);

	ui.listWidget->setViewMode(QListView::IconMode);
	ui.listWidget->setSpacing(11);

	//线路名称
	QString data = QString("select line_name from line_name order by create_time desc");
	QSqlQuery query(data);//查询表的内容
	while (query.next())
	{
		ui.comboBox_lineName->addItem(query.value(0).toString());
	}

	//作品分类默认值
	QStringList classifyType = {
		QStringLiteral("输电"),
		QStringLiteral("配电"),
		QStringLiteral("变电"),
		QStringLiteral("施工"),
		QStringLiteral("其他")
	};
	ui.comboBox_classifyType->addItems(classifyType);  //默认第一个

	//connect
	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	connect(ui.pushButton_add, SIGNAL(clicked()), this, SLOT(slotAddPic()));
	connect(ui.pushButton_clear, SIGNAL(clicked()), this, SLOT(slotClear()));
	connect(ui.pushButton_create, SIGNAL(clicked()), this, SLOT(slotCreate()));
	connect(ui.listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(slotOpenPicture(QListWidgetItem*)));

	connect(ui.textEdit_tilte, SIGNAL(textChanged()), this, SLOT(slotEditTitleCount()));  //标题字数统计
	connect(ui.textEdit_introduction, SIGNAL(textChanged()), this, SLOT(slotEditIntCount())); //简介字数统计
	connect(ui.textEdit_keyword, SIGNAL(textChanged()), this, SLOT(slotEditKeyCount())); //关键词字数统计

	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));

	ui.pushButton_add->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_clear->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
	ui.pushButton_create->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));

	ui.comboBox_lineName->setStyleSheet(QString("QComboBox{background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);font: 12pt \"微软雅黑\"; border-width:0;border-style:outset;}"
		"QComboBox::item:hover{background-color:rgb(0, 85, 255)};"
		"QComboBox::item:selected{background-color:rgb(0, 85, 255)};}"));

	ui.comboBox_lineName->setCurrentIndex(-1);
	connect(ui.comboBox_lineName, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelChanged(int)));
	ui.comboBox_lineName->setCurrentIndex(0);

	/*ui.verticalLayout->setSpacing(0);
	ui.verticalLayout_2->setSpacing(0);
	ui.verticalLayout_3->setSpacing(0);*/
}

project_new::~project_new()
{
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}

}

void project_new::slotOpenPicture(QListWidgetItem* item)
{
	QString picPath = item->toolTip();
	//文件格式必须调整
	QProcess* process = new QProcess();
	QString sPicPath = picPath.replace("/", "\\");
	QString program = "cmd.exe";
	QStringList para;
	para << "/c";
	//命令间需要添加空格
	para << QString("rundll32") + QString(" %Systemroot%\\System32\\shimgvw.dll,ImageView_Fullscreen ") + sPicPath;
	process->start(program, para);
}

void project_new::slotClose()
{
	this->close();
	this->destroy(true);
}

void project_new::slotAddPic()
{
	QStringList tmpPathList = QFileDialog::getOpenFileNames(this, u8"选择全景场景", "", u8"场景 (*.png *.jpg *.jpeg)");

	//删除路径重复的
	std::map<QString, bool> mapPicHave;
	for (auto tmpI : tmpPathList)
	{
		mapPicHave[tmpI] = false;
	}
 	for (auto tmpI : tmpPathList)
	{
		for (auto tmpJ : m_ListWidgetPathList)
		{
			if (tmpJ == tmpI)
			{
				mapPicHave[tmpI] = true;
				break;
			}
		}
	}
	//添加
	m_tmpPathList.clear();
	for (auto tmp : mapPicHave)
	{
		if (tmp.second == false)
		{
			m_tmpPathList.append(tmp.first);
		}
	}

	if (m_tmpPathList.count() == 0) return;
	m_currentItemIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在加载场景...", u8"取消", 0, m_tmpPathList.size(), this);
	m_progressDialog->setWindowTitle(u8"请稍等");
	m_progressDialog->setStyleSheet("QProgressDialog {"
		"background-color: white;"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"padding: 10px;"
		"}");
	m_progressDialog->move( width() / 2 - 50, height() / 2 - 20);

	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setWindowFlags(Qt::FramelessWindowHint);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimumDuration(0);

	// 使用定时器开始处理
	QTimer::singleShot(0, this, &project_new::processNextPicItem);
}

void project_new::slotClear()
{
	ui.listWidget->clear();
	ui.label_picNum->setText(QString::number(0));

	//
	m_ListWidgetPathList.clear();
}

void project_new::processNextPicItem()
{
	if (m_currentItemIndex >= m_tmpPathList.size() || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}

	QString picPath = m_tmpPathList[m_currentItemIndex];
	QListWidgetItem* item = new QListWidgetItem(ui.listWidget);
	item->setSizeHint(QSize(300, 100));
	item->setToolTip(picPath);
	ui.listWidget->addItem(item);
	project_new_picture* item2 = new project_new_picture(picPath);
	item2->resize(QSize(300, 100));		
	connect(item2, SIGNAL(sigPic(QString)), this, SLOT(slotDelPic(QString)));
	ui.listWidget->setItemWidget(item, item2);
	//
	m_ListWidgetPathList.append(picPath);
	ui.label_picNum->setText(QString::number(m_ListWidgetPathList.count()));

	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_new::processNextPicItem);
}

void project_new::initializePicItem(QListWidgetItem* item, project_new_picture* item2,const QString& picPath)
{
	item->setSizeHint(QSize(300, 100));
	item->setToolTip(picPath);
	ui.listWidget->addItem(item);

	item2 = new project_new_picture(picPath);
	item2->resize(QSize(300, 100));
	
	connect(item2, SIGNAL(sigPic(QString)), this, SLOT(slotDelPic(QString)));
}

void project_new::slotDelPic(QString picPath)
{
	int num = ui.listWidget->count();
	for (int i = 0; i < ui.listWidget->count(); i++)
	{
		QListWidgetItem* pItem = ui.listWidget->item(i);
		QString tip = pItem->toolTip();
		if (tip.compare(picPath) == 0)
		{
			ui.listWidget->removeItemWidget(pItem);
			delete pItem;

			// 找到第一个匹配项的索引
			int index = m_ListWidgetPathList.indexOf(tip);
			if (index != -1) { // 确保索引有效（存在匹配项）
				m_ListWidgetPathList.removeAt(index);
			}

			break;
		}
	}
	ui.label_picNum->setText(QString::number(num - 1));
}

void project_new::slotCreate()
{
	if (ui.listWidget->count() == 0)
	{
		noticeWin.Notice(this, u8"未添加任何全景图片！", 2000);
		return;
	}
	if (ui.textEdit_lineName->toPlainText() == "")
	{
		noticeWin.Notice(this, u8"线路名称为空！", 2000);
		return;
	}
	if (ui.textEdit_tilte->toPlainText() == "")
	{
		noticeWin.Notice(this, u8"作品标题为空！", 2000);
		return;
	}
	//按钮失效
	ui.pushButton_create->setEnabled(false);

	m_currentItemIndex = 0;
	if (m_progressDialog)
	{
		delete m_progressDialog;
	}
	m_progressDialog = new QProgressDialog(u8"正在保存数据...", u8"取消", 0, ui.listWidget->count(), this);
	m_progressDialog->setWindowTitle(u8"请稍等");
	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setStyleSheet("QProgressDialog {"
		"background-color: white;"
		"border: 2px solid gray;"
		"border-radius: 5px;"
		"padding: 10px;"
		"}");

	m_progressDialog->move(width() / 2 - 50 , height() / 2 - 20);

	m_progressDialog->setCancelButtonText(nullptr);
	m_progressDialog->setWindowFlags(Qt::FramelessWindowHint);
	m_progressDialog->setWindowModality(Qt::WindowModal);
	m_progressDialog->setMinimumDuration(0);

	m_Project_UUID = getUUID();
	//创建项目文件夹
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_Project_UUID;
	QString Groups = dirName + "/Groups";
	QString dirName_pic = dirName + "/Pictures";
	QString dirName_pic_thumb = dirName + "/Pictures_Thumb";
	QString dirName_pic_thumb_tmp = dirName + "/Pictures_Thumb_tmp";
	QDir dir(dirName);
	if (!dir.exists())
	{
		dir.mkdir(dirName);
	}
	QDir dir2(dirName_pic);
	if (!dir2.exists())
	{
		dir2.mkdir(dirName_pic);
	}
	QDir dir3(dirName_pic_thumb);
	if (!dir3.exists())
	{
		dir3.mkdir(dirName_pic_thumb);
	}
	QDir dir4(dirName_pic_thumb_tmp);
	if (!dir4.exists())
	{
		dir4.mkdir(dirName_pic_thumb_tmp);
	}
	//插入数据库
	QString lineName = ui.textEdit_lineName->toPlainText();
	QString volLavel = "";
	QString projectName = ui.textEdit_tilte->toPlainText();
	QString keyWord = ui.textEdit_keyword->toPlainText();
	QString introduction = ui.textEdit_introduction->toPlainText();
	int picture_count = ui.listWidget->count();
	QString classifyType = ui.comboBox_classifyType->currentText(); 

	QListWidgetItem* pItem = ui.listWidget->item(0);
	QString picPath = pItem->toolTip();
	//截取全景图片中间区域512*512区域
	QPixmap currentImage = QPixmap(picPath);
	int x = (currentImage.width() - 512) / 2;
	int y = (currentImage.height() - 512) / 2;
	// 确保裁剪区域在图片范围内
	QPixmap endImage = currentImage.copy(x, y, 512, 512);
	QFileInfo fileInfo(picPath);
	QString suf = fileInfo.suffix();
	QString FrontCoverPath = dirName + "/FrontCover." + suf;
	endImage.save(FrontCoverPath, suf.toUtf8());
	QString front_cover = FrontCoverPath;
	QString create_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	QString update_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
	{
		//线路名称表
		QString sql = QString("INSERT INTO line_name( \
				line_name, \
				voltage_level, \
				create_time \
			) VALUES (\
				'%1',\
				'%2', \
				'%3' \
			); ")
			.arg(lineName)
			.arg(volLavel)
			.arg(create_time);
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}

		//项目表
		QString sql2 = QString("INSERT INTO project( \
				id, \
				project_name, \
				line_name, \
				keyword, \
				introduction, \
				picture_count, \
				front_cover, \
				create_time, \
				update_time, \
				classifyType \
			) VALUES (\
				'%1',\
				'%2', \
				'%3', \
				'%4', \
				'%5', \
				'%6', \
				'%7', \
				'%8', \
				'%9', \
				'%10' \
			); ")
			.arg(m_Project_UUID)
			.arg(projectName)
			.arg(lineName)
			.arg(keyWord)
			.arg(introduction)
			.arg(picture_count)
			.arg(front_cover)
			.arg(create_time)
			.arg(update_time)
			.arg(classifyType);

		//执行语句
		QSqlQuery query2;
		if (!query2.exec(sql2))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}
	// 使用定时器开始处理
	QTimer::singleShot(0, this, &project_new::processInsertPicItem);
}

void project_new::slotEditTitleCount()
{
	QString title = ui.textEdit_tilte->toPlainText();
	int len = title.length();

	// 限制最大 100 字
	if (len > 100) {
		title = title.left(100);
		ui.textEdit_tilte->setPlainText(title);
		// 保持光标在末尾
		QTextCursor cursor = ui.textEdit_tilte->textCursor();
		cursor.movePosition(QTextCursor::End);
		ui.textEdit_tilte->setTextCursor(cursor);
		len = 100;
	}

	// 更新字数显示：例如 "12/100"
	ui.label_titleCount->setText(QString::number(len) + "/100");
}

void project_new::slotEditIntCount()
{
	QString introduction = ui.textEdit_introduction->toPlainText();
	int len = introduction.length();
	// 限制最大 250 字
	if (len > 250) {
		introduction = introduction.left(250);
		ui.textEdit_introduction->setPlainText(introduction);
		// 保持光标在末尾
		QTextCursor cursor = ui.textEdit_introduction->textCursor();
		cursor.movePosition(QTextCursor::End);
		ui.textEdit_introduction->setTextCursor(cursor);
		len = 250;
	}
	// 更新字数显示
	ui.label_IntCount->setText(QString::number(len) + "/250");
}

void project_new::slotEditKeyCount()
{
	QString keyword = ui.textEdit_keyword->toPlainText();
	int len = keyword.length();
	// 限制最大 64 字
	if (len > 64) {
		keyword = keyword.left(64);
		ui.textEdit_keyword->setPlainText(keyword);
		// 保持光标在末尾
		QTextCursor cursor = ui.textEdit_keyword->textCursor();
		cursor.movePosition(QTextCursor::End);
		ui.textEdit_keyword->setTextCursor(cursor);
		len = 64;
	}
	// 更新字数显示
	ui.label_KeyCount->setText(QString::number(len) + "/64");
}



void project_new::slotSelChanged(int index)
{
	QString lineName =  ui.comboBox_lineName->currentText();
	ui.textEdit_lineName->setPlainText(lineName);
}

void project_new::processInsertPicItem()
{
	if (m_currentItemIndex >= ui.listWidget->count() || (m_progressDialog && m_progressDialog->wasCanceled()))
	{
		if (m_progressDialog)
			m_progressDialog->close();

		close();
		//刷新项目列表
		emit sig_Update();

		QScreen* screen = QGuiApplication::primaryScreen();
		QRect screenRect = screen->geometry();
		int scrWidth = screenRect.width();
		int scrHeight = screenRect.height() - getBottomPix();
		m_todoWin = new project_new_todo(m_Project_UUID);
		m_todoWin->setWindowModality(Qt::ApplicationModal);

		connect(m_todoWin, SIGNAL(sig_edit(QString)), this, SIGNAL(sig_Edit(QString)));
		connect(m_todoWin, SIGNAL(sig_see(QString)), this, SIGNAL(sig_See(QString)));
		connect(m_todoWin, SIGNAL(sig_new()), this, SIGNAL(sig_New()));
		m_todoWin->showFullScreen();
	
		return;
	}
	if (m_progressDialog)
	{
		m_progressDialog->setValue(m_currentItemIndex);
	}

	QListWidgetItem* pItem = ui.listWidget->item(m_currentItemIndex);
	QString picPath = pItem->toolTip();

	QString newName;
	QWidget* widget = ui.listWidget->itemWidget(pItem);
	if (widget != nullptr) 
	{
		QLineEdit* targetLabel = widget->findChild<QLineEdit*>("lineEdit_name");
		if (targetLabel) 
			newName = targetLabel->text();	
	}
	//复制图片
	QString picUUID = getUUID();
	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_Project_UUID;
	QFileInfo fileInfo(picPath);
	QString suf = fileInfo.completeSuffix();
	QString newPath = dirName + "/Pictures/" + newName + "." + suf;
	CopyFile(picPath, newPath, true);
	//QImage image(picPath);
	//QImage scaledImage = image.scaled(image.width(), image.height(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
	//scaledImage.save(newPath, "JPG", 50); // 质量参数范围0-100
	//拇指图
	{
		QPixmap currentImage = QPixmap(newPath);
		int x = (currentImage.width() - 512) / 2;
		int y = (currentImage.height() - 512) / 2;
		QPixmap endImage = currentImage.copy(x, y, 512, 512);
		QString thumbPath = dirName + "/Pictures_Thumb/" + newName + "." + suf;
		endImage.save(thumbPath, suf.toUtf8());
	}
	//插入数据库
	QString uuid = getUUID();
	QString front_cover = newPath;
	QString projectName = ui.textEdit_tilte->toPlainText();
	QString start_angle = "0.0;0.0;0.0";
	QString fov = "60.0";
	QString zoom = "1.0";
	QString zoom_range = "0.5;3.0";
	QString h_range = "-180.0;180.0";
	QString v_range = "-90.0;90.0";

	QString compass_s_angle = "0.0";
	QString compass_pic_path = "";
	QString compass_location = ""; // LeftTop,BottomCentre

	QString unsel_bgColor = "26,26,26";
	QString unsel_textColor = "255,255,255";
	QString sel_bgColor = "250,100,0";
	QString sel_textColor = "255,255,255";
	{
		//全景图片表
		QString sql = QString("INSERT INTO picture( \
				id, \
				picture_path, \
				project_name_id, \
				project_name, \
				start_angle,\
				fov,\
				zoom,\
				zoom_range,\
				h_range,\
				v_range,\
				compass_s_angle,\
				compass_pic_path,\
				compass_location,\
				group_unsel_bg_color,\
				group_unsel_text_color,\
				group_sel_bg_color,\
				group_sel_text_color\
			) VALUES (\
				'%1',\
				'%2', \
				'%3', \
				'%4', \
				'%5', \
				'%6', \
				'%7', \
				'%8', \
				'%9', \
				'%10',\
				'%11', \
				'%12', \
				'%13',\
				'%14',\
				'%15',\
				'%16',\
				'%17'\
			); ")
			.arg(picUUID)
			.arg(newPath)
			.arg(m_Project_UUID)
			.arg(projectName)
			.arg(start_angle)
			.arg(fov)
			.arg(zoom)
			.arg(zoom_range)
			.arg(h_range)
			.arg(v_range)
			.arg(compass_s_angle)
			.arg(compass_pic_path)
			.arg(compass_location)
			.arg(unsel_bgColor)
			.arg(unsel_textColor)
			.arg(sel_bgColor)
			.arg(sel_textColor);
		//执行语句
		QSqlQuery query;
		if (!query.exec(sql))
		{
			qDebug() << QString::fromLocal8Bit("failed!") << query.lastError();
		}
	}

	m_currentItemIndex++;

	// 继续处理下一个图片，保持UI响应
	QTimer::singleShot(30, this, &project_new::processInsertPicItem);
}
