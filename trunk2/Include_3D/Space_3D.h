#pragma once

#include <qscreen.h>

#include <QSqlError>
#include <QSqlQuery>
#include <QWidget>
#include <QSplashScreen>
#include <QMovie>
#include <QLabel>
#include <iostream>
#include <string>

#include "OSGEarthApp.h"
#include "ui_Space_3D.h"

class Space_3D : public QWidget {
  Q_OBJECT

 public:
  Space_3D(QWidget* parent = nullptr);
  ~Space_3D();

  void refreshProjectList();

 private:
  Ui::Space_3DClass ui;

 private:
  void initLayout();
  void initSignalsAndSlots();
  void initDatabase();
  void onDeleteProject(QString pName);
  void onLoadProject(QString pName);
  

  int _scrWidth = 1920;
  int _scrHeight = 1080;

  OSGEarthApp* m_earthWindow = nullptr;

 private slots:
  void handleCreateProject();
  void executeBatchDelete();
  void applySort();
};
