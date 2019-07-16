#pragma once

#include <QWidget>
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class QComboBox;
class TableView;
class DeviceProgramPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	DeviceProgramPage(QWidget *parent = Q_NULLPTR);

private slots:
	void onFilterChanged(int index);
	void showProgramItemContextMenu(const QPoint&);
	void refresh();
	void add();
	void modify();

private:
	void initProgramView();
	void updateProgramView(const QString& deviceType = QString());

private:
	QComboBox *_filterComboBox;
	TableView *_view;
	QStandardItemModel * _programModel;
};
