#include "CustomDialog.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QStyle>
#include <QVBoxLayout>

#include "styleSheet.h"

CustomDialog::CustomDialog(int style, const QString& text, QWidget* parent)
    : QDialog(parent) {
  // 1. 窗口属性设置
  setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
  setAttribute(Qt::WA_QuitOnClose, false);
  setFixedSize(340, 180);  // 稍微增加一点高度，防止文字多时拥挤

  initUi(style, text);
}

void CustomDialog::initUi(int style, const QString& text) {
  // 2. 样式表 - 集中管理
  QString baseStyle =
      QString(
          "QWidget#MainWindow { background-color: white; }"
          "QLabel { font-family: '微软雅黑'; color: black; background: "
          "transparent; }"
          "QPushButton#ActionBtn { "
          "   font: %2pt '微软雅黑'; color: black; "
          "   border-image: url(%1/Resource/common/cancel.png); "
          "}"
          "QPushButton#ActionBtn:hover { border-image: "
          "url(%1/Resource/common/ok.png); }")
          .arg(qApp->applicationDirPath())
          .arg(getFontSize(12));

  this->setStyleSheet(baseStyle);

  // 3. 布局构建
  QVBoxLayout* mainVLayout = new QVBoxLayout(this);
  mainVLayout->setContentsMargins(0, 0, 0, 0);
  mainVLayout->setSpacing(0);

  // 外层容器（用于画边框）
  QWidget* container = new QWidget(this);
  container->setObjectName("MainWindow");
  QVBoxLayout* contentLayout = new QVBoxLayout(container);
  mainVLayout->addWidget(container);

  // --- A. 标题栏 ---
  QHBoxLayout* titleLayout = new QHBoxLayout();
  QLabel* titleLabel = new QLabel(style == 1 ? u8" 警告 " : u8" 提示 ");
  titleLabel->setStyleSheet(
      QString("font-size: %1pt; font-weight: bold;").arg(getFontSize(12)));

  QPushButton* closeBtn = new QPushButton();
  closeBtn->setFixedSize(18, 18);
  closeBtn->setStyleSheet(
      QString("border-image: url(%1/Resource/common/close.png);")
          .arg(qApp->applicationDirPath()));
  connect(closeBtn, &QPushButton::clicked, this, &CustomDialog::reject);

  titleLayout->addWidget(titleLabel);
  titleLayout->addStretch();
  titleLayout->addWidget(closeBtn);
  contentLayout->addLayout(titleLayout);

  // --- B. 中间内容区 (图标 + 文字) ---
  QHBoxLayout* bodyLayout = new QHBoxLayout();
  bodyLayout->setContentsMargins(20, 10, 20, 10);
  bodyLayout->setSpacing(15);

  QLabel* iconLabel = new QLabel();
  iconLabel->setFixedSize(32, 32);
  iconLabel->setStyleSheet(
      QString("border-image: url(%1);").arg(getIconPath(style)));

  QLabel* textLabel = new QLabel(text);
  textLabel->setWordWrap(true);
  textLabel->setAlignment(Qt::AlignCenter);  // 核心：文字真正居中
  textLabel->setStyleSheet("font-size: 14px;");


  bodyLayout->addWidget(iconLabel);
  bodyLayout->addWidget(textLabel, 1);  // 权重1，占用剩余空间
  bodyLayout->addSpacing(32);
  contentLayout->addLayout(bodyLayout);

  // --- C. 底部按钮区 ---
  QHBoxLayout* btnLayout = new QHBoxLayout();
  btnLayout->setContentsMargins(0, 0, 0, 15);
  btnLayout->setSpacing(20);
  btnLayout->addStretch();

  QPushButton* okBtn = new QPushButton(u8"确定");
  okBtn->setObjectName("ActionBtn");
  okBtn->setFixedSize(85, 30);
  connect(okBtn, &QPushButton::clicked, this, &CustomDialog::accept);
  btnLayout->addWidget(okBtn);

  if (style == 3) {  // 只有询问模式才显示取消
    QPushButton* cancelBtn = new QPushButton(u8"取消");
    cancelBtn->setObjectName("ActionBtn");
    cancelBtn->setFixedSize(85, 30);
    connect(cancelBtn, &QPushButton::clicked, this, &CustomDialog::reject);
    btnLayout->addWidget(cancelBtn);
  }
  btnLayout->addStretch();
  contentLayout->addLayout(btnLayout);
}

QString CustomDialog::getIconPath(int style) {
  QString path = qApp->applicationDirPath() + "/Resource/common/";
  if (style == 1)
    path += "warning.png";
  else if (style == 2)
    path += "info.png";
  else if (style == 3)
    path += "question.png";
  else
    path += "error.png";
  return path;
}

// --- 窗口拖动实现 ---
void CustomDialog::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_isPressed = true;
    m_movePoint = event->pos();
  }
}
void CustomDialog::mouseMoveEvent(QMouseEvent* event) {
  if (m_isPressed) {
    move(event->globalPos() - m_movePoint);
  }
}
void CustomDialog::mouseReleaseEvent(QMouseEvent*) { m_isPressed = false; }