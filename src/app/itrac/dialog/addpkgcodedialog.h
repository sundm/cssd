#pragma once

#include <QDialog>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
	class PrimaryButton;
}

class TableView;
class QStandardItemModel;
class QItemSelectionModel;

class AddpkgcodeDialog : public QDialog, public Scanable, public JsonHttpClient
{
	Q_OBJECT

public:
	AddpkgcodeDialog(QWidget *parent, const QString& pkg_name, const QString& pkg_id);

protected:
	void accept() override;
	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void removeEntry();

private:
	void initInstrumentView();
	void initData();
	int findRow(int code);

	Ui::FlatEdit * _pkgNameEdit;
	Ui::FlatEdit * _pkgCodeEdit;

	Ui::PrimaryButton *_commitButton;
	
	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;

	QString _package_type_id;
};
