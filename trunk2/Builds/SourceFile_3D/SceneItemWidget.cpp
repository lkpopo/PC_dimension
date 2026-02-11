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

  ui.lblName->setText(name);
  filePath = path;
}

SceneItemWidget::~SceneItemWidget() {}
