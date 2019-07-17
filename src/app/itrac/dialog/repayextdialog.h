#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
	class NormalLabel;
}

class QSpinBox;
class FormGroup;
class PackageEdit;
class VendorComboBox;
class GenderComboBox;
class QStandardItem;
class QStandardItemModel;
class TableView;
class WaitingSpinner;
class ExtRepayDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

	enum Column {
		Name,
		Doctor,
		Patient,
		Identifier,
		Age,
		Gender,
		Barcode
	};

public:
	ExtRepayDialog(const QVariantMap *map, QWidget *parent);

protected:
	void accept() override;

private:
	void load();

	Ui::NormalLabel * _vendor;
	Ui::NormalLabel * _sender;
	Ui::NormalLabel * _senderPhone;
	Ui::NormalLabel * _receiver;
	Ui::NormalLabel * _receiveTime;

	Ui::NormalLabel * _repaier;
	Ui::FlatEdit * _repayEdit;
	Ui::FlatEdit * _repayPhoneEdit;
	Ui::NormalLabel * _repayTime;

	TableView *_view;
	QStandardItemModel *_model;
	WaitingSpinner *_waiter;
	int _orderId;
};
