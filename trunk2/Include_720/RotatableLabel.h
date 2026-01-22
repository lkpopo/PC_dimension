#ifndef ROTATABLELABEL_H
#define ROTATABLELABEL_H

#include <QLabel>
#include <QPainter>
#include <QStyleOption>
#include <QDebug>

class RotatableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit RotatableLabel(QWidget* parent = nullptr)
        : QLabel(parent), m_rotationAngle(0) {}

    // 重写 setPixmap 方法，保存图片供手动绘制
    void setPixmap(const QPixmap& pixmap) {
        m_customPixmap = pixmap; // 保存图片到成员变量
       // 标签大小调整：若图片太大，标签自适应图片大小；若图片太小，不放大（避免模糊）
        if (pixmap.width() > 10 && pixmap.height() > 10) { // 避免空图片
            QSize labelSize = pixmap.size().scaled(100, 100, Qt::KeepAspectRatio); // 限制最大尺寸300x300
            this->setFixedSize(labelSize); // 标签大小适配图片，避免过度缩放
        }
        update();
    }

    void setRotationAngle(int angle) {
        m_rotationAngle = angle;
        qDebug() << "旋转角度:" << angle;
        update();
    }

    int rotationAngle() const { return m_rotationAngle; }

protected:
    void paintEvent(QPaintEvent* event) override {
        if (!isVisible() || width() <= 0 || height() <= 0) {
            return;
        }

        QPointF center = rect().center();
        QPainter painter(this);
        // ========== 新增：高质量绘图设置 ==========
        painter.setRenderHint(QPainter::Antialiasing, true); // 抗锯齿
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true); // 平滑图片变换
        painter.setRenderHint(QPainter::HighQualityAntialiasing, true); // 高质量抗锯齿（Qt5支持）
        painter.setRenderHint(QPainter::TextAntialiasing, true); // 文字抗锯齿（若有文字）
        painter.save();

        // 旋转核心逻辑（只创建一个 QPainter，避免冲突）
        painter.translate(center.x(), center.y());
        painter.rotate(m_rotationAngle);
        painter.translate(-center.x(), -center.y());

        // ========== 关键：手动绘制图片（响应旋转） ==========
        if (!m_customPixmap.isNull()) {
            // 图片缩放：适应标签大小，保持宽高比
            QPixmap scaledPix = m_customPixmap.scaled(
                rect().size() - QSize(20, 20), // 内边距 20px
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation // 平滑缩放，避免模糊
            );
            // 图片居中：在旋转后的坐标系中居中绘制
            QPointF pixPos = center - QPointF(scaledPix.width() / 2, scaledPix.height() / 2);
            painter.drawPixmap(pixPos, scaledPix);
        }
        else {
            // 若无图片，绘制文字（兼容文字标签）
            painter.setPen(this->palette().text().color());
            painter.setFont(this->font());
            painter.drawText(rect(), this->alignment(), this->text());
        }

        painter.restore();
    }

private:
    int m_rotationAngle;
    QPixmap m_customPixmap; // 保存图片，用于手动绘制
};

#endif // ROTATABLELABEL_H