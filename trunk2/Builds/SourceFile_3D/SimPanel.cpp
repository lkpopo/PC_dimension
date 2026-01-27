#include "SimPanel.h"

SimPanel::SimPanel(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  initLayout();

  m_dirGroup = new QButtonGroup(this);
  m_dirGroup->addButton(ui.btnFront, 0);  // 假设 0 是 FRONT
  m_dirGroup->addButton(ui.btnBack, 1);   // 1 是 BACK
  m_dirGroup->addButton(ui.btnLeft, 2);   // 2 是 LEFT
  m_dirGroup->addButton(ui.btnRight, 3);  // 3 是 RIGHT
  m_dirGroup->setExclusive(true);         // 互斥，只能选一个

  // 2. 绑定按钮点击事件
  connect(m_dirGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
          [this](int id) {
            m_selectedDir = id;
          });
  // 初始状态：确保在第一页
  ui.stackedWidget->setCurrentIndex(0);
}

SimPanel::~SimPanel() {}

void SimPanel::on_btnGoToSelect_clicked() {
  // 1. 停止拖拽器
  // if (m_manip) m_manip->detach();

  //// 2. 翻页
  // ui.stackedWidget->setCurrentIndex(1);
}

void SimPanel::on_btnGoToSim_clicked() {
  // ui.stackedWidget->setCurrentIndex(2);

  //// 3. 启动动画：从 0度 变到 90度
  // QVariantAnimation* anim = new QVariantAnimation(this);
  // anim->setDuration(2500);  // 2.5秒倒塌
  // anim->setStartValue(0.0f);
  // anim->setEndValue(90.0f);
  // anim->setEasingCurve(QEasingCurve::InQuad);  // 物理特性：越倒越快

  // connect(anim, &QVariantAnimation::valueChanged,
  //         [this](const QVariant& value) {
  //           if (m_tower)
  //             m_tower->updateFall((TowerController::Direction)m_direction,
  //                                 value.toFloat());
  //           ui.progressBar->setValue((value.toFloat() / 90.0) * 100);
  //         });
  // anim->start();
}

void SimPanel::on_btnExit_clicked() {}

void SimPanel::updateAnimation(float value) {}

void SimPanel::initLayout() {
  // --- 1. 面板整体设置 ---
  this->setFixedSize(300, 450);  // 固定大小，防止自由布局变形
  this->setWindowTitle(u8"倒塔模拟控制面板");

  // 让 stackedWidget 撑满整个面板
  ui.stackedWidget->setGeometry(0, 0, 300, 450);

  // --- 2. Page 0: 姿态调整页 ---
  ui.lblTip->setGeometry(20, 40, 260, 60);
  ui.lblTip->setWordWrap(true);
  ui.lblTip->setText(
      u8"<b>步骤 "
      u8"1：姿态调整</b><br>请在场景中拖拽手柄，确定铁塔的初始位置和偏航角。");

  ui.btnGoToSelect->setGeometry(50, 350, 200, 40);
  ui.btnGoToSelect->setText(u8"锁定姿态并继续");

  // --- 3. Page 1: 方向选择页 ---
  ui.lblTip_2->setGeometry(20, 30, 260, 50);
  ui.lblTip_2->setText(u8"<b>步骤 2：选择倒塌方向</b>");

  // 假设你的 gridLayout 放在一个容器 widget 里，或者直接设置按钮
  // 布局四个方向按钮（呈十字形排列）
  int btnW = 70, btnH = 40;
  int centerX = 150, centerY = 200;
  ui.btnFront->setGeometry(centerX - btnW / 2, centerY - 60, btnW,
                           btnH);                                         // 上
  ui.btnBack->setGeometry(centerX - btnW / 2, centerY + 20, btnW, btnH);  // 下
  ui.btnLeft->setGeometry(centerX - 110, centerY - 20, btnW, btnH);       // 左
  ui.btnRight->setGeometry(centerX + 40, centerY - 20, btnW, btnH);       // 右

  ui.btnGoToSim->setGeometry(50, 350, 200, 40);
  ui.btnGoToSim->setText(u8"开始执行模拟");

  // --- 4. Page 2: 模拟展示页 ---
  ui.lblTip_3->setGeometry(20, 100, 260, 80);
  ui.lblTip_3->setAlignment(Qt::AlignCenter);

  ui.btnExit->setGeometry(50, 350, 200, 40);
  ui.btnExit->setText(u8"结束并退出");
}