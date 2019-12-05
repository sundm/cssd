#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
	class NormalLabel;
	class PrimaryButton;
}

class TableView;
class PackageEdit;
class QDateTimeEdit;
class QSpinBox;
class QComboBox;
class DeptEdit;
class InstrumentEdit;
class XPicture;
class QFile;
class QStandardItemModel;
class QItemSelectionModel;
class QHttpMultiPart;

class AddOperatinDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddOperatinDialog(QWidget *parent);
	void setInfo(const QString& operationId);

protected:
	void accept() override;

private slots:
	void addEntry();
	void removeEntry();

private:
	void initData();
	void initPackageInfo();

	void initInstrumentView();
	int findRow(int insId);

	Ui::FlatEdit * _operIdEdit;
	Ui::FlatEdit * _operRoomEdit;
	QDateTimeEdit * _operTimeEdit;
	Ui::FlatEdit * _operNameEdit;
	Ui::FlatEdit * _patientIdEdit;
	Ui::FlatEdit * _patientNameEdit;
	PackageEdit * _packageEdit;

	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;
	QList<QVariant> _orders;

	Ui::PrimaryButton *_commitButton;
	bool _isModfy;
	int _operation_id;
};
