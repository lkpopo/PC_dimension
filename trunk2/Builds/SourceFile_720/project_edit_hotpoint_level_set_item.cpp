#include "project_edit_hotpoint_level_set_item.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QEnterEvent>
project_edit_hotpoint_level_set_item::project_edit_hotpoint_level_set_item(Hotspot& hp, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_iconUUID = hp.iconID;
	m_iconPath = hp.iconPath;
	QPixmap pixmap(hp.iconPath);
	QPixmap fitpixmap = pixmap.scaled(20, 20, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_icon->setPixmap(fitpixmap);
	ui.pushButton_title->setText(hp.name);

	if (hp.style == "nornmal;")
		ui.pushButton_changPic->setText(u8"一般热点");
	else if (hp.style.contains("picText;") == true)
		ui.pushButton_changPic->setText(u8"图文展示");
	else
		ui.pushButton_changPic->setText(u8"场景切换");

	connect(ui.pushButton_title, SIGNAL(clicked()), this, SLOT(slotSelOperation()));
	connect(ui.pushButton_changPic, SIGNAL(clicked()), this, SLOT(slotChangePic()));
	connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SIGNAL(sig_SelStatus(int)));
}

project_edit_hotpoint_level_set_item::~project_edit_hotpoint_level_set_item()
{}

void project_edit_hotpoint_level_set_item::slotSelOperation()
{
	//定位到位置
	emit sig_edit_hp_batch_item_RotateCutHotPoint(m_iconUUID);
	//选择
	bool blCheck = ui.checkBox->isChecked();
	ui.checkBox->setChecked(!blCheck);
}




