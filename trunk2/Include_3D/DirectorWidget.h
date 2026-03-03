#pragma once

#include <QComboBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>
#include <osg/Node>
#include <osg/Object>
#include <osg/ref_ptr>
#include <QTimer>

#include "ui_DirectorWidget.h"
class DirectorWidget : public QWidget {
  Q_OBJECT

 public:
  DirectorWidget(QWidget* parent = nullptr);
  ~DirectorWidget();

  void refreshAssetList(
      const QMap<QString, osg::ref_ptr<osg::Object>>& pathNodeMap);

 private:
  Ui::DirectorWidgetClass ui;

  QMap<QString, osg::ref_ptr<osg::Object>> m_pathNodeMap;
  int m_currentStepIndex;
  QTimer* m_playTimer;



  void executeNextStep();
  void highlightNode(osg::Node* node, bool on);
  void flashNode(osg::Node* node, int times, int intervalMs,
                                 std::function<void()> callback);

 public slots:
  void on_btnAdd_clicked();
  void on_btnPlay_clicked();
  void on_btnExit_clicked();
  void on_btnReset_clicker();
};
