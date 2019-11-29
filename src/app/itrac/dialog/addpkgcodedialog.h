#pragma once

#include <QDialog>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"
#include "rdao/dao/PackageDao.h"

namespace Ui {
	class FlatEdit;
	class PrimaryButton;
}

class PackageEdit;
class XPicture;
class QHttpMultiPart;
class QFile;
class WaitingSpinner;
class TableView;
class QStandardItemModel;
class QItemSelectionModel;

class AddpkgcodeDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddpkgcodeDialog(QWidget *parent);
	void setPackageId(const QString &pkgId);

protected:
	void accept() override;

private slots:
	void onPackageTypeChange(int);
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
	void loadImg();
	void reset();
private:
	void initInstrumentView();
	
	int findRow(int code);
	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg(const QString& instrument_id);

	bool loadInstrumentType(int);
	void loadPackageInfo();

	const QList<Instrument> getInstruments();

	Ui::FlatEdit * _pkgNameEdit;
	Ui::FlatEdit * _pkgCodeEdit;
	PackageEdit *_pkgEdit;
	Ui::FlatEdit * _insRfidEdit;
	Ui::PrimaryButton *_loadImgButton;

	Ui::PrimaryButton *_commitButton;
	Ui::PrimaryButton *_resetButton;
	WaitingSpinner *_waiter;
	
	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;

	TableView *_unview;
	QStandardItemModel *_unmodel;

	QHttpMultiPart *_multiPart;
	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;

	QString _package_id;

	QStringList _codeScanedList;
	QStringList _codeUnusualList;

	QList<Instrument> _insList;
	int _step;
	int _code;
	bool _isModify;
};