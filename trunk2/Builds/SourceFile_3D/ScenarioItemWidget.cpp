#include "ScenarioItemWidget.h"

#include <QButtonGroup>

ScenarioItemWidget::ScenarioItemWidget(int index, QString name, Mode mode,
                                       QWidget* parent)
    : QWidget(parent), m_name(name), m_mode(mode) {
  ui.setupUi(this);
  ui.lblText->setText(m_name);
  ui.lblIndex->setText(QString::number(index));

  ui.btnIn->hide();
  ui.btnOut->hide();
  ui.btnPlay->hide();

  // 2. 根据模式显示对应的“状态标签”（虽然是按钮，但仅做显示）
  switch (m_mode) {
    case MODE_IN:
      ui.btnIn->show();
      break;
    case MODE_OUT:
      ui.btnOut->show();
      break;
    case PLAY:
      ui.btnPlay->show();
      break;
    case MODE_HISTORY:
      // 历史记录模式，默认显示“回放”按钮
      ui.btnPlay->show();
      ui.btnPlay->setText(u8"查看");
      break;
  }

  connect(ui.btnDel, &QPushButton::clicked, this,
          [=]() { emit signalDelete(this); });
  connect(ui.btnPlay, &QPushButton::clicked, this,
          [=]() { emit signalPlayScenario(index); });
}
void ScenarioItemWidget::setIndex(int index) {
  ui.lblIndex->setText(QString::number(index));
}

ScenarioItemWidget::~ScenarioItemWidget() {}
