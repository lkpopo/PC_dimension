#include "NoticeToast.h"

#include <QFontMetrics>
#include <QPainter>
#include <QResizeEvent>

NoticeToast::NoticeToast(QWidget* parent)
    : QWidget(parent), m_timeLeftMs(0), m_opacity(255) {
  setAttribute(Qt::WA_TranslucentBackground);
  setWindowFlags(Qt::FramelessWindowHint | Qt::ToolTip);

  QFont f = font();
  f.setPointSize(12);
  setFont(f);

  if (parent) {
    parent->installEventFilter(this);  
  }

  m_timer.setInterval(30);
  connect(&m_timer, &QTimer::timeout, this, &NoticeToast::onTimeout);
}


void NoticeToast::popup(QWidget* parent, const QString& text, int durationMs) {
  if (!parent) return;

  auto* toast = new NoticeToast(parent);
  toast->m_text = text;
  toast->m_timeLeftMs = durationMs;
  toast->m_opacity = 255;

  toast->updateGeometryToParent();  // ⭐ 第一次算位置和大小
  toast->QWidget::show();

  toast->m_timer.start();
}


void NoticeToast::onTimeout() {
  m_timeLeftMs -= m_timer.interval();

  // 最后 500ms 开始淡出
  if (m_timeLeftMs < 500) {
    m_opacity -= 15;
    if (m_opacity < 0) m_opacity = 0;
  }

  if (m_timeLeftMs <= 0) {
    m_timer.stop();
    deleteLater();
    return;
  }

  update();
}

void NoticeToast::paintEvent(QPaintEvent*) {
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing);

  // 背景
  QColor bg(250, 100, 0, m_opacity);
  p.setBrush(bg);
  p.setPen(Qt::NoPen);
  p.drawRoundedRect(rect(), 8, 8);

  // 文字
  p.setPen(Qt::white);
  p.drawText(rect(), Qt::AlignCenter, m_text);
}

//void NoticeToast::resizeEvent(QResizeEvent*) {
//  if (!parentWidget()) return;
//
//  // 父窗口变化时，始终保持居中
//  move((parentWidget()->width() - width()) / 2, parentWidget()->height() / 5);
//}

void NoticeToast::updateGeometryToParent() {
  QWidget* p = parentWidget();
  if (!p) return;

  QFontMetrics fm(font());

  // ===== 宽度规则 =====
  int minW = 280;
  int maxW = static_cast<int>(p->width() * 0.7);

  int textW = fm.horizontalAdvance(m_text);
  int w = qBound(minW, textW + 64, maxW);

  // ===== 高度规则（支持换行）=====
  QRect textRect(0, 0, w - 64, 1000);
  QRect br = fm.boundingRect(textRect, Qt::TextWordWrap, m_text);
  int h = br.height() + 32;

  resize(w, h);

  // ===== 位置：靠顶部 =====
  // 顶部位置
  int topMargin = 40;  // 顶部固定留白
  move((p->width() - w) / 2, topMargin);
}


bool NoticeToast::eventFilter(QObject* obj, QEvent* event) {
  if (obj == parentWidget()) {
    if (event->type() == QEvent::Resize || event->type() == QEvent::Move) {
      updateGeometryToParent();
    }
  }
  return QWidget::eventFilter(obj, event);
}
