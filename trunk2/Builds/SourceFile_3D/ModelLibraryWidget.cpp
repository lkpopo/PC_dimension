#include "ModelLibraryWidget.h"

#include <QDir>
#include <QFileInfo>
#include <QIcon>
#include <QScreen>
#include <iostream>
#include <QFileDialog>

ModelLibraryWidget::ModelLibraryWidget(QWidget* parent)
    : QWidget(parent){
  ui.setupUi(this);

  setWindowTitle(u8"3D 模型库");
  setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint | Qt::WindowTitleHint |
                 Qt::CustomizeWindowHint);
  centerOnScreen();

  // 1. 自动执行扫描加载
  scanDirectory();

  // 2. 绑定信号槽
  connect(ui.btnRefresh, &QPushButton::clicked, this,
          &ModelLibraryWidget::onRefresh);
  connect(ui.searchLineEdit, &QLineEdit::textChanged, this,
          &ModelLibraryWidget::onSearch);
  connect(ui.modelList, &QListWidget::itemDoubleClicked, this,
          &ModelLibraryWidget::onItemDoubleClicked);
  connect(ui.btnExit, &QPushButton::clicked, this,
          &ModelLibraryWidget::onExitClicked);
  connect(ui.btnChoosePath, &QPushButton::clicked, this,
          &ModelLibraryWidget::onChoosePath);
  this->hide();
}

ModelLibraryWidget::~ModelLibraryWidget() {}

void ModelLibraryWidget::scanDirectory() {
  ui.modelList->clear();

  // 1. 路径处理（包含中文兼容）
  QString safePath = QDir::cleanPath(m_path);
  QDir rootDir(safePath);
  if (!rootDir.exists()) {
    rootDir = QDir(QString::fromLocal8Bit(m_path.toLocal8Bit()));
  }

  if (!rootDir.exists()) {
    ui.lblStatus->setText(tr(u8"当前路径无效"));
    return;
  }

  QIcon defaultIcon(QString("%1/Resource/common/model.png")
                        .arg(QApplication::applicationDirPath()));

  // 内部 lambda 辅助函数：避免重复写添加 item 的代码
  auto addObjFilesFromDir = [&](const QDir& dir) {
    QFileInfoList objFiles =
        dir.entryInfoList(QStringList() << "*.obj", QDir::Files);
    for (const QFileInfo& info : objFiles) {
      QListWidgetItem* item = new QListWidgetItem(ui.modelList);
      item->setText(info.baseName());
      item->setData(Qt::UserRole, info.absoluteFilePath());
      item->setIcon(defaultIcon);
      item->setTextAlignment(Qt::AlignCenter);
      item->setToolTip(info.absoluteFilePath());
    }
  };

  // 2. 扫描当前根目录下的 obj
  addObjFilesFromDir(rootDir);

  // 3. 扫描一级子目录下的 obj
  QStringList subDirs = rootDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
  for (const QString& subDirName : subDirs) {
    QDir subDir(rootDir.absoluteFilePath(subDirName));
    addObjFilesFromDir(subDir);
  }

  ui.lblStatus->setText(
      tr(u8"加载成功，共 %1 个模型").arg(ui.modelList->count()));
}

void ModelLibraryWidget::onRefresh() {
  scanDirectory();  // 重新读取磁盘文件
}

void ModelLibraryWidget::onSearch(const QString& text) {
  // 简单的名称匹配过滤
  for (int i = 0; i < ui.modelList->count(); ++i) {
    QListWidgetItem* item = ui.modelList->item(i);
    // 如果包含搜索词（忽略大小写），则显示，否则隐藏
    bool match = item->text().contains(text, Qt::CaseInsensitive);
    item->setHidden(!match);
  }
}

void ModelLibraryWidget::onItemDoubleClicked(QListWidgetItem* item) {
  QString fullPath = item->data(Qt::UserRole).toString();
  emit signalModelDoubleClicked(fullPath);  // 发射完整路径
  this->hide();
}

void ModelLibraryWidget::onExitClicked() { this->hide(); }

void ModelLibraryWidget::centerOnScreen() {
  // 获取主屏幕（或者当前鼠标所在的屏幕）
  QScreen* screen = QGuiApplication::primaryScreen();
  if (screen) {
    QRect screenGeometry = screen->geometry();

    int x = (screenGeometry.width() - this->width()) / 2;
    int y = (screenGeometry.height() - this->height()) / 2;

    this->move(x, y);
  }
}

void ModelLibraryWidget::onChoosePath() {
  // 弹出文件夹选择对话框
  QString selectedDir = QFileDialog::getExistingDirectory(
      this, u8"选择模型库根目录",
      m_path.isEmpty() ? QApplication::applicationDirPath() : m_path,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!selectedDir.isEmpty()) {
    m_path = selectedDir;  // 更新路径
    scanDirectory();       // 立即刷新列表
  }
}