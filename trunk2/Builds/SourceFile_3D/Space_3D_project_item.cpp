#include "Space_3D_project_item.h"

Space_3D_project_item::Space_3D_project_item(const QString& pro_name,
                                             const QString& create_time,
                                             const QString& modify_time,
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

  // 假设 UI 里的按钮叫 btnEdit, btnDelete
  connect(ui.btnDelPro, &QPushButton::clicked,
          [this]() { emit deleteRequested(m_projectName); });

  connect(ui.btnEditPro, &QPushButton::clicked,
          [this]() { emit loadRequested(m_projectName); });
}

Space_3D_project_item::~Space_3D_project_item() {}
