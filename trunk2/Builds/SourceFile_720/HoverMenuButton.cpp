#include "HoverMenuButton.h"
#include <QWidget>
#include <QAction>
#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QWidgetAction>
// 辅助函数：初始化菜单
void HoverMenuButton::initMenu()
{
    m_menu.setFixedWidth(50);
    // 1. 先设置菜单基础样式（无菜单项干扰）
    m_menu.setStyleSheet(R"(
        QMenu {
            border-radius:5px;
            background-color: white;
            border: 1px solid #eeeeee;
            padding: 2px 0; /* 菜单内边距，优化菜单项间距 */
        }
    )");

    // 2. 定义菜单项文本列表
    QStringList itemTexts = { u8"锁定", u8"隐藏", u8"删除" };

    // 3. 为每个文本创建【QWidgetAction】（低版本Qt兼容，替代setWidgetForAction）
    foreach(QString text, itemTexts)
    {
        // --------------------------
        // 关键：使用 QWidgetAction 替代 QAction + setWidgetForAction
        // --------------------------
        QWidgetAction* widgetAction = new QWidgetAction(&m_menu);
        widgetAction->setText(text);
        // 创建自定义菜单项容器Widget
        QWidget* itemWidget = new QWidget();
        itemWidget->setFixedWidth(48); // 略小于菜单宽度，避免溢出
        itemWidget->setFixedHeight(30); // 固定高度，保证美观

        // 给自定义Widget设置悬浮样式（核心：100%生效，不受Qt默认样式影响）
        itemWidget->setStyleSheet(R"(
            /* 菜单项默认样式 */
            QWidget {
                background-color: transparent; /* 透明默认背景 */
                border-radius: 3px;
            }
            /* 菜单项悬浮样式（鼠标移入时底色变化） */
            QWidget:hover {
                background-color: #f0f0f0 !important; /* 悬浮底色，可自定义颜色 */
            }
        )");

        // 创建文本标签，设置居中显示（与你原有需求一致）
        QLabel* itemLabel = new QLabel(text, itemWidget);
        itemLabel->setAlignment(Qt::AlignCenter);
        itemLabel->setStyleSheet("color: #333333;"); // 文本颜色

        // 布局标签（消除内边距干扰，保证显示整齐）
        QHBoxLayout* itemLayout = new QHBoxLayout(itemWidget);
        itemLayout->addWidget(itemLabel);
        itemLayout->setContentsMargins(0, 0, 0, 0); // 去除布局内边距
        itemLayout->setSpacing(0);

        // --------------------------
        // 关键：将自定义Widget绑定到 QWidgetAction
        // --------------------------
        widgetAction->setDefaultWidget(itemWidget);

        // 将 QWidgetAction 添加到菜单（替代原有 m_menu.addAction(action)）
        m_menu.addAction(widgetAction);
        //
        m_menuItemWidgetActionMap.insert(text, widgetAction);
        m_menuItemLabelMap.insert(text, itemLabel);

        if (text == u8"锁定")
        {
            m_menuItemWidgetActionMap.insert(u8"解锁", widgetAction);
            m_menuItemLabelMap.insert(u8"解锁", itemLabel);
        }
        else if (text == u8"隐藏")
        {
            m_menuItemWidgetActionMap.insert(u8"显示", widgetAction);
            m_menuItemLabelMap.insert(u8"显示", itemLabel);
        }
        else
        {
        }// } 
    }
    
    // 初始化定时器：100ms检测一次，非单次执行
    m_checkTimer.setInterval(100);
    // 绑定定时器到检测槽函数
    connect(&m_checkTimer, &QTimer::timeout, this, &HoverMenuButton::checkMousePosAndHideMenu);
    // 绑定菜单点击事件：点击后隐藏菜单+停止定时器
    connect(&m_menu, &QMenu::triggered, this, &HoverMenuButton::onMenuActionTriggered);

    // 按钮开启鼠标跟踪
    this->setMouseTracking(true);
    // 菜单开启鼠标跟踪
    enableMenuMouseTracking(&m_menu);
}

// 新增：给菜单开启鼠标跟踪
void HoverMenuButton::enableMenuMouseTracking(QMenu* menu)
{
    if (!menu) return;
    menu->setMouseTracking(true);
    // 子菜单递归开启（若有子菜单，可保留此逻辑）
    foreach(QAction * action, menu->actions()) {
        if (action->menu()) {
            enableMenuMouseTracking(action->menu());
            action->menu()->setMouseTracking(true);
        }
    }
}

// 原有构造函数不变
HoverMenuButton::HoverMenuButton(const QString& text, QWidget* parent)
    : QPushButton(text, parent)
{
    initMenu();
}

HoverMenuButton::HoverMenuButton(QWidget* parent)
    : QPushButton(parent)
{
    initMenu();
}

void HoverMenuButton::setMenuItemText(QString oldText, QString newText)
{
    if (oldText == u8"锁定")
    {
        m_menuItemWidgetActionMap[oldText]->setText(u8"解锁");
        m_menuItemLabelMap[oldText]->setText(u8"解锁");
    }
       
    else if (oldText == u8"解锁")
    {
        m_menuItemWidgetActionMap[oldText]->setText(u8"锁定");
        m_menuItemLabelMap[oldText]->setText(u8"锁定");
    }
       
    else if (oldText == u8"隐藏")
    {
        m_menuItemWidgetActionMap[oldText]->setText(u8"显示");
        m_menuItemLabelMap[oldText]->setText(u8"显示");
    }
       
    else if (oldText == u8"显示")
    {
        m_menuItemWidgetActionMap[oldText]->setText(u8"隐藏");
        m_menuItemLabelMap[oldText]->setText(u8"隐藏");
    }
       
}

// 修改：鼠标进入按钮，非阻塞显示菜单+启动定时器
void HoverMenuButton::enterEvent(QEvent* event)
{
    QPushButton::enterEvent(event);
    QPoint menuPos = calculateMenuPos();

    // 第一步：先移动菜单到目标位置（替代直接给show传参）
    m_menu.move(menuPos);
    // 第二步：调用无参show()显示菜单（非阻塞，不阻塞事件循环）
    m_menu.show();

    // 原有定时器启动逻辑（若已添加）保留不变
    m_checkTimer.start();
}

// 新增：检测鼠标位置，判断是否隐藏菜单
void HoverMenuButton::checkMousePosAndHideMenu()
{
    // 若鼠标不在按钮和菜单区域内，隐藏菜单+停止定时器
    if (!isMouseInButtonOrMenu()) {
        m_menu.hide();
        m_checkTimer.stop();
    }
}

// 新增：判断鼠标是否在按钮或菜单区域内
bool HoverMenuButton::isMouseInButtonOrMenu()
{
    // 1. 获取鼠标全局坐标
    QPoint globalMousePos = QCursor::pos();

    // 2. 正确计算按钮的全局矩形（修复核心：分步转换，不直接传QRect给mapToGlobal）
    // 步骤1：获取按钮局部矩形的左上角和右下角（局部坐标）
    QPoint btnLocalTopLeft = this->rect().topLeft(); // 局部左上角 (0,0)
    QPoint btnLocalBottomRight = this->rect().bottomRight(); // 局部右下角 (width-1, height-1)
    // 步骤2：将两个局部坐标转换为全局坐标
    QPoint btnGlobalTopLeft = this->mapToGlobal(btnLocalTopLeft);
    QPoint btnGlobalBottomRight = this->mapToGlobal(btnLocalBottomRight);
    // 步骤3：用两个全局坐标构建按钮的全局矩形
    QRect btnGlobalRect(btnGlobalTopLeft, btnGlobalBottomRight);

    // 判断鼠标是否在按钮全局矩形内
    if (btnGlobalRect.contains(globalMousePos)) {
        return true;
    }

    // 3. 判断鼠标是否在菜单区域内（原有逻辑不变，无需修改）
    if (m_menu.isVisible()) {
        // 菜单全局矩形也用同样的正确方式计算（可选，避免后续报错）
        QPoint menuLocalTopLeft = m_menu.rect().topLeft();
        QPoint menuLocalBottomRight = m_menu.rect().bottomRight();
        QPoint menuGlobalTopLeft = m_menu.mapToGlobal(menuLocalTopLeft);
        QPoint menuGlobalBottomRight = m_menu.mapToGlobal(menuLocalBottomRight);
        QRect menuGlobalRect(menuGlobalTopLeft, menuGlobalBottomRight);

        if (menuGlobalRect.contains(globalMousePos)) {
            return true;
        }
    }

    // 鼠标不在目标区域
    return false;
}

// 新增：菜单项点击后，隐藏菜单+停止定时器
void HoverMenuButton::onMenuActionTriggered(QAction* action)
{
    if (!action) return;
    m_menu.hide();
    m_checkTimer.stop();  
    m_txt = action->text();
    emit sig_Txt(m_txt);
}

// 保留菜单位置计算逻辑不变
QPoint HoverMenuButton::calculateMenuPos()
{
    QPoint btnGlobalPos = this->mapToGlobal(QPoint(0, 0));
    int menuX = btnGlobalPos.x() - 30;
    int menuY = btnGlobalPos.y() + this->height();
    return QPoint(menuX, menuY);
}
