#include "MsgBox.h"
#include <QApplication>
#include "styleSheet.h"
// 新增必要头文件
#include <QMouseEvent>

// 定义边框宽度常量，方便统一调整
const int BORDER_WIDTH = 1;

MsgBox::MsgBox(int style, QString text, QWidget* parent) :QDialog(parent),
mouse_press(false)  // 初始化鼠标按下状态
{
    // 关闭时不退出应用
    setAttribute(Qt::WA_QuitOnClose, false);

    // 原始窗口尺寸增加边框宽度（左右+上下都加），避免边框遮挡内容
    this->resize(340 + 2 * BORDER_WIDTH, 167 + 2 * BORDER_WIDTH);
    //获取主界面的宽度（已包含边框）
    int width = this->width();
    int height = this->height();

    //设置标题栏隐藏
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    // 设置窗口背景透明，让边框样式生效
    this->setAttribute(Qt::WA_TranslucentBackground);

    // ========== 核心修改：添加边框样式 ==========
    bkgLabel = new QLabel(this);
    // bkgLabel 向内偏移 BORDER_WIDTH，留出边框位置
    bkgLabel->setGeometry(BORDER_WIDTH, BORDER_WIDTH,
        width - 2 * BORDER_WIDTH,
        height - 2 * BORDER_WIDTH);
    // 设置带边框的样式：灰色边框 + 白色背景
    bkgLabel->setStyleSheet(QString("QLabel{"
        "border: %1px solid rgb(200, 200, 200);"  // 边框宽度、样式、颜色
        "border-radius: 4px;"  // 可选：圆角效果，更美观
        "background-color: rgb(255,255,255);"
        "color : rgb(0,0,0);}").arg(BORDER_WIDTH));

    titleLabel = new QLabel(this);
    // titleLabel 也向内偏移 BORDER_WIDTH
    titleLabel->setGeometry(BORDER_WIDTH + 2, BORDER_WIDTH + 2,width - 2 * BORDER_WIDTH - 10, 30);
    if (style == 1)
        titleLabel->setText(u8"  警告  ");
    else
        titleLabel->setText(u8"  提示  ");
    titleLabel->setStyleSheet(QString("QLabel{font-family :微软雅黑;font: %1pt;color: rgb(0, 0, 0);background-color: rgb(255,255,255); border-radius:0px;}").arg(getFontSize(12)));

    closeBtn = new QPushButton(this);
    // closeBtn 位置调整（向右偏移 BORDER_WIDTH）
    closeBtn->setGeometry(width - 25 - BORDER_WIDTH - 2, 5 + BORDER_WIDTH + 2, 18, 18);
    closeBtn->setStyleSheet(QString("QPushButton{border-image:url(%1/Resource/common/close.png)};").arg(QApplication::applicationDirPath()));

    msgBtn = new QPushButton(this);
    // msgBtn 位置调整（偏移 BORDER_WIDTH）
    msgBtn->setGeometry(25 + BORDER_WIDTH, 56 + BORDER_WIDTH, 32, 32);
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
    // askLabel 位置调整（偏移 BORDER_WIDTH）
    askLabel->setGeometry(120 + BORDER_WIDTH, 70 + BORDER_WIDTH, width - 10 - 2 * BORDER_WIDTH - 150, 30);
    askLabel->setText(text);
    askLabel->setStyleSheet("QLabel{background-color: rgb(255,255,255);font-family :微软雅黑;font-size:14px;color : rgb(0,0,0);}");

    okBtn = new QPushButton(this);
    // okBtn 位置调整（偏移 BORDER_WIDTH）
    if (style == 3)
        okBtn->setGeometry(80 + BORDER_WIDTH, 119 + BORDER_WIDTH, 85, 30);
    else
        okBtn->setGeometry(125 + BORDER_WIDTH, 119 + BORDER_WIDTH, 85, 30);

    okBtn->setText(QStringLiteral("确定"));
    okBtn->setStyleSheet(QString("QPushButton{font: %2pt \"微软雅黑\";color: rgb(0, 0, 0);border-image:url(%1/Resource/common/cancel.png);}"
        "QPushButton::hover{border-image:url(%1/Resource/common/ok.png);}"
        "QPushButton::pressed{border-image:url(%1/Resource/common/ok.png);}").arg(QApplication::applicationDirPath()).arg(getFontSize(12)));

    cancleBtn = new QPushButton(this);
    // cancleBtn 位置调整（偏移 BORDER_WIDTH）
    cancleBtn->setGeometry(177 + BORDER_WIDTH, 119 + BORDER_WIDTH, 85, 30);
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
    delete bkgLabel;  // 新增：释放bkgLabel内存
}

void MsgBox::mousePressEvent(QMouseEvent* qevent)
{
    if (qevent->button() == Qt::LeftButton)
    {
        mouse_press = true;
        //鼠标相对于窗体的位置（或者使用event->globalPos() - this->pos()）
        move_point = qevent->pos();
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