#pragma once

#include <QWidget>
#include "ui_project_edit_resee.h"
#include "PanoramaWidget.h"
#include <QPushButton>
#include <QOpenGLWidget>
#include "project_edit_group_item.h"
#include "project_style_picText.h"
class project_edit_resee : public QWidget
{
	Q_OBJECT

public:
	project_edit_resee(QString projUUID, QStringList listPicPath, std::map<QString, QString> mapPicPathUUid, QWidget *parent = nullptr);
	~project_edit_resee();

	void processNextPicItem();
	void processNextHpItem();
	void processNextGroupItem();
	void Reload();
	void clearListWidgetGroup();
public slots:
	void slotClose();
	void slotFullScreen();
	void slotNewPicClick(const QString& picPath);
	void slotSetAngleZoom(QString);
	void slotSetSelStyle(QString groupID);
	void Slot_autoRotate(int);
	void SlotShowElement(bool bl);
	void slotStyle_picText(QString);
	void slotStyle_picChange(QString);
private:
	project_style_picText* m_picText;
	QString m_projUUID;
	QStringList m_listPicPath;
	QString m_picPath;
	PanoramaWidget* m_panWidget;

	bool m_isFull;

	QVector3D m_rotation;
	float m_CAngle;
	float m_zoom;

	QString m_DefGroup = "";//默认选中分组

	int m_currentPicItemIndex;
	int m_currentHpItemIndex;

	std::vector<Hotspot> m_vecCurPicHp;
	QString m_cutGroupID = "";//当前选择的分组

	std::map<QString, QString> m_mapUUidPicPath;
	std::map<QString, QString> m_mapPicPathUUid;

	int m_curInsertGroupNum;
	std::map<QString, std::vector<HotspotGroupShow>> m_mapVecGroup;
	std::vector<QString> m_vecGroupID;
	std::vector<GroupItemStyle> m_vecGroupStyle;



	Ui::project_edit_reseeClass ui;
};