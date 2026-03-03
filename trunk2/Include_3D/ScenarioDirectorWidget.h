#pragma once

#include <QWidget>

#include "ScenarioItemWidget.h"
#include "ui_ScenarioDirectorWidget.h"

class ScenarioDirectorWidget : public QWidget {
  Q_OBJECT
  enum ActionType { ADD, REMOVE };

  // 定义剧本动作结构
  struct ScenarioStep {
    QString path;
    ActionType actionType;

    double longitude = 0.0;
    double latitude = 0.0;
    double altitude = 0.0;
  };

 public:
  ScenarioDirectorWidget(QWidget* parent = nullptr);
  ~ScenarioDirectorWidget();
 private slots:
  void on_btnExit_clicked();
  void on_btnAddIn_clicked();
  void on_btnAddOut_clicked();
  void on_btnLoadHistory_clicked();
  void on_btnSaveScenario_clicked();
  void onItemDeleteRequested(ScenarioItemWidget* ptr);
  void onScenarioDeleteRequested(ScenarioItemWidget* ptr);
  //void onPlayScenarioRequested(int idx);

  void handleModelAction(const ActionType& type);
  void refreshListIndices();

 private:
  Ui::ScenarioDirectorWidgetClass ui;
  QWidget* m_parent;

  QList<ScenarioStep> m_currentWorkflow;
  QSet<QString> m_aliveModelIds;
};
