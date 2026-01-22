#include "project_edit_hotpoint_group_ResetGroupStyle.h"
#include <QScreen>
#include "CommonTool.h"
#include "styleSheet.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QListView>
#include <QColorDialog>

project_edit_hotpoint_group_ResetGroupStyle::project_edit_hotpoint_group_ResetGroupStyle(QString projUUID, QString picID, QWidget *parent)
    :m_projUUID(projUUID), m_picID(picID), QWidget(parent)
{
	ui.setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_QuitOnClose, true);
    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenRect = screen->geometry();
    int scrHeight = screenRect.height() - getBottomPix();
    //动态长度
    {
        resize(250, scrHeight - 60);
        ui.pushButton_done->move(10, scrHeight - 60 - 10 - 30);
    }

    m_group_unsel_bg_color = QColor(26, 26, 26);
    m_group_unsel_text_color = QColor(255, 255, 255);
    m_group_sel_bg_color = QColor(250, 100, 0);
    m_group_sel_text_color = QColor(255, 255, 255);

    QString data = QString("select group_unsel_bg_color, group_unsel_text_color, group_sel_bg_color, group_sel_text_color from picture where id = '%1' LIMIT 1").arg(m_picID);
    QSqlQuery query(data);//查询表的内容
    while (query.next())
    {
        QString unsel_bg_color = query.value(0).toString();
        QString unsel_text_color = query.value(1).toString();
        QString sel_bg_color = query.value(2).toString();
        QString sel_text_color = query.value(3).toString();

        QStringList unsel_bg_colorList = unsel_bg_color.split(",");
        QStringList unsel_text_colorList = unsel_text_color.split(",");
        QStringList sel_bg_colorList = sel_bg_color.split(",");
        QStringList sel_text_colorList = sel_text_color.split(",");

        if (unsel_bg_colorList.size() == 3)
            m_group_unsel_bg_color = QColor(unsel_bg_colorList[0].toInt(), unsel_bg_colorList[1].toInt(), unsel_bg_colorList[2].toInt());
        if (unsel_text_colorList.size() == 3)
            m_group_unsel_text_color = QColor(unsel_text_colorList[0].toInt(), unsel_text_colorList[1].toInt(), unsel_text_colorList[2].toInt());
        if (sel_bg_colorList.size() == 3)
            m_group_sel_bg_color = QColor(sel_bg_colorList[0].toInt(), sel_bg_colorList[1].toInt(), sel_bg_colorList[2].toInt());
        if (sel_text_colorList.size() == 3)
            m_group_sel_text_color = QColor(sel_text_colorList[0].toInt(), sel_text_colorList[1].toInt(), sel_text_colorList[2].toInt());
    }

    ui.pushButton_unsel_bg_color->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_group_unsel_bg_color.red()).arg(m_group_unsel_bg_color.green()).arg(m_group_unsel_bg_color.blue()));
    ui.pushButton_unsel_text_color->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_group_unsel_text_color.red()).arg(m_group_unsel_text_color.green()).arg(m_group_unsel_text_color.blue()));
    ui.pushButton_sel_bg_color->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_group_sel_bg_color.red()).arg(m_group_sel_bg_color.green()).arg(m_group_sel_bg_color.blue()));
    ui.pushButton_sel_text_color->setStyleSheet(QString("QPushButton{background-color: rgb(%1, %2, %3)};") \
        .arg(m_group_sel_text_color.red()).arg(m_group_sel_text_color.green()).arg(m_group_sel_text_color.blue()));

    connect(ui.pushButton_unsel_bg_color, SIGNAL(clicked()), this, SLOT(slot_unsel_bg_color()));
    connect(ui.pushButton_unsel_text_color, SIGNAL(clicked()), this, SLOT(slot_unsel_text_color()));
    connect(ui.pushButton_sel_bg_color, SIGNAL(clicked()), this, SLOT(slot_sel_bg_color()));
    connect(ui.pushButton_sel_text_color, SIGNAL(clicked()), this, SLOT(slot_sel_text_color()));

    connect(ui.pushButton_done, SIGNAL(clicked()), this, SLOT(slotDone()));
}

project_edit_hotpoint_group_ResetGroupStyle::~project_edit_hotpoint_group_ResetGroupStyle()
{}

void project_edit_hotpoint_group_ResetGroupStyle::slot_unsel_bg_color()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_group_unsel_bg_color.red(), m_group_unsel_bg_color.green(), m_group_unsel_bg_color.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_group_unsel_bg_color = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_unsel_bg_color->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
}

void project_edit_hotpoint_group_ResetGroupStyle::slot_unsel_text_color()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_group_unsel_text_color.red(), m_group_unsel_text_color.green(), m_group_unsel_text_color.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_group_unsel_text_color = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_unsel_text_color->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
}

void project_edit_hotpoint_group_ResetGroupStyle::slot_sel_bg_color()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_group_sel_bg_color.red(), m_group_sel_bg_color.green(), m_group_sel_bg_color.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_group_sel_bg_color = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_sel_bg_color->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
}

void project_edit_hotpoint_group_ResetGroupStyle::slot_sel_text_color()
{
    QColorDialog dlg;
    dlg.setWindowTitle(u8"选择颜色");
    dlg.setStyleSheet("color: rgb(8, 20, 40);");
    dlg.setWindowIcon(QIcon(QString(u8"%1/Resource/edit/colorBoard.png").arg(QApplication::applicationDirPath())));
    dlg.setCurrentColor(QColor(m_group_sel_text_color.red(), m_group_sel_text_color.green(), m_group_sel_text_color.blue()));
    if (dlg.exec() == QDialog::Accepted)
    {
        QColor color = dlg.selectedColor();
        m_group_sel_text_color = QColor(color.red(), color.green(), color.blue());
        ui.pushButton_sel_text_color->setStyleSheet(QString("QPushButton{background-color: rgba(%1, %2, %3, %4)};") \
            .arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha()));
    }
}

void project_edit_hotpoint_group_ResetGroupStyle::slotDone()
{
    QString unsel_bg_color = QString::number(m_group_unsel_bg_color.red()) + ","
        + QString::number(m_group_unsel_bg_color.green()) + ","
        + QString::number(m_group_unsel_bg_color.blue());
    QString unsel_text_color = QString::number(m_group_unsel_text_color.red()) + ","
        + QString::number(m_group_unsel_text_color.green()) + ","
        + QString::number(m_group_unsel_text_color.blue());
    QString sel_bg_color = QString::number(m_group_sel_bg_color.red()) + ","
        + QString::number(m_group_sel_bg_color.green()) + ","
        + QString::number(m_group_sel_bg_color.blue());
    QString sel_text_color = QString::number(m_group_sel_text_color.red()) + ","
        + QString::number(m_group_sel_text_color.green()) + ","
        + QString::number(m_group_sel_text_color.blue());

    QString updata = QString("update picture set \
                              group_unsel_bg_color = '%1', group_unsel_text_color = '%2',group_sel_bg_color = '%3',group_sel_text_color = '%4'where project_name_id = '%5'")
        .arg(unsel_bg_color).arg(unsel_text_color).arg(sel_bg_color).arg(sel_text_color).arg(m_projUUID);
    QSqlQuery query;
    query.exec(updata);
}

