#ifndef MULTIPOSSLIDER_H
#define MULTIPOSSLIDER_H

#include <QSlider>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOptionSlider>
#include <QPixmap> 
#include <QApplication>


// 滑块位置枚举：上方1个、下方左/右各1个
enum HandlePos {
    Handle_Top = 0,    // 上方滑块
    Handle_Bottom_Left = 1,  // 下方左侧滑块
    Handle_Bottom_Right = 2  // 下方右侧滑块
};

class MultiPosSlider : public QSlider
{
    Q_OBJECT
public:
    explicit MultiPosSlider(QWidget* parent = nullptr)
        : QSlider(Qt::Horizontal, parent), m_selectedHandle(-1)
    {
        // 初始化3个滑块的默认值（0~100）
        m_handleValues = { 3.0, 1.0, 90.0 }; // 上/下左/下右
        setRange(0.0, 100.0);
        // 隐藏原生滑块（自定义绘制）
        setStyleSheet("QSlider::handle { background: transparent; }");
        // 设置控件最小高度（容纳上下滑块）
        setMinimumHeight(40);

        // 构造函数中加载不同图片
        m_topArrowPixmap.load(QString("%1/Resource/hotpoint/item/top_arrow2.png").arg(QApplication::applicationDirPath()));
        m_leftArrowPixmap.load(QString("%1/Resource/hotpoint/item/left_arrow2.png").arg(QApplication::applicationDirPath()));
        m_rightArrowPixmap.load(QString("%1/Resource/hotpoint/item/right_arrow2.png").arg(QApplication::applicationDirPath()));
    }

    // 获取指定滑块的数值（pos: Handle_Top/Handle_Bottom_Left/Handle_Bottom_Right）
    int getHandleValue(HandlePos pos) const {
        return m_handleValues[pos];
    }

    // 设置指定滑块的数值（限制在0~100范围内）
    void setHandleValue(HandlePos pos, double value) 
    {
        if (pos == Handle_Top)
        {
            update();
            return;
        }

        value = qBound(0.0, value, 100.0);
        if (pos == Handle_Bottom_Left)
        {
            if(value > m_handleValues[Handle_Bottom_Right])
                value = m_handleValues[Handle_Bottom_Right];

            if (value > m_handleValues[Handle_Top])
                m_handleValues[Handle_Top] = value;
        }
        if (pos == Handle_Bottom_Right)
        {
            if(value < m_handleValues[Handle_Bottom_Left])
                value = m_handleValues[Handle_Bottom_Left];
            if (value < m_handleValues[Handle_Top])
                m_handleValues[Handle_Top] = value;
        }
        if (value < 1.0)
            m_handleValues[Handle_Top] = 1.0;
        else if (value > 90.0)
            m_handleValues[Handle_Top] = 90.0;

        m_handleValues[pos] = value;
        update(); // 重绘滑块
        emit handleValueChanged(Handle_Top, m_handleValues[Handle_Top]);
        emit handleValueChanged(pos, value);
    }

signals:
    // 滑块数值变化信号（pos：滑块位置，value：新值）
    void handleValueChanged(HandlePos pos, double value);

protected:
    // 重写绘制事件：绘制轨道+3个滑块
    void paintEvent(QPaintEvent* event) override {
        QSlider::paintEvent(event); // 绘制原生轨道

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing); // 抗锯齿

        // ========== 1. 获取轨道位置 ==========
        QStyleOptionSlider opt;
        initStyleOption(&opt);
        QRect trackRect = style()->subControlRect(QStyle::CC_Slider, &opt, QStyle::SC_SliderGroove, this);
        if (trackRect.isEmpty()) {
            trackRect = QRect(5, height() / 2 - 3, width() - 10, 6);
        }
        int trackCenterY = trackRect.center().y();
        {
            int leftHandleX = valueToPixel(1.0);
            int rightHandleX = valueToPixel(90.0);
            int fillY = trackCenterY + 8 - 5;
            QRect fillRect(leftHandleX, trackCenterY - 3, rightHandleX - leftHandleX, 5);
            painter.setBrush(QColor(26, 26, 26));
            painter.setPen(Qt::NoPen);
            painter.drawRect(fillRect);
        }
        // ========== 2. 绘制下方左右滑块之间的绿色填充 ==========
        int leftHandleX = valueToPixel(m_handleValues[Handle_Bottom_Left]);
        int rightHandleX = valueToPixel(m_handleValues[Handle_Bottom_Right]);
        int fillY = trackCenterY + 8 - 5;
        QRect fillRect(leftHandleX, trackCenterY - 3, rightHandleX - leftHandleX, 5);
        painter.setBrush(QColor(0, 0, 255));
        painter.setPen(Qt::NoPen);
        painter.drawRect(fillRect);

        // ========== 3. 绘制3个图片滑块 ==========
        // 上方滑块（箭头图片）
        drawImageHandle(painter, trackCenterY - 8, m_handleValues[Handle_Top], Handle_Top);
        // 下方左侧滑块
        drawImageHandle(painter, trackCenterY + 8, m_handleValues[Handle_Bottom_Left], Handle_Bottom_Left);
        // 下方右侧滑块
        drawImageHandle(painter, trackCenterY + 8, m_handleValues[Handle_Bottom_Right], Handle_Bottom_Right);
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

        // 遍历3个滑块，判断鼠标是否点击在滑块范围内
        for (int i = 0; i < 3; ++i) {
            int handleY = (i == Handle_Top) ? (trackCenterY - 11) : (trackCenterY + 11);
            int handleX = valueToPixel(m_handleValues[i]);
            // 滑块点击区域：16x16的矩形（扩大检测范围，提升交互体验）
            QRect clickRect(handleX - 13, handleY - 11, 26, 22);
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
            setHandleValue(static_cast<HandlePos>(m_selectedHandle), value);
        }
        QSlider::mouseMoveEvent(event);
    }

    // 重写鼠标释放：取消选中
    void mouseReleaseEvent(QMouseEvent* event) override {
        m_selectedHandle = -1;
        QSlider::mouseReleaseEvent(event);
    }

private:
    // 新增成员变量
    QPixmap m_topArrowPixmap;    // 上方滑块图片
    QPixmap m_leftArrowPixmap;   // 下方左侧图片
    QPixmap m_rightArrowPixmap;  // 下方右侧图片

   
    // 新增：绘制图片滑块的核心函数
    void drawImageHandle(QPainter& painter, int y, double value, HandlePos pos) {
        int x = valueToPixel(value);
        if (pos == Handle_Bottom_Left)
            x -= 4;
        if (pos == Handle_Bottom_Right)
            x += 4;
        QPixmap currentPixmap;
        if (pos == Handle_Top) currentPixmap = m_topArrowPixmap;
        else if (pos == Handle_Bottom_Left) currentPixmap = m_leftArrowPixmap;
        else currentPixmap = m_rightArrowPixmap;

        if (!currentPixmap.isNull()) {
            int imgW = currentPixmap.width();
            int imgH = currentPixmap.height();
            painter.drawPixmap(x - imgW / 2, y - imgH / 2, currentPixmap);
        }
        else {
            // 加载失败：绘制兜底方块（按原颜色区分）
            QColor color;
            if (pos == Handle_Top) color = QColor(0, 255, 0);
            else if (pos == Handle_Bottom_Left) color = QColor(122, 0, 217);
            else color = QColor(0, 122, 217);

            painter.setBrush(color);
            painter.setPen(color);
            painter.drawRect(x - 5, y - 5, 10, 10);
        }
    }
    // 辅助函数：绘制单个滑块
    void drawHandle(QPainter& painter, int y, double value, const QColor& color, HandlePos pos) {
        double x = valueToPixel(value); // 数值转像素X坐标
        QRect handleRect(x - 5, y - 5, 10, 10); // 滑块尺寸：10x10圆形

        // 选中的滑块高亮（边框+填充色加深）
        painter.setBrush(color);
        painter.setPen(color); 
        //if (m_selectedHandle == pos) {
        //    painter.setBrush(color.darker(120)); // 加深颜色
        //    painter.setPen(QColor(255, 255, 255)); // 白色边框
        //}
        //else {
        //    painter.setBrush(color);
        //    painter.setPen(Qt::black); // 黑色边框
        //}
        //painter.drawEllipse(handleRect); // 绘制圆形滑块
        painter.drawRect(handleRect); // 绘制纯方形滑块
    }

    // 辅助函数：数值转像素X坐标（0~100 → 控件像素）
    //float valueToPixel(float value) const {
    //    double sliderWidth = width() - 10; // 左右边距各5px
    //    return 5 + (value - 0.1) * sliderWidth / (180 - 0.1);
    //}
    // 辅助函数：像素X坐标转数值（匹配上面的映射逻辑）
    double pixelToValue(int x) const {
        const int HANDLE_HALF_WIDTH = 7; // 与上面保持一致
        double sliderWidth = width() - 2 * HANDLE_HALF_WIDTH;

        // 限制鼠标X坐标在有效区间内（避免拖动到边界外）
        x = qBound(HANDLE_HALF_WIDTH, x, width() - HANDLE_HALF_WIDTH);

        double value = 0.0 + (x - HANDLE_HALF_WIDTH) * (100 - 0.0) / sliderWidth;
        return qBound(1.0, value, 90.0);
    }
    //// 辅助函数：像素X坐标转数值
    //float pixelToValue(int x) const {
    //    double sliderWidth = width() - 10;
    //    double value = 0.1 + (x - 5) * (180 -0.1) / sliderWidth;
    //    return qBound(0.1, value, 180.0); 
    //}
    // 辅助函数：数值转像素X坐标（适配滑块/图片半宽，避免截断）
    int valueToPixel(double value) const {
        // 滑块/图片的半宽（根据实际尺寸调整，这里按10px滑块算半宽=5，图片16px则半宽=8）
        const int HANDLE_HALF_WIDTH = 7; // 若用16px箭头图片，改为8
        // 有效绘制宽度：控件宽度 - 2*半宽（左右各留半宽）
        double sliderWidth = width() - 2 * HANDLE_HALF_WIDTH;
        // 数值映射到[半宽, 宽度-半宽]区间
        return HANDLE_HALF_WIDTH + (value - 0.0) * sliderWidth / (100 - 0.0);
    }

    QList<double> m_handleValues; // 存储3个滑块数值：[上, 下左, 下右]
    int m_selectedHandle;      // 当前选中的滑块索引（-1=未选中）
    QPixmap m_arrowPixmap;        // 箭头图片
    QBrush m_defaultBrush;        // 图片加载失败时的兜底画刷
};

#endif // MULTIPOSSLIDER_H