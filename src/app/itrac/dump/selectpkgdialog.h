#ifndef SELECTPKGDIALOG_H
#define SELECTPKGDIALOG_H

#include <QDialog>
#include "ui_selectpkgdialog.h"

class QStandardItemModel;
class QNetworkReply;
class DeptPicker;
class SelectPkgDialog : public QDialog, Ui::SelectPkgDialog
{
	Q_OBJECT

public:
	explicit SelectPkgDialog(QWidget *parent = 0);
	~SelectPkgDialog();

protected:
	void accept();

signals:
	void packageSelected(const QString &, int, const QString &, int);

private slots:
	void searchDept();
	void onDeptChanged(int id, const QString &name);

private:
	QStandardItemModel * _depModel;
	QStandardItemModel * _pkgModel;
	DeptPicker *_searchDeptWidget;
	int _currentDeptId;
};

#endif // !SELECTPKGDIALOG_H
