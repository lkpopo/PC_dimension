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
  m_earthWindow->resetForNewProject();
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
  cout << "Loading Project: " << pName.toStdString() << endl;

  // 1. 从数据库查询该项目所有的文件路径
  QSqlQuery query;
  query.prepare("SELECT file_path FROM t_project_assets WHERE p_name = ?");
  query.addBindValue(pName);

  if (!query.exec()) return;

  QStringList filesToLoad;
  while (query.next()) {
    filesToLoad << query.value(0).toString();
  }

  if (filesToLoad.isEmpty()) {
    QMessageBox::warning(this, QStringLiteral("提示"),
                         QStringLiteral("该项目没有任何关联文件。"));
    return;
  }

  // 2. 调用你的 OSG 加载逻辑
  // 假设你有一个全局的加载函数或者你可以访问 OSGEarthApp 的实例
  for (const QString& path : filesToLoad) {
    // 这里根据后缀名调用你之前的加载代码
    if (path.endsWith(".las", Qt::CaseInsensitive)) {
      // 加载点云...
      // m_osgApp->addPointCloud(path);
    } else if (path.endsWith(".tif", Qt::CaseInsensitive) ||
               path.endsWith(".img", Qt::CaseInsensitive)) {
      // 加载影像...
      // m_osgApp->addImageLayer(path);
    }
  }

  QMessageBox::information(
      this, QStringLiteral("加载成功"),
      QStringLiteral("已成功加载项目 [%1] 的所有图层。").arg(pName));
}