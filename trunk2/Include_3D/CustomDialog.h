#ifndef CUSTOMDIALOG_H
#define CUSTOMDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

class CustomDialog : public QDialog {
  Q_OBJECT

 public:
  // Style: 1-警告, 2-提示, 3-询问, 4-错误
  explicit CustomDialog(int style, const QString& text,
                        QWidget* parent = nullptr);
  ~CustomDialog() = default;

 protected:
  // 窗口拖动支持
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;

 private:
  void initUi(int style, const QString& text);
  QString getIconPath(int style);

  QPoint m_movePoint;
  bool m_isPressed = false;
};

#endif  // CUSTOMDIALOG_H