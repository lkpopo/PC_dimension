#include "Space_3D_project_item.h"

Space_3D_project_item::Space_3D_project_item(const QString& pro_name,
                                             const QString& create_time,
                                             const QString& modify_time,
                                             const QString& icon_path,
                                             const int scrWidth,
                                             QWidget* parent)
    : QWidget(parent) {
  ui.setupUi(this);

  setFixedSize(scrWidth - 80, 100);

  m_projectName = pro_name;
  ui.lbl_projectName->setText(pro_name);
  ui.lbl_createTime->setText(create_time);
  ui.lbl_lastChange->setText(modify_time);

  ui.btnViewPro->move(scrWidth - 150 - 150, 34);
  ui.btnEditPro->move(scrWidth - 150 - 100, 34);
  ui.btnDelPro->move(scrWidth - 150 - 50, 34);

  //QPixmap pixmap(icon_path);
  //QPixmap fitpixmap =
  //    pixmap.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  //ui.pushButton_frontCover->setIcon(QIcon(fitpixmap));
  //ui.pushButton_frontCover->setIconSize(QSize(64, 64));
  //ui.pushButton_frontCover->setFlat(true);
  //ui.pushButton_frontCover->setStyleSheet("border: 0px");

  QPixmap pix(icon_path);
  QPixmap fitpixmap =
      pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation);
  ui.lbl_selPic_change->setPixmap(fitpixmap);
  ui.lbl_selPic_change->setAlignment(Qt::AlignCenter);

  connect(ui.btnDelPro, &QPushButton::clicked,
          [this]() { emit deleteRequested(m_projectName); });

  connect(ui.btnEditPro, &QPushButton::clicked,
          [this]() { emit loadRequested(m_projectName); });

  connect(ui.btnViewPro, &QPushButton::clicked,
          [this]() { emit BrowseRequested(m_projectName); });

  connect(ui.btnBackGround, &QPushButton::clicked,
          [this]() { emit BrowseRequested(m_projectName); });
}

Space_3D_project_item::~Space_3D_project_item() {}
