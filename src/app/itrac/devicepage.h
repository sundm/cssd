#pragma once

#include <QWidget>
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class QComboBox;
class TableView;
class DevicePage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	DevicePage(QWidget *parent = Q_NULLPTR);
	~DevicePage();
	
private slots:
	void onFilterChanged(int index);
	void showDeviceItemContextMenu(const QPoint&);
	void lockDevice();
	void unlockDevice();
	void modifyDevice();

private:
	void initDeviceView();
	void updateDeviceView(const QString& deviceType = QString());
	void setDeviceEnabled(bool);
	
	void refresh();
	void add();
	void modify();
	void disable();

private:
	TableView *_deviceView;
	QStandardItemModel * _deviceModel;
	QComboBox *_filterComboBox;
};
