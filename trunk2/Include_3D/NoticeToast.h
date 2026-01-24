#pragma once
#include <QTimer>
#include <QWidget>

class NoticeToast : public QWidget {
  Q_OBJECT
 public:
  // ❗注意：不叫 show
  static void popup(QWidget* parent, const QString& text,
                    int durationMs = 2000);
  explicit NoticeToast(QWidget* parent = nullptr);
 protected:
  void paintEvent(QPaintEvent* event) override;

 private slots:
  void onTimeout();

 private:
  
  void updateGeometryToParent();  // ⭐ 新增：统一调整位置和尺寸

  bool eventFilter(QObject* obj, QEvent* event);

 private:
  QString m_text;
  QTimer m_timer;
  int m_timeLeftMs;
  int m_opacity;  // 0~255
};
