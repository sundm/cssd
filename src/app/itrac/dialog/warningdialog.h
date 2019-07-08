#pragma once

#include <QDialog>

class TableView;
class QStandardItemModel;
class WarningDialog : public QDialog
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
