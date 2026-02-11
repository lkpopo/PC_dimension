#include "Space_3D.h"


#include "Space_3D_project_item.h"


Space_3D::Space_3D(QWidget* parent) : QWidget(parent) {
  ui.setupUi(this);

  setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint);
  setAttribute(Qt::WA_TranslucentBackground, true);
  setAttribute(Qt::WA_QuitOnClose, true);

  initLayout();    // 初始化布局
  initDatabase();  // 初始化数据库表
  // 预加载地球
  if (!m_earthWindow) {
    m_earthWindow = new OSGEarthApp();
    m_earthWindow->applyFullLayout(_scrWidth, _scrHeight);
    m_earthWindow->hide();
  }
  initSignalsAndSlots();  // 处理信号槽

  m_noticeToast = new NoticeToast(this);
}

Space_3D::~Space_3D() {
  if (m_earthWindow) {
    m_earthWindow->close();
    delete m_earthWindow;
    m_earthWindow = nullptr;
  }
}

void Space_3D::handleCreateProject() {
  if (!m_earthWindow || !m_earthWindow->getEarthInitStatus()) {
    NoticeToast::popup(this, u8"地球尚未初始化完成，请稍后再试。", 2000);
    return;
  }
  m_earthWindow->resetScene();
  m_earthWindow->setWorkMode(OSGEarthApp::WorkMode::NewMode);
  m_earthWindow->onReShow();
  m_earthWindow->activateWindow();  // 确保窗口在最前面
}

void Space_3D::executeBatchDelete() {
  // 1. 搜集所有勾选的项目名
  QStringList namesToDelete;
  for (int i = 0; i < ui.projectList->count(); ++i) {
    QListWidgetItem* item = ui.projectList->item(i);
    auto* widget =
        qobject_cast<Space_3D_project_item*>(ui.projectList->itemWidget(item));

    if (widget && widget->isCheckBoxChecked()) {
      // 从刚刚存的 UserRole 里取回项目名
      namesToDelete << item->data(Qt::UserRole).toString();
    }
  }

  if (namesToDelete.isEmpty()) {
    NoticeToast::popup(this, u8"尚未勾选任何项目", 2000);
    return;
  }

  // 2. 确认弹窗

  CustomDialog dlg(
      3, QString(u8"确定要删除选中的 %1 个项目吗？").arg(namesToDelete.size()),
      this);
  if (dlg.exec() == QDialog::Rejected) return;

  // 3. 执行删除逻辑
  QSqlDatabase db = QSqlDatabase::database();
  db.transaction();  // 开启事务

  try {
    QSqlQuery query;
    for (const QString& name : namesToDelete) {
      // A. 删除子表记录
      query.prepare("DELETE FROM t_project_assets WHERE p_name = ?");
      query.addBindValue(name);
      query.exec();

      // B. 删除主表记录
      query.prepare("DELETE FROM t_project WHERE p_name = ?");
      query.addBindValue(name);
      query.exec();
    }

    if (db.commit()) {
      NoticeToast::popup(this, u8"批量删除成功", 2000);
      refreshProjectList();  // 刷新 UI
    }
  } catch (...) {
    db.rollback();
    QMessageBox::critical(this, u8"错误", u8"数据库操作失败，已回滚");
  }
}

void Space_3D::applySort() {}

void Space_3D::initLayout() {
  QRect screenRect = QGuiApplication::primaryScreen()->availableGeometry();
  _scrHeight = screenRect.height();
  _scrWidth = screenRect.width();
  resize(_scrWidth, _scrHeight);
  ui.projectList->setSpacing(10);

  ui.label_bg->resize(_scrWidth, _scrHeight);

  int offset = 100;  // 向左移动的距离
  ui.btnNewProject->move(_scrWidth - 120 * 2 - 100 - offset, 20);
  ui.btnBatchDelete->move(_scrWidth - 120 - 100 - offset, 20);
  ui.btnSortCategory->move(_scrWidth - 100 - offset, 20);
  ui.projectList->move(0, 69);
  ui.projectList->resize(_scrWidth - 80, _scrHeight - 70);
}

void Space_3D::initSignalsAndSlots() {
  connect(ui.btnNewProject, &QPushButton::clicked, this,
          &Space_3D::handleCreateProject);
  connect(ui.btnBatchDelete, &QPushButton::clicked, this,
          &Space_3D::executeBatchDelete);
  connect(ui.btnSortCategory, &QPushButton::clicked, this,
          &Space_3D::applySort);
  connect(m_earthWindow, &OSGEarthApp::projectSavedSuccess, this,
          &Space_3D::refreshProjectList);
}

void Space_3D::initDatabase() {
  if (!ProjectDataManager::initDatabase()) {
    qDebug() << "Database Error:" << ProjectDataManager::lastError();
  }
}

void Space_3D::refreshProjectList() {
  ui.projectList->clear();  // 1. 先清空旧列表

  QSqlQuery query(
      "SELECT p_name, create_time, modify_time  FROM t_project ORDER BY "
      "modify_time DESC");

  while (query.next()) {
    QString pro_name = query.value(0).toString();
    QString create_time = query.value(1).toString();
    QString modify_time = query.value(2).toString();
    QString icon_path =
        QString(
            "%1/Resource/common/erath_icon.png")
            .arg(QApplication::applicationDirPath());

    Space_3D_project_item* itemWidget = new Space_3D_project_item(
        pro_name, create_time, modify_time, icon_path, _scrWidth);
    connect(itemWidget, &Space_3D_project_item::deleteRequested, this,
            &Space_3D::onDeleteProject);
    connect(itemWidget, &Space_3D_project_item::loadRequested, this,
            &Space_3D::onLoadProject);
    connect(itemWidget, &Space_3D_project_item::BrowseRequested, this,
            &Space_3D::onBrowseProject);
    itemWidget->resize(QSize(_scrWidth - 25, 100));

    // 3. 将自定义 Widget 包装进 QListWidgetItem
    QListWidgetItem* item = new QListWidgetItem(ui.projectList);
    item->setSizeHint(QSize(_scrWidth - 205, 100));
    item->setData(Qt::UserRole, pro_name);

    // 4. 关键：关联 QListWidget 和你的自定义界面
    ui.projectList->addItem(item);
    ui.projectList->setItemWidget(item, itemWidget);
  }
}

void Space_3D::onDeleteProject(QString pName) {
  CustomDialog dlg(3, QString(u8"确认要删除吗？"), this);
  if (dlg.exec() == QDialog::Rejected) return;

  // 2. 数据库操作
  QSqlDatabase db = QSqlDatabase::database();
  if (!db.transaction()) return;

  QSqlQuery query(db);
  // A. 先删资源路径表
  query.prepare("DELETE FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);
  query.exec();

  // B. 再删主表
  query.prepare("DELETE FROM t_project WHERE p_name = ?");
  query.addBindValue(pName);

  if (query.exec()) {
    db.commit();
    cout << "Deleted project: " << pName.toStdString() << endl;
    refreshProjectList();  // 3. 刷新界面
  } else {
    db.rollback();
    QMessageBox::critical(this, QStringLiteral("错误"),
                          QStringLiteral("删除失败！"));
  }
}

void Space_3D::onLoadProject(QString pName) {
  loadProjectCommon(pName, OSGEarthApp::WorkMode::EditMode);
}

void Space_3D::onBrowseProject(QString pName) {
  loadProjectCommon(pName, OSGEarthApp::WorkMode::BrowseMode);
}

void Space_3D::loadProjectCommon(QString pName, OSGEarthApp::WorkMode mode) {
  if (!m_earthWindow || !m_earthWindow->getEarthInitStatus()) {
    NoticeToast::popup(this, u8"地球尚未初始化完成，请稍后再试。", 2000);
    return;
  }

  m_earthWindow->setWorkMode(mode, pName);

  // 使用管理器获取数据
  QList<AssetRecord> assets = ProjectDataManager::getProjectAssets(pName);

  for (const auto& asset : assets) {
    m_earthWindow->onLoadScene(asset.filePath, asset.lon, asset.lat, asset.alt);
  }

  m_earthWindow->updateUIByWorkMode();
  m_earthWindow->onReShow();
}