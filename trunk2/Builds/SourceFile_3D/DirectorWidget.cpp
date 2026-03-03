#include "DirectorWidget.h"

#include <QFileInfo>
#include <iostream>
#include <osg/Material>
#include "CustomDialog.h"
#include "NoticeToast.h"

DirectorWidget::DirectorWidget(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  setWindowTitle(u8"场景管理器");
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowTitleHint |
                 Qt::CustomizeWindowHint);

  m_playTimer = new QTimer(this);
  connect(m_playTimer, &QTimer::timeout, this,
          &DirectorWidget::executeNextStep);

  ui.table_script->horizontalHeader()->setSectionResizeMode(
      QHeaderView::ResizeToContents);
  ui.table_script->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::Stretch);
  ui.table_script->setMinimumWidth(300);

  this->hide();
}

DirectorWidget::~DirectorWidget() {}

void DirectorWidget::on_btnAdd_clicked() {
  QListWidgetItem* currentItem = ui.list_assets->currentItem();
  if (!currentItem) return;

  // 1. 获取“艺名”（显示出来的文字）和“真名”（UserRole 里的隐藏路径）
  QString displayName = currentItem->text();
  QString realKey = currentItem->data(Qt::UserRole).toString();

  int currentRow = ui.table_script->rowCount();
  ui.table_script->insertRow(currentRow);

  // 2. 设置“物体名称”列 (第0列)
  QTableWidgetItem* nameItem = new QTableWidgetItem(displayName);

  // --- 核心：把真名藏在 UserRole 里，用户看不见，代码能取到 ---
  nameItem->setData(Qt::UserRole, realKey);

  ui.table_script->setItem(currentRow, 0, nameItem);

  // 3. 设置“动作选择”列 (第1列)
  QComboBox* combo = new QComboBox();
  // 使用 QStringLiteral 避免乱码
  combo->addItems({QStringLiteral("建造"), QStringLiteral("拆除")});
  ui.table_script->setCellWidget(currentRow, 1, combo);
}

void DirectorWidget::refreshAssetList(
    const QMap<QString, osg::ref_ptr<osg::Object>>& pathNodeMap) {
  ui.list_assets->clear();  // 先清空旧的

  for (auto it = pathNodeMap.begin(); it != pathNodeMap.end(); ++it) {
    QString fullPath = it.key();

    // 提取文件名 (例如: "C:/models/tower.obj" -> "tower.obj")
    QString fileName = QFileInfo(fullPath).fileName();

    QListWidgetItem* item = new QListWidgetItem(fileName);

    // --- 核心操作：把全路径/真名藏起来 ---
    item->setData(Qt::UserRole, fullPath);

    // 设置图标等
    if (fullPath.endsWith(".line")) {
      item->setIcon(style()->standardIcon(QStyle::SP_DriveNetIcon));
    } else {
      item->setIcon(style()->standardIcon(QStyle::SP_FileIcon));
    }

    ui.list_assets->addItem(item);
  }

  m_pathNodeMap = pathNodeMap;
}

void DirectorWidget::on_btnPlay_clicked() {
  std::cout << "[DirectorWidget] Play button clicked, total steps: "
            << ui.table_script->rowCount() << std::endl;
  if (ui.table_script->rowCount() == 0) return;

  // 1. 隐藏导演界面，让出视野
  this->hide();

  // 2. 初始化：所有参与剧本的物体先全部隐藏
  for (int i = 0; i < ui.table_script->rowCount(); ++i) {
    QString assetId =
        ui.table_script->item(i, 0)->data(Qt::UserRole).toString();
    if (m_pathNodeMap.contains(assetId)) {
      osg::Node* node = dynamic_cast<osg::Node*>(m_pathNodeMap[assetId].get());
      if (node) node->setNodeMask(0x0);  // 全部初始隐藏
    }
  }

  m_currentStepIndex = 0;

  QTimer::singleShot(1000, this, [this]() { executeNextStep(); });
}

void DirectorWidget::executeNextStep() {
  if (m_currentStepIndex >= ui.table_script->rowCount()) {
    m_playTimer->stop();
    this->show();  // 播完了，导演重新回来
    return;
  }

  QString assetId = ui.table_script->item(m_currentStepIndex, 0)
                        ->data(Qt::UserRole)
                        .toString();
  QComboBox* cb = qobject_cast<QComboBox*>(
      ui.table_script->cellWidget(m_currentStepIndex, 1));
  bool isShow = (cb->currentText() == QStringLiteral("建造"));

  if (m_pathNodeMap.contains(assetId)) {
    osg::Node* node = dynamic_cast<osg::Node*>(m_pathNodeMap[assetId].get());
    if (node) {
      if (isShow) {
        // --- 显示逻辑：先出现，再高亮 0.5 秒 ---
        node->setNodeMask(0xffffffff);
        highlightNode(node, true);
        QTimer::singleShot(500, [=]() { highlightNode(node, false); });
      } else {
        // --- 隐藏逻辑：先闪烁 3 次，再消失 ---
        flashNode(node, 6, 100, [=]() { node->setNodeMask(0x0); });
      }
    }
  }

  m_currentStepIndex++;
  int nextTimeout = 1000;
  m_playTimer->start(nextTimeout);
}

void DirectorWidget::highlightNode(osg::Node* node, bool on) {
  osg::StateSet* ss = node->getOrCreateStateSet();
  if (on) {
    osg::ref_ptr<osg::Material> mat = new osg::Material;
    mat->setEmission(osg::Material::FRONT,
                     osg::Vec4(1.0, 0.8, 0.0, 1.0));  // 金色发光
    ss->setAttributeAndModes(
        mat, osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
  } else {
    ss->removeAttribute(osg::StateAttribute::MATERIAL);  // 移除特效
  }
}

void DirectorWidget::flashNode(osg::Node* node, int times, int intervalMs,
                               std::function<void()> callback) {
  if (times <= 0) {
    if (callback) callback();
    return;
  }
  // 切换状态
  node->setNodeMask(node->getNodeMask() == 0x0 ? 0xffffffff : 0x0);

  QTimer::singleShot(
      intervalMs, [=]() { flashNode(node, times - 1, intervalMs, callback); });
}

void DirectorWidget::on_btnExit_clicked() { this->hide(); }

void DirectorWidget::on_btnReset_clicker()
{

  CustomDialog dlg(3, QString(u8"确认要删除吗？"), this);
  if (dlg.exec() == QDialog::Rejected) return;

  ui.table_script->setRowCount(0);
  NoticeToast::popup(this, u8"脚本已重置", 1000);
}