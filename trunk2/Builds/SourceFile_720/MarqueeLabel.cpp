#include "marqueelabel.h"
#include <QPainter>
#include <QFontMetrics>

MarqueeLabel::MarqueeLabel(QWidget* parent)
    : QLabel(parent)
    , m_scrollPos(0)
    , m_scrollSpeed(1)
    , m_scrollEnabled(true)
    , m_textWidth(0)
{
    init();
}

MarqueeLabel::MarqueeLabel(const QString& text, QWidget* parent)
    : QLabel(text, parent)
    , m_scrollPos(0)
    , m_scrollSpeed(1)
    , m_scrollEnabled(true)
    , m_textWidth(0)
{
    init();
}

void MarqueeLabel::init()
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MarqueeLabel::updateScrollPosition);

    // 设置固定高度，避免尺寸变化影响滚动判断
    setMinimumHeight(25);
    setMaximumHeight(30);
}

void MarqueeLabel::setScrollSpeed(int speed)
{
    m_scrollSpeed = qMax(1, speed);
}

void MarqueeLabel::setScrollEnabled(bool enabled)
{
    m_scrollEnabled = enabled;
    updateRollingState();
}

void MarqueeLabel::paintEvent(QPaintEvent* event)
{
    if (!m_scrollEnabled || m_textWidth <= width()) {
        // 如果不需要滚动，使用默认绘制
        QLabel::paintEvent(event);
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 设置字体和颜色
    QFont font = this->font();
    painter.setFont(font);
    painter.setPen(palette().color(foregroundRole()));

    // 计算文本绘制位置
    int textX = width() - m_scrollPos;
    int textY = (height() + painter.fontMetrics().height()) / 2 - painter.fontMetrics().descent();

    // 绘制文本
    painter.drawText(textX, textY, text());

    // 当文本开始从左侧消失时，在右侧绘制重复文本
    if (m_scrollPos > m_textWidth) {
        int repeatX = textX - m_textWidth - 20; // 留出一些间距
        painter.drawText(repeatX, textY, text());
    }
}

void MarqueeLabel::resizeEvent(QResizeEvent* event)
{
    QLabel::resizeEvent(event);
    updateRollingState();
}

void MarqueeLabel::updateScrollPosition()
{
    m_scrollPos += m_scrollSpeed;

    // 当文本完全滚动出视野时，重置位置
    if (m_scrollPos > m_textWidth + width()) {
        m_scrollPos = 0;
    }

    update(); // 触发重绘
}

void MarqueeLabel::updateRollingState()
{
    QFontMetrics fm(font());
    m_textWidth = fm.horizontalAdvance(text());

    // 只有当文本宽度超过控件宽度时才启用滚动
    if (m_scrollEnabled && m_textWidth > width()) {
        if (!m_timer->isActive()) {
            m_timer->start(30); // 30ms刷新一次
        }
    }
    else {
        m_timer->stop();
        m_scrollPos = 0;
    }
}