#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

class TableView;
class QStandardItemModel;
class WarningDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	enum {PackageId, PackageName, PackType, ExpireDate, LeftDays, DeptName, DeptPhone};
	WarningDialog(QWidget *parent);
	~WarningDialog();

private:
	void load();

	TableView * _view;
	QStandardItemModel *_model;
};
