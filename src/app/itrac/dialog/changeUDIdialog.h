#pragma once

#include <QDialog>
#include <QNetworkReply>
#include <qscanner/qscanner.h>
#include "rdao/dao/PackageDao.h"

namespace Ui {
	class FlatEdit;
	class PrimaryButton;
}

class QLabel;
class PackageEdit;
class WaitingSpinner;
class TableView;
class QStandardItemModel;
class QItemSelectionModel;

class ChangeUDIDialog : public QDialog
{
	Q_OBJECT

public:
	ChangeUDIDialog(QWidget *parent);

protected:
	void accept() override;

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void showItemContextMenu(const QPoint&);
	void onChangeInsUDI();
	void onChangePkgUDIBtn();
	void onCheckInsBtn();
	void reset();
private:
	void initInstrumentView();
	void onLoadPackage(const QString &code);
	void onLoadPackageInfo(const QString& code);

	QLabel * _pkgCodeLabel;
	QLabel * _pkgTypeLabel;

	Ui::PrimaryButton *_loadPkgUDIButton;
	Ui::PrimaryButton *_checkInsButton;
	Ui::PrimaryButton *_commitButton;
	Ui::PrimaryButton *_resetButton;
	WaitingSpinner *_waiter;
	
	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;

	QString _package_id_old;
	QString _package_id_new;

	QMap<QString, QString> _insNewMap;
	QMap<QString, int> _insOldMap;
	int _step;
	int _row;
};