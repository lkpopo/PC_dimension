#pragma once

#include <QWidget>
#include <QButtonGroup>
#include "ui_SimPanel.h"
#include "Controller.h"
#include "ManipulatorHelper.h"

class SimPanel : public QWidget {
  Q_OBJECT
 public:
  explicit SimPanel(QWidget* parent = nullptr);
  ~SimPanel();

  // 关键：把操作对象“借”给 Panel
  void init(Controller* controller, ManipulatorHelper* manip);

 signals:
  void signalFinished();  // 告诉主界面：用户玩完了，可以关了

 private slots:
  void on_btnGoToSelect_clicked();
  void on_btnGoToSim_clicked();
  void on_btnExit_clicked();
  void updateAnimation(float value);

 private:
  void initLayout();

  Ui::SimPanelClass ui;

  QButtonGroup* m_dirGroup;
  int m_selectedDir;

  Controller* m_tower;
  ManipulatorHelper* m_manip;
  int m_direction = 0;  // 记录选中的方向
};