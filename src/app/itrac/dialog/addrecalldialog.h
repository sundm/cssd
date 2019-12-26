#pragma once

#include <QDialog>

namespace Ui {
	class FlatEdit;
}

struct RecallInfo {
	int deviceId;
	QString deviceName;
	int cycle;
	int pkgNums;
	QString reason;
};

class QComboBox;
class QTextEdit;
class QSpinBox;
class TableView;
class QStandardItemModel;

class AddRecallDialog : public QDialog
{
	Q_OBJECT

public:
	enum { UDI, Name, Dept };
	AddRecallDialog(QWidget *parent = Q_NULLPTR);
protected:
	void accept() override;

signals:
	void addRecall(const RecallInfo&);

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void onToggled(int, bool);
	void onDeviceChanged(int);
	void onCycleChanged(int);
private:
	void startTrace();
	void initDevice();

	Ui::FlatEdit *_pkgUDIEdit;
	QComboBox * _deviceBox;
	QSpinBox * _cycleBox;
	QTextEdit *_reasonEdit;

	TableView * _view;
	QStandardItemModel *_model;

	QFont _font;
	bool _byPkg;
};
