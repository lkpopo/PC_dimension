// 头文件：自定义ListWidget（修复自定义项拖拽丢失问题）
#ifndef CUSTOMLISTWIDGET_H
#define CUSTOMLISTWIDGET_H

#include <QListWidget>
#include <QDropEvent>
#include <QListWidgetItem>
//#include "CustomItemWidget.h"

class CustomListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit CustomListWidget(QWidget* parent = nullptr) : QListWidget(parent) {}

protected:
    // 重写放置事件：手动处理自定义项的移动，避免Widget丢失
    void dropEvent(QDropEvent* event) override
    {
        // 1. 获取拖拽的源项和源位置
        QListWidgetItem* sourceItem = this->currentItem();
        if (!sourceItem) { // 无有效源项，忽略事件
            event->ignore();
            return;
        }
        int sourceRow = this->row(sourceItem);

        // 2. 获取放置的目标位置
        QPoint dropPos = event->pos();
        QListWidgetItem* targetItem = this->itemAt(dropPos);
        int targetRow = -1;
        if (targetItem) {
            targetRow = this->row(targetItem);
        }
        else {
            // 放置到列表末尾
            targetRow = this->count();
        }

        // 3. 关键：取出源项的自定义Widget（避免丢失）
        QWidget* sourceWidget = this->itemWidget(sourceItem);
        if (!sourceWidget) { // 无自定义Widget，使用默认逻辑
            QListWidget::dropEvent(event);
            return;
        }

        // 4. 移动项：先移除源项，再插入到目标位置
        this->takeItem(sourceRow); // 从源位置移除项（不销毁Item和Widget）
        QListWidgetItem* newItem = new QListWidgetItem();
        newItem->setSizeHint(sourceItem->sizeHint()); // 继承原项大小
        this->insertItem(targetRow, newItem);
        this->setItemWidget(newItem, sourceWidget); // 重新绑定Widget

        // 5. 清理源项 + 标记事件成功
        delete sourceItem; // 销毁原Item（已重新创建新Item绑定Widget）
        this->setCurrentItem(newItem); // 选中移动后的项
        event->acceptProposedAction();
    }
};

#endif // CUSTOMLISTWIDGET_H