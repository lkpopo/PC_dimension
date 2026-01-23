#ifndef MULTIPOSSLIDER_V_H
#define MULTIPOSSLIDER_V_H

#include <QSlider>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionSlider>

// 滑块位置枚举：下方左/右各1个
enum HandlePos_V {
    Handle_Top_V = 0,
    Handle_Bottom_Left_V = 1,  // 下方左侧滑块
    Handle_Bottom_Right_V = 2  // 下方右侧滑块
};

class MultiPosSlider_V : public QSlider
{
    Q_OBJECT
public:
    explicit MultiPosSlider_V(QWidget* parent = nullptr)
        : QSlider(Qt::Horizontal, parent), m_selectedHandle(-1)
    {
        // 初始化3个滑块的默认值（0~100）
        m_handleValues = { -90,90 }; // 下左/下右
        setRange(-180, 180);
        // 隐藏原生滑块（自定义绘制）
        setStyleSheet("QSlider::handle { background: transparent;}");
        // 设置控件最小高度（容纳上下滑块）
        setMinimumHeight(40);
    }

    // 获取指定滑块的数值（pos: Handle_Top/Handle_Bottom_Left/Handle_Bottom_Right）
    int getHandleValue(HandlePos_V pos) const {
        return m_handleValues[pos];
    }

    // 设置指定滑块的数值（限制在0~100范围内）
    void setHandleValue(HandlePos_V pos, int value)
    {
        value = qBound(-180, value, 180);
        if (pos == Handle_Bottom_Left_V)
        {
            value = qBound(-180, value, m_handleValues[Handle_Bottom_Right_V] - 20);
            if (value > m_handleValues[Handle_Bottom_Right_V])
                value = m_handleValues[Handle_Bottom_Right_V];
            if (m_handleValues[Handle_Bottom_Right_V] - value < 20)
            {
                m_handleValues[Handle_Bottom_Right_V] = value + 20;
                emit handleValueChangedV(Handle_Bottom_Right_V, m_handleValues[Handle_Bottom_Right_V]);
            }
        }
        else if (pos == Handle_Bottom_Right_V)
        {
            value = qBound(m_handleValues[Handle_Bottom_Left_V] + 20, value, 180);
            if (value < m_handleValues[Handle_Bottom_Left_V])
                value = m_handleValues[Handle_Bottom_Left_V];
            if (value - m_handleValues[Handle_Bottom_Left_V] < 20)
            {
                m_handleValues[Handle_Bottom_Left_V] = value - 20;
                emit handleValueChangedV(Handle_Bottom_Left_V, m_handleValues[Handle_Bottom_Left_V]);
            }
        }
        else
        {
            //
        }
        m_handleValues[pos] = value;
        update(); // 重绘滑块
        emit handleValueChangedV(pos, value);
    }
signals:
    // 滑块数值变化信号（pos：滑块位置，value：新值）
    void handleValueChangedV(HandlePos_V pos, int value);
protected:
    // 重写绘制事件：绘制轨道+3个滑块
    void paintEvent(QPaintEvent* event) override {
        QSlider::paintEvent(event); // 绘制原生轨道

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

        // ========== 1. 获取轨道位置（兼容所有Qt版本） ==========
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        QRect trackRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
        // 兜底：轨道为空时手动计算
        if (trackRect.isEmpty()) {
            trackRect = QRect(5, height() / 2 - 3, width() - 10, 6);
        }
        int trackCenterY = trackRect.center().y(); // 轨道中心Y坐标
        //填充灰色
        {
            // 获取下方左右滑块的像素X坐标（中心）
            int leftHandleX = valueToPixel(-180);
            int rightHandleX = valueToPixel(180);
            // 填充区域：与下方滑块同高度，左右为两个滑块的中心（对齐滑块边缘则-5）
            int fillY = trackCenterY + 8 - 5; // 与下方滑块Y坐标一致
            QRect fillRect(leftHandleX, trackCenterY - 2, rightHandleX - leftHandleX, 5); // 高度10px（与滑块一致）
            painter.setBrush(QColor(26, 26, 26)); // 绿色填充
            painter.setPen(Qt::NoPen); // 无边框
            painter.drawRect(fillRect); // 绘制填充矩形
        }
         // ========== 2. 绘制下方左右滑块之间的绿色填充 ==========
        // 获取下方左右滑块的像素X坐标（中心）
        int leftHandleX = valueToPixel(m_handleValues[Handle_Bottom_Left_V]);
        int rightHandleX = valueToPixel(m_handleValues[Handle_Bottom_Right_V]);
        // 填充区域：与下方滑块同高度，左右为两个滑块的中心（对齐滑块边缘则-5）
        int fillY = trackCenterY + 8 - 5; // 与下方滑块Y坐标一致
        QRect fillRect(leftHandleX, trackCenterY - 2, rightHandleX - leftHandleX, 5); // 高度10px（与滑块一致）
        painter.setBrush(QColor(40, 110, 250)); // 绿色填充
        painter.setPen(Qt::NoPen); // 无边框
        painter.drawRect(fillRect); // 绘制填充矩形
        // ========== 2. 绘制2个滑块 ==========
        drawHandle(painter, trackCenterY - 8, m_handleValues[Handle_Top_V],
        QColor(40, 40, 40), Handle_Top_V);
        // 2.1 下方左侧滑块（Handle_Bottom_Left）
        drawHandle(painter, trackCenterY + 8, m_handleValues[Handle_Bottom_Left_V],
            QColor(122, 0, 217), Handle_Bottom_Left_V);

        // 2.2 下方右侧滑块（Handle_Bottom_Right）
        drawHandle(painter, trackCenterY + 8, m_handleValues[Handle_Bottom_Right_V],
            QColor(217, 0, 122), Handle_Bottom_Right_V);
    }

    // 重写鼠标按下：识别选中的滑块
    void mousePressEvent(QMouseEvent* event) override {
        m_selectedHandle = -1;
        // 获取轨道中心Y坐标
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        QRect trackRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
        if (trackRect.isEmpty()) trackRect = QRect(5, height() / 2 - 3, width() - 10, 6);
        int trackCenterY = trackRect.center().y();

        // 遍历2个滑块，判断鼠标是否点击在滑块范围内
        for (int i = 0; i < 3; ++i) {
            int handleY = (i == Handle_Top_V) ? (trackCenterY - 8) : (trackCenterY + 8);
            int handleX = valueToPixel(m_handleValues[i]);
            // 滑块点击区域：16x16的矩形（扩大检测范围，提升交互体验）
            QRect clickRect(handleX - 8, handleY - 8, 16, 16);
            if (clickRect.contains(event->pos())) {
                m_selectedHandle = i;
                break;
            }
        }
        QSlider::mousePressEvent(event);
    }

    // 重写鼠标移动：拖动选中的滑块
    void mouseMoveEvent(QMouseEvent* event) override {
        if (m_selectedHandle != -1) {
            double value = pixelToValue(event->x()); // 鼠标X坐标转数值
            setHandleValue(static_cast<HandlePos_V>(m_selectedHandle), value);
        }
        QSlider::mouseMoveEvent(event);
    }

    // 重写鼠标释放：取消选中
    void mouseReleaseEvent(QMouseEvent* event) override {
        m_selectedHandle = -1;
        QSlider::mouseReleaseEvent(event);
    }

private:
    // 辅助函数：绘制单个滑块
    void drawHandle(QPainter& painter, int y, double value, const QColor& color, HandlePos_V pos) {
        double x = valueToPixel(value); // 数值转像素X坐标
        QRect handleRect(x - 5, y - 5, 10, 10); // 滑块尺寸：10x10圆形

        // 选中的滑块高亮（边框+填充色加深）
        painter.setBrush(color);
        painter.setPen(color); 
        painter.drawRect(handleRect); // 绘制纯方形滑块
    }

    // 辅助函数：数值转像素X坐标（0~100 → 控件像素）
    float valueToPixel(float value) const {
        double sliderWidth = width() - 10; // 左右边距各5px
        return 5 + (value - (-180)) * sliderWidth / (180 - (-180));
    }

    // 辅助函数：像素X坐标转数值
    float pixelToValue(int x) const {
        double sliderWidth = width() - 10;
        double value = -180 + (x - 5) * (180 -(-180)) / sliderWidth;
        return qBound(-180.0, value, 180.0);
    }

    QList<int> m_handleValues; // 存储3个滑块数值：[上, 下左, 下右]
    int m_selectedHandle;      // 当前选中的滑块索引（-1=未选中）
};

#endif // MULTIPOSSLIDER_FOV_H