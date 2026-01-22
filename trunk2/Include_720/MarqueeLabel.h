#ifndef MARQUEELABEL_H
#define MARQUEELABEL_H

#include <QLabel>
#include <QTimer>
#include <QPainter>

class MarqueeLabel : public QLabel
{
    Q_OBJECT

public:
    explicit MarqueeLabel(QWidget* parent = nullptr);
    explicit MarqueeLabel(const QString& text, QWidget* parent = nullptr);

    void setScrollSpeed(int speed); // 设置滚动速度
    void setScrollEnabled(bool enabled); // 启用/禁用滚动

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void updateScrollPosition();

private:
    void init();
    void updateRollingState();

    QTimer* m_timer;
    int m_scrollPos; // 滚动位置
    int m_scrollSpeed; // 滚动速度
    bool m_scrollEnabled; // 是否启用滚动
    int m_textWidth; // 文本宽度
};
#endif