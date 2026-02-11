#include "ScenarioDirectorWidget.h"

#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QUuid>
#include <QSqlError>
#include "NoticeToast.h"

#include<iostream>

ScenarioDirectorWidget::ScenarioDirectorWidget(QWidget* parent)
    : m_parent(parent) {
  ui.setupUi(this);

  setWindowTitle(u8"场景管理器");
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowTitleHint |
                 Qt::CustomizeWindowHint);

  this->hide();
}

ScenarioDirectorWidget::~ScenarioDirectorWidget() {}

void ScenarioDirectorWidget::on_btnExit_clicked() { this->hide(); }

void ScenarioDirectorWidget::on_btnAddIn_clicked() { handleModelAction(ADD); }

void ScenarioDirectorWidget::on_btnAddOut_clicked() {
  handleModelAction(REMOVE);
}

void ScenarioDirectorWidget::on_btnSaveScenario_clicked() {
  QString scenarioName = ui.lineEdit->text();
  if (scenarioName.isEmpty()) {
      return;
  }

  QSqlDatabase db = QSqlDatabase::database();
  if (!db.transaction()) {
    return;
  }

  QSqlQuery query;

  // 1. 【覆盖式第一步】删除该剧本名称下的所有旧步骤
  query.prepare("DELETE FROM t_scenario_steps WHERE scenario_name = :name");
  query.bindValue(":name", scenarioName);
  if (!query.exec()) {
    db.rollback();
    return;
  }

  // 2. 【覆盖式第二步】循环插入当前列表中的所有步骤
  query.prepare(
      "INSERT INTO t_scenario_steps (scenario_name, step_index, file_path, "
      "action_type, lon, lat, alt) "
      "VALUES (:name, :idx, :path, :type, :lon, :lat, :alt)");

  bool success = true;
  for (int i = 0; i < m_currentWorkflow.size(); ++i) {
    const auto& step = m_currentWorkflow.at(i);

    query.bindValue(":name", scenarioName);
    query.bindValue(":idx", i + 1);  // 保持 UI 上的 1, 2, 3 顺序
    query.bindValue(":path", step.path);
    query.bindValue(":type", static_cast<int>(step.actionType));

    // 逻辑：只有 obj 记录坐标，其余默认为 0
    if (QFileInfo(step.path).suffix().toLower() == "obj") {
      query.bindValue(":lon", step.longitude);
      query.bindValue(":lat", step.latitude);
      query.bindValue(":alt", step.altitude);
    } else {
      query.bindValue(":lon", 0.0);
      query.bindValue(":lat", 0.0);
      query.bindValue(":alt", 0.0);
    }

    if (!query.exec()) {
      success = false;
      break;
    }
  }

  // 3. 【覆盖式第三步】提交事务
  if (success) {
    db.commit();
    NoticeToast::popup(m_parent, u8"剧本保存成功", 1500);
  } else {
    db.rollback();
    NoticeToast::popup(m_parent, u8"保存失败: " + query.lastError().text(),
                       2000);
  }
}

void ScenarioDirectorWidget::handleModelAction(const ActionType& type) {
  QString path = QFileDialog::getOpenFileName(
      this, "选择模型", "", "(*.las *.tif *.img *.shp *.obj)");
  if (path.isEmpty()) return;

  // 2. 逻辑校验
  if (type == REMOVE && !m_aliveModelIds.contains(path)) {
    NoticeToast::popup(this, u8"该模型实例不存在，无法删除。", 1000);
    return;
  }

  if (type == ADD && m_aliveModelIds.contains(path)) {
    NoticeToast::popup(this, u8"该模型实例已存在，无法重复添加。", 1000);
    return;
  }

  // 3. 构造剧本步骤
  ScenarioStep step;
  step.path = path;
  step.actionType = type;
  m_currentWorkflow.append(step);

  // 4. 存在的模型 ID 列表
  if (type == ADD)
    m_aliveModelIds.insert(path);
  else if (type == REMOVE)
    m_aliveModelIds.remove(path);

  // 5. 创建 Item
  ScenarioItemWidget::Mode mode = (type == ADD) ? ScenarioItemWidget::MODE_IN
                                                : ScenarioItemWidget::MODE_OUT;

  int nextIndex = ui.listScenario->count() + 1;
  QListWidgetItem* item = new QListWidgetItem(ui.listScenario);
  item->setSizeHint(QSize(0, 45));
  // 创建自定义 Widget (编辑模式)
  ScenarioItemWidget* widget =
      new ScenarioItemWidget(nextIndex, QFileInfo(path).fileName(), mode, this);
  connect(widget, &ScenarioItemWidget::signalDelete, this,
          &ScenarioDirectorWidget::onItemDeleteRequested);
  ui.listScenario->setItemWidget(item, widget);

  return;
}

void ScenarioDirectorWidget::onItemDeleteRequested(ScenarioItemWidget* ptr) {
  if (!ptr) return;

  // 1. 遍历列表，定位指针所在的行
  for (int i = 0; i < ui.listScenario->count(); ++i) {
    QListWidgetItem* item = ui.listScenario->item(i);
    if (ui.listScenario->itemWidget(item) == ptr) {
      if (i < m_currentWorkflow.size()) {
        m_aliveModelIds.remove(m_currentWorkflow.at(i).path);
        m_currentWorkflow.removeAt(i);
      }

      QListWidgetItem* targetItem = ui.listScenario->takeItem(i);
      delete targetItem;
      refreshListIndices();

      break;
    }
  }
}

void ScenarioDirectorWidget::refreshListIndices() {
  for (int i = 0; i < ui.listScenario->count(); ++i) {
    QListWidgetItem* item = ui.listScenario->item(i);
    ScenarioItemWidget* widget =
        qobject_cast<ScenarioItemWidget*>(ui.listScenario->itemWidget(item));
    if (widget) {
      widget->setIndex(i + 1);
    }
  }
}