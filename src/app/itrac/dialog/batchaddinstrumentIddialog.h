#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class InstrumentEdit;
class QCheckBox;
class QSpinBox;
class TableView;
class QStandardItemModel;
class QItemSelectionModel;
class WaitingSpinner;
class XPicture;
class QHttpMultiPart;
class QFile;

class BatchAddInstrumentIdDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	BatchAddInstrumentIdDialog(QWidget *parent = Q_NULLPTR);
protected:
	void accept() override;

signals:
	void reload();

private slots:
	void loadImg();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void onInsTypeChange(int type);
private:
	InstrumentEdit * _insEdit;

	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;

	WaitingSpinner *_waiter;

	QHttpMultiPart *_multiPart;
	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;

	QStringList _scannedList;

	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg(const QString&);
};
