#include "project_new_pictrue_resee.h"

project_new_pictrue_resee::project_new_pictrue_resee(QString picPath, QWidget *parent)
	:m_picPath(picPath),
	QWidget(parent)
{
	ui.setupUi(this);

	setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
	setAttribute(Qt::WA_TranslucentBackground, true);
	setAttribute(Qt::WA_QuitOnClose, true);

	m_panoramaWidget = new PanoramaWidget(picPath);
	ui.horizontalLayout->addWidget(m_panoramaWidget);

	connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
	ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
}

project_new_pictrue_resee::~project_new_pictrue_resee()
{}

void project_new_pictrue_resee::slotClose()
{
	this->close();
	this->destroy(true);
}

