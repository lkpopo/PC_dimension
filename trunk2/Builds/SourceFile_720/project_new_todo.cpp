#include "project_new_todo.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"


project_new_todo::project_new_todo(QString Project_UUID, QWidget *parent)
	: m_project_UUID(Project_UUID),QWidget(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	QScreen* screen = QGuiApplication::primaryScreen();
	QRect screenRect = screen->geometry();
	int scrWidth = screenRect.width();
	int scrHeight = screenRect.height() - getBottomPix();

	ui.label_bg->resize(scrWidth, scrHeight);
	ui.groupBox->move((scrWidth - 360) / 2, (scrHeight - 400) / 2);

	connect(ui.pushButton_edit, SIGNAL(clicked()), this, SLOT(slotEdit()));
	connect(ui.pushButton_see, SIGNAL(clicked()), this, SLOT(slotSee()));
	connect(ui.pushButton_renew, SIGNAL(clicked()), this, SLOT(slotNew()));

	ui.pushButton_edit->setStyleSheet(QString("QPushButton{border-radius:5px;background-color: rgb(40, 110, 250);color: rgb(255, 255, 255);}"
		"QPushButton::hover{background-color: rgb(0, 85, 255);}"));
}

project_new_todo::~project_new_todo()
{}

void project_new_todo::slotEdit()
{
	close();
	emit sig_edit(m_project_UUID);	
}

void project_new_todo::slotSee()
{
	close();
	emit sig_see(m_project_UUID);
}

void project_new_todo::slotNew()
{
	close();
	emit sig_new();
}