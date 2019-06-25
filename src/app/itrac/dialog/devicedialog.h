#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class QStandardItemModel;
class TableView;

class AddDeviceDialog : public QDialog
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
};

struct Device;

class ModifyDeviceDialog : public QDialog
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
};
