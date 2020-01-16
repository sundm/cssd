#pragma once

#include <QDialog>
#include "rdao/entity/device.h"

namespace Ui {
	class FlatEdit;
}

class QComboBox;
class QTextEdit;
class QSpinBox;
class TableView;
class QStandardItemModel;

class AddRecallDialog : public QDialog
{
	Q_OBJECT

public:
	enum { DeviceName, BatchId, Date, Cycle, PackageNum };
	AddRecallDialog(QWidget *parent = Q_NULLPTR);
protected:
	void accept() override;

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void onRowDoubleClicked(const QModelIndex &);
	void onToggled(int, bool);
	void onDeviceChanged(int);
	void onCycleChanged(int);
private:
	void startTrace();
	void initDevice();
	void clear();
	Ui::FlatEdit *_pkgUDIEdit;
	QComboBox * _deviceBox;
	QSpinBox * _cycleBox;
	QTextEdit *_reasonEdit;

	TableView * _view;
	QStandardItemModel *_model;
	SterBatchInfo _sbi;
	int _devId;
	QFont _font;
	bool _byPkg;
};
