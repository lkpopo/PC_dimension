#pragma once

#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

#include "ui_ScenarioItemWidget.h"

class ScenarioItemWidget : public QWidget {
  Q_OBJECT

 public:
  enum Mode { MODE_IN, MODE_OUT, PLAY };

  ScenarioItemWidget(int index, QString name, Mode mode, QWidget *parent = nullptr);
  ~ScenarioItemWidget();
  void setIndex(int index);
 
signals:
  // 回放模式信号
  void signalPlayScenario(int scenarioId);
  void signalDelete(ScenarioItemWidget* ptr);

 private:
  Ui::ScenarioItemWidgetClass ui;
  Mode m_mode;
  QString m_name;
};
