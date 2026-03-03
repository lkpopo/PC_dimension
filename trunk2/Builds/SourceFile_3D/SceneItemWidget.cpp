#include "SceneItemWidget.h"
SceneItemWidget::SceneItemWidget(QString name, QString path, QWidget *parent)
    : QWidget(parent) {
  ui.setupUi(this);

  QFileInfo info(path);
  if (info.suffix().toLower() == "obj") {
    ui.btnSimulate->setVisible(true);
  } else {
    ui.btnSimulate->setVisible(false);
  }

  if (info.suffix().toLower() == "las") {
    ui.btnClassRGB->setVisible(true);
  } else {
    ui.btnClassRGB->setVisible(false);
  }

  ui.lblName->setText(name);
  filePath = path;
}

SceneItemWidget::~SceneItemWidget() {}

void SceneItemWidget::on_btnSimulate_clicked() {
  if (ui.btnSimulate->text() == u8"调整") {
    ui.btnSimulate->setText(u8"完成");
    emit signalSimulateStarted(filePath);  // 发送开始信号
  } else {
    ui.btnSimulate->setText(u8"调整");
    emit signalSimulateFinished(filePath);  // 发送结束信号
  }
}
