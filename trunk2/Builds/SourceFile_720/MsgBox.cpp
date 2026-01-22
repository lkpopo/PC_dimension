#include "MsgBox.h"
#include <QApplication>
#include "styleSheet.h"

MsgBox::MsgBox(int style, QString text, QWidget* parent) :QDialog(parent)
{
    //
    setAttribute(Qt::WA_QuitOnClose, false);
    //
    //

    this->resize(340, 167);
    //获取主界面的宽度
    int width = this->width();
    int height = this->height();
    setStyleSheet(QString("QWidget{background-color: rgb(255,255,255);font-family :微软雅黑;font: %1pt;color : rgb(0,0,0);}").arg(getFontSize(12)));
    //初始化为未按下鼠标左键

    //设置标题栏隐藏
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    bkgLabel = new QLabel(this);
    bkgLabel->setGeometry(0, 0, width, height);
    bkgLabel->setStyleSheet("QLabel{border-width: 0px;border-style: solid;border-color: rgb(78, 107,125);background-color: rgb(255,255,255);color : rgb(0,0,0);}");

    titleLabel = new QLabel(this);
    titleLabel->setGeometry(0, 0, width, 30);
    if (style == 1)
        titleLabel->setText(u8"  警告  ");
    else
        titleLabel->setText(u8"  提示  ");
    titleLabel->setStyleSheet(QString("QLabel{font-family :微软雅黑;font: %1pt;color: rgb(0, 0, 0);background-color: rgb(255,255,255); border-radius:0px;}").arg(getFontSize(12)));

    closeBtn = new QPushButton(this);
    closeBtn->setGeometry(width - 25, 5, 18, 18);
    closeBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));
    msgBtn = new QPushButton(this);
    msgBtn->setGeometry(25, 56, 32, 32);
    if (style == 1)      //警告
        msgBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/warning.png)};").arg(QApplication::applicationDirPath()));
    else if (style == 2)  //提示
        msgBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/info.png)};").arg(QApplication::applicationDirPath()));
    else if (style == 3)  //询问
        msgBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/question.png)};").arg(QApplication::applicationDirPath()));
    else if (style == 4)  //错误
        msgBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/error.png)};").arg(QApplication::applicationDirPath()));


    QString tipText = text;
    QFont font;
    font.setFamily("微软雅黑");//字体
    //设置显示字体的大小
    font.setPixelSize(14);
    QFontMetrics fm(font);
    int charWidth = fm.horizontalAdvance(tipText);
    charWidth = fm.boundingRect(tipText).width();

    askLabel = new QLabel(this);
    askLabel->setAlignment(Qt::AlignLeft);
    askLabel->setGeometry(120, 70, width - 10, 50);
    askLabel->setText(text);
    askLabel->setStyleSheet("QLabel{background-color: rgb(255,255,255);font-family :微软雅黑;font-size:14px;color : rgb(0,0,0);}");

    okBtn = new QPushButton(this);
    if (style == 3)
        okBtn->setGeometry(80, 119, 85, 30);
    else
        okBtn->setGeometry(125, 119, 85, 30);

    okBtn->setText(QStringLiteral("确定"));
    okBtn->setStyleSheet(QString("QPushButton{font: %2pt \"微软雅黑\";color: rgb(0, 0, 0);border-image:url(%1/Resource/common/cancel.png);}"
        "QPushButton::hover{border-image:url(%1/Resource/common/ok.png);}"
        "QPushButton::pressed{border-image:url(%1/Resource/common/ok.png);}").arg(QApplication::applicationDirPath()).arg(getFontSize(12)));

    cancleBtn = new QPushButton(this);
    cancleBtn->setGeometry(177, 119, 85, 30);
    cancleBtn->setText(QStringLiteral("取消"));
    cancleBtn->setStyleSheet(QString("QPushButton{font: %2pt \"微软雅黑\";color: rgb(0, 0, 0);border-image:url(%1/Resource/common/cancel.png);}"
        "QPushButton::hover{border-image:url(%1/Resource/common/ok.png);}"
        "QPushButton::pressed{border-image:url(%1/Resource/common/ok.png);}").arg(QApplication::applicationDirPath()).arg(getFontSize(12)));

    if (style == 3)
        cancleBtn->show();
    else
        cancleBtn->hide();

    connect(okBtn, SIGNAL(clicked()), this, SLOT(okBtn_press()));
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(closeBtn_press()));
    connect(cancleBtn, SIGNAL(clicked()), this, SLOT(cancleBtn_press()));

}
MsgBox::~MsgBox()
{
    delete okBtn;
    delete cancleBtn;
    delete closeBtn;
    delete msgBtn;
    delete titleLabel;
    delete askLabel;
}

void MsgBox::mousePressEvent(QMouseEvent* qevent)
{
    if (qevent->button() == Qt::LeftButton)
    {
        mouse_press = true;
        //鼠标相对于窗体的位置（或者使用event->globalPos() - this->pos()）
        move_point = qevent->pos();;
    }
}
void MsgBox::mouseMoveEvent(QMouseEvent* qevent)
{
    //若鼠标左键被按下
    if (mouse_press)
    {
        //鼠标相对于屏幕的位置
        QPoint move_pos = qevent->globalPos();
        //移动主窗体位置
        this->move(move_pos - move_point);
    }
}
void MsgBox::mouseReleaseEvent(QMouseEvent* qevent)
{
    //设置鼠标为未被按下
    mouse_press = false;
}

void MsgBox::okBtn_press()
{
    this->accept();
}
void MsgBox::cancleBtn_press()
{
    this->reject();
}
void MsgBox::closeBtn_press()
{
    close();
}

void MsgBox::closeWin()
{
}
