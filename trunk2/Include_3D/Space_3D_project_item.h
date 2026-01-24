#pragma once

#include <QWidget>

#include "ui_Space_3D_project_item.h"

class Space_3D_project_item : public QWidget {
  Q_OBJECT

 public:
  Space_3D_project_item(const QString& pro_name, const QString& create_time,
                        const QString& modify_time, const QString& icon_path,
                        const int scrWidth,
      QWidget* parent = nullptr);
  ~Space_3D_project_item();

  bool isCheckBoxChecked() const { return ui.checkBox_sel->isChecked(); };



  signals:
  void deleteRequested(QString pName);  // 请求删除信号
  void loadRequested(QString pName);    // 请求加载信号
  void BrowseRequested(QString pName);  // 请求浏览信号

 private:
  Ui::Space_3D_project_itemClass ui;

  QString m_projectName;
};
