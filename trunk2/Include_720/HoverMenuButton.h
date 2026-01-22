#ifndef HOVERMENUBUTTON_H
#define HOVERMENUBUTTON_H

#include <QPushButton>
#include <QMenu>
#include <QPoint>
#include <QEvent>
#include <QTimer> // 新增：定时器头文件
#include <QCursor> // 新增：鼠标位置获取头文件
#include <QLabel>
#include <QWidgetAction>

class HoverMenuButton : public QPushButton
{
    Q_OBJECT
public:
    explicit HoverMenuButton(const QString& text, QWidget* parent = nullptr);
    explicit HoverMenuButton(QWidget* parent = nullptr);
    QMenu* getMenu() { return &m_menu; }
    //修改菜单文字
    void setMenuItemText(QString, QString);
protected:
    void enterEvent(QEvent* event) override;
    // 注释/删除 leaveEvent：不再依赖它隐藏菜单
    // void leaveEvent(QEvent* event) override;
signals:
    void sig_Txt(QString);

private slots:
    // 新增：定时器触发的鼠标位置检测槽函数
    void checkMousePosAndHideMenu();
    // 新增：菜单项点击后隐藏菜单
    void onMenuActionTriggered(QAction* action);

private:
    QString m_txt;
    QMenu m_menu;
    QMap<QString, QWidgetAction*> m_menuItemWidgetActionMap;
    QMap<QString, QLabel*> m_menuItemLabelMap;
    QTimer m_checkTimer; // 新增：鼠标位置检测定时器
    QPoint calculateMenuPos();
    void initMenu();
    // 新增：判断鼠标是否在按钮或菜单区域内
    bool isMouseInButtonOrMenu();
    // 新增：给菜单开启鼠标跟踪
    void enableMenuMouseTracking(QMenu* menu);
};

#endif // HOVERMENUBUTTON_H