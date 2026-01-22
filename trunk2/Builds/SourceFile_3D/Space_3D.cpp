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
}

Space_3D::~Space_3D() {
  if (m_earthWindow) {
    m_earthWindow->close();
    delete m_earthWindow;
    m_earthWindow = nullptr;
  }
}

void Space_3D::handleCreateProject() {
  m_earthWindow->resetScene();
  m_earthWindow->onReShow();
  m_earthWindow->activateWindow();  // 确保窗口在最前面
}

void Space_3D::executeBatchDelete() {}

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
  QSqlQuery query;

  // 1. 创建项目主表
  QString createProjectTable =
      "CREATE TABLE IF NOT EXISTS t_project ("
      "p_name TEXT PRIMARY KEY, "
      "create_time DATETIME, "
      "modify_time DATETIME)";
  if (!query.exec(createProjectTable)) {
    qDebug() << "创建主表失败:" << query.lastError().text();
  }

  // 2. 创建资源路径表
  QString createAssetsTable =
      "CREATE TABLE IF NOT EXISTS t_project_assets ("
      "id INTEGER PRIMARY KEY AUTOINCREMENT, "
      "p_name TEXT, "
      "file_path TEXT, "
      "FOREIGN KEY(p_name) REFERENCES t_project(p_name) ON DELETE CASCADE)";
  if (!query.exec(createAssetsTable)) {
    qDebug() << "创建辅表失败:" << query.lastError().text();
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

    Space_3D_project_item* itemWidget = new Space_3D_project_item(
        pro_name, create_time, modify_time, _scrWidth);
    connect(itemWidget, &Space_3D_project_item::deleteRequested, this,
            &Space_3D::onDeleteProject);
    connect(itemWidget, &Space_3D_project_item::loadRequested, this,
            &Space_3D::onLoadProject);
    itemWidget->resize(QSize(_scrWidth - 25, 100));



    // 3. 将自定义 Widget 包装进 QListWidgetItem
    QListWidgetItem* item = new QListWidgetItem(ui.projectList);
    item->setSizeHint(QSize(_scrWidth - 205, 100));

    // 4. 关键：关联 QListWidget 和你的自定义界面
    ui.projectList->addItem(item);
    ui.projectList->setItemWidget(item, itemWidget);
  }
}

void Space_3D::onDeleteProject(QString pName) {
  // 1. 弹窗确认，防止误删
  auto result = QMessageBox::question(
      this, QStringLiteral("确认删除"),
      QStringLiteral("确定要删除项目 [%1] 吗？数据将无法恢复。").arg(pName));

  if (result != QMessageBox::Yes) return;

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
  // 1. 获取数据库中的路径
  QSqlQuery query;
  query.prepare("SELECT file_path FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);

  if (!query.exec()) {
    QMessageBox::warning(this, "Error", "Failed to query project assets.");
    return;
  }

  // 2. 准备加载，先把当前的 OSGEarthApp 窗口切出来
  // 假设 m_osgApp 是你保存在 Space_3D 里的 OSGEarthApp 实例指针
  if (!m_earthWindow) return;

  // 这里的逻辑：显示 OSG 窗口，并可以设置项目名称到它的输入框
  m_earthWindow->onReShow();

  // 3. 循环加载文件
  int loadCount = 0;
  while (query.next()) {
    QString path = query.value(0).toString();
    QFileInfo info(path);
    QString suffix = info.suffix().toLower();

    // 根据后缀分发给 OSGEarthApp 的对应函数
    if (suffix == "las") {
      m_earthWindow->onSlotLoadLas(path);
    } else if (suffix == "tif" || suffix == "img") {
      // 注意：这里可能需要区分是 Elevation 还是 Imagery
      // 简单处理可以先默认作为影像加载，或者根据你数据库的设计增加类型字段
      m_earthWindow->onSlotTif(path);
    } else if (suffix == "shp") {
      m_earthWindow->onSlotShp(path);
    } else if (suffix == "obj") {
      m_earthWindow->onSlotLoadObj(path);
    }
    loadCount++;
  }
}