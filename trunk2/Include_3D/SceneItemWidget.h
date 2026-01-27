#pragma once

#include <QFileInfo>
#include <QWidget>

#include "ui_SceneItemWidget.h"

class SceneItemWidget : public QWidget {
  Q_OBJECT

 public:
  explicit SceneItemWidget(QString name, QString path,
                           QWidget *parent = nullptr);
  ~SceneItemWidget();

  void setDeleteButtonVisible(bool visible) {
    ui.btnDelete->setVisible(visible);
  }

  QString filePath;

 signals:
  void signalLocate(QString path);
  void signalDelete(QString path);
  void signalSimulate(QString path);

 private slots:
  void on_btnLocate_clicked() { emit signalLocate(filePath); }
  void on_btnDelete_clicked() { emit signalDelete(filePath); }
  void on_btnSimulate_clicked() { emit signalSimulate(filePath); }

 private:
  Ui::SceneItemWidgetClass ui;
};
