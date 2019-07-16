#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
	class NormalLabel;
}

class QLabel;
class QSpinBox;
class FormGroup;
class PackageEdit;
class VendorComboBox;
class GenderComboBox;
class QStandardItem;
class QItemSelectionModel;
class QStandardItemModel;
class TableView;

class ExtRecvDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	ExtRecvDialog(QWidget *parent);

protected:
	void accept() override;

private slots:
	void slotRowDoubleClicked(const QModelIndex &index);

private:
	void initTableView();
	void loadData();

	void add();
	void remove();

	VendorComboBox * _vendorCombo;
	Ui::FlatEdit * _senderEdit;
	Ui::FlatEdit * _senderPhoneEdit;
	Ui::NormalLabel * _receiverEdit;
	Ui::NormalLabel *_receiveTimeLabel;

	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;
};
