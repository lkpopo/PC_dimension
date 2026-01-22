#pragma once
#ifndef MSGBOX_H
#define MSGBOX_H

#include <QPushButton>
#include <QLabel>
#include <QMouseEvent>
#include<QDialog>
#include<QTimer>
#include<QDebug>
#include <windows.h>
#include <QWidget>
class MsgBox : public QDialog
{
    Q_OBJECT
public:
    MsgBox(int style, QString text, QWidget* parent = 0);
    ~MsgBox();
public:
    QPushButton* okBtn;
    QPushButton* cancleBtn;
    QPushButton* closeBtn;
    QPushButton* msgBtn;
    QLabel* titleLabel;
    QLabel* askLabel;
    QLabel* bkgLabel;
protected:
    QPoint move_point;                                    //移动的距离
    bool mouse_press;                                    //鼠标按下
    void mousePressEvent(QMouseEvent* qevent);            //鼠标按下事件
    void mouseReleaseEvent(QMouseEvent* qevent);         //鼠标释放事件
    void mouseMoveEvent(QMouseEvent* qevent);             //鼠标移动事件

public slots:
    void okBtn_press();
    void cancleBtn_press();
    void closeBtn_press();
    void closeWin();
};
#endif // MSGBOX_H

