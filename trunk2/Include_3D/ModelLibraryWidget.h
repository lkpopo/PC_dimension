#pragma once

#include <QListWidgetItem>
#include <QWidget>

#include "ui_ModelLibraryWidget.h"

class ModelLibraryWidget : public QWidget {
  Q_OBJECT

 public:
  // 构造函数传入模型根目录
  explicit ModelLibraryWidget(QWidget* parent = nullptr);
  ~ModelLibraryWidget();

 signals:
  // 双击时发射此信号，通知主界面在指定位置加载模型
  void signalModelDoubleClicked(const QString& filePath);

 private slots:
  void onRefresh();                    // 刷新按钮槽函数
  void onSearch(const QString& text);  // 搜索框槽函数
  void onItemDoubleClicked(QListWidgetItem* item);
  void onExitClicked();
  void onChoosePath();

 private:
  Ui::ModelLibraryWidgetClass ui;
  void scanDirectory();  // 核心：扫描并构建列表项
  void centerOnScreen();
  QString m_path;
};
