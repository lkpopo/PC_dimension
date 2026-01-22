#include "project_edit_group_item.h"

project_edit_group_item::project_edit_group_item(GroupItemStyle gpStyle, QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

    m_itemStyle = gpStyle;
    if (m_itemStyle.unsel_bgColor == "")
        m_itemStyle.unsel_bgColor = "26,26,26";
    if (m_itemStyle.unsel_textColor == "")
        m_itemStyle.unsel_textColor = "255,255,255";
    if (m_itemStyle.sel_bgColor == "")
        m_itemStyle.sel_bgColor = "250,100,0";
    if (m_itemStyle.sel_textColor == "")
        m_itemStyle.sel_textColor = "255,255,255";

	ui.pushButton_HPGroup->setText(m_itemStyle.groupName);

    ui.pushButton_HPGroup->setStyleSheet(
        QString("QPushButton{"
        "background-color: rgba(%1,120);"
        "color: rgb(%2);"
        "border-radius: 5px;"
        "border: 0px;"
        "}").arg(m_itemStyle.unsel_bgColor).arg(m_itemStyle.unsel_textColor));
    
}

project_edit_group_item::~project_edit_group_item()
{}

void project_edit_group_item::setText(QString groupName)
{
    ui.pushButton_HPGroup->setText(groupName);
}

void project_edit_group_item::clickEvent()
{
    //µã»÷
    connect(ui.pushButton_HPGroup, SIGNAL(clicked()), this, SLOT(slotHPGroup()));
}

void project_edit_group_item::clickButton()
{
    ui.pushButton_HPGroup->click();
}

void project_edit_group_item::setSelStatus(bool bl)
{
    m_isSelStatus = bl;
}

void project_edit_group_item::slotHPGroup()
{
    m_isSelStatus = !m_isSelStatus;
    if (m_isSelStatus == true)
    {
        slotSelHPGroup();
        emit itemClicked(m_itemStyle.groupID);
    }
    else
    {
        slotUnSelHPGroup();
        emit itemClicked("");
    }
}

void project_edit_group_item::slotSelHPGroup()
{
    ui.pushButton_HPGroup->setStyleSheet(
        QString("QPushButton{"
            "background-color: rgb(%1);"
            "color: rgb(%2);"
            "border-radius: 5px;"
            "border: 0px;"
            "}").arg(m_itemStyle.sel_bgColor).arg(m_itemStyle.sel_textColor));
}

void project_edit_group_item::slotUnSelHPGroup()
{
    ui.pushButton_HPGroup->setStyleSheet(
        QString("QPushButton{"
            "background-color: rgba(%1,120);"
            "color: rgb(%2);"
            "border-radius: 5px;"
            "border: 0px;"
            "}").arg(m_itemStyle.unsel_bgColor).arg(m_itemStyle.unsel_textColor));
}
