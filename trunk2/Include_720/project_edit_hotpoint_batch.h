#pragma once

#include <QWidget>
#include "ui_project_edit_hotpoint_batch.h"
#include "PanoramaWidget.h"

class project_edit_hotpoint_batch : public QWidget
{
	Q_OBJECT

public:
	project_edit_hotpoint_batch(QString projUUID, QString picPath, QString picID, QWidget *parent = nullptr);
	~project_edit_hotpoint_batch();

	void addIcon2List(QString picID);
	void processNextIconItem();
	void enableBatch(bool);
	std::vector<QString> getSelIconID();

public slots:
	void slotClose();
	void SlotSelList(int);
	void slotSelStatus(int);
	void slotOperationShowHide();
	void slotOperationShow();
	void slotOperationHide();

	void slotHide();
	void slotShow();
	void slotLock();
	void slotUnlock();
	void slotDel();
	void slotclearHpList();
	void slotSearch(const QString&);
	void slotUpdateHotpointList(QVector<Hotspot>& filterHpList);
	void slotClearSearchLine();

signals:
	void sig_closeSet();
	void sig_edit_hp_batch_RotateCutHotPoint(QString);
	void sig_batch_setItemShow_Lock(QString, QString);
	void sig_batch_edit_hp_DelIcon(QString);
	void sig_batch_ReloadHp(QString);
	void sig_UpdateHotpointList(QVector<Hotspot>&);

private:
	QString m_projUUID;
	QString m_cutPicPath;
	QString m_picID;

	bool m_operationMenuShow = false;//ƒ¨»œ≤ªœ‘ æ

	int m_curInsertNum;
	std::vector<Hotspot> m_vecCurPicHp;

	Ui::project_edit_hotpoint_batchClass ui;
};

