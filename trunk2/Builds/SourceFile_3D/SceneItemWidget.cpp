#include "SceneItemWidget.h"

SceneItemWidget::SceneItemWidget(QString name, QString path, QWidget *parent)
    : QWidget(parent) {
  ui.setupUi(this);

  ui.lblName->setText(name);
  filePath = path;
}

SceneItemWidget::~SceneItemWidget() {}
