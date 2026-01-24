#include "project_edit_delPics_item.h"
#include <QFileInfo>


project_edit_delPics_item::project_edit_delPics_item(QString projID, QString picID, QString picPath, QWidget *parent)
	:m_projID(projID), m_picID(picID), m_picPath(picPath),QWidget(parent)
{
	ui.setupUi(this);

	QString dirName = QApplication::applicationDirPath() + "/DataBase/" + m_projID;
	QString file_name = m_picPath.section("/", -1).section(".", 0, -2);
	QFileInfo fileInfo(m_picPath);
	QString suf = fileInfo.completeSuffix();
	QString thumbPath = dirName + "/Pictures_Thumb/" + file_name + "." + suf;

	QPixmap pixmap(thumbPath);
	QPixmap fitpixmap = pixmap.scaled(32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	ui.label_icon->setPixmap(fitpixmap);
	ui.pushButton_title->setText(file_name);

	connect(ui.checkBox, SIGNAL(stateChanged(int)), this, SIGNAL(sig_SelStatus(int)));
	connect(ui.pushButton_title, SIGNAL(clicked()), this, SLOT(slotSelOperation()));

}

project_edit_delPics_item::~project_edit_delPics_item()
{}

void project_edit_delPics_item::slotSelOperation()
{
	bool blCheck = ui.checkBox->isChecked();
	ui.checkBox->setChecked(!blCheck);
}



