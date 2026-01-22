#include "project_edit_mask.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>

project_edit_mask::project_edit_mask(QString projUUID, QWidget *parent)
	: QWidget(parent)
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

	connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));

	ui.pushButton_done->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
}

project_edit_mask::~project_edit_mask()
{}

