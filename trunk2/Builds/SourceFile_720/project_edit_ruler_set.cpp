#include "project_edit_ruler_set.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QPixmap>

project_edit_ruler_set::project_edit_ruler_set(QString projUUID, QString picID, QString rulerID, QWidget *parent)
	:m_projUUID(projUUID), m_picID(picID), m_rulerID(rulerID), QWidget(parent)
{
	ui.setupUi(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_QuitOnClose, true);
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrHeight = screenRect.height() - getBottomPix();
    //¶¯Ì¬³¤¶È
    {
        resize(250, scrHeight - 60 + 10);
        ui.label_bg->resize(250, scrHeight - 60 + 10);
        ui.pushButton_done->move(10, scrHeight - 60 - 10 - 30);
    }

    m_isExist = false;

    connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(slotClose()));
    connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));

    ui.pushButton_close->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));


}

project_edit_ruler_set::~project_edit_ruler_set()
{}

