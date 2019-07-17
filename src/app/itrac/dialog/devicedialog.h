#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class QStandardItemModel;
class TableView;
class WaitingSpinner;
class AddDeviceDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddDeviceDialog(QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

protected slots:
	void toggleCheckSate(const QModelIndex&);

private:
	void initProgramView();
	void loadPrograms(int);

	Ui::FlatEdit *_nameEdit;
	QComboBox *_typeCombo;
	TableView *_view;
	QStandardItemModel *_model;
	WaitingSpinner *_waiter;
};

struct Device;

class ModifyDeviceDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	ModifyDeviceDialog(Device *device, QWidget *parent = Q_NULLPTR);

protected:
	void accept() override;

protected slots:
	void toggleCheckSate(const QModelIndex&);

private:
	void initProgramView();
	void loadPrograms();
	void selectPrograms();

	Device *_device;
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_typeEdit;
	TableView *_view;
	QStandardItemModel *_model;
	WaitingSpinner *_waiter;
};
