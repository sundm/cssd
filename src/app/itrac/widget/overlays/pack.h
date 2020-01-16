#pragma once

#include "cssd_overlay.h"
#include <QNetworkReply>
#include "rdao/entity/package.h"

class DeptEdit;
class PackageEdit;
class QSpinBox;
class TableView;
class XPicture;
class UnusualInstrumentView;
class PackageInfoView;
class PackageDetailView;
class QStandardItemModel;

class PackPanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	PackPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
	void reprint();
	void abnormal();
	void reset();

	void onScanned(const QString& code);
	void onUnusual(const QString& code);
	void loadInstrumentImg(const QString& udi);

	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

	void imgInsClicked();
	void imgPkgClicked();

	void imgError(QNetworkReply::NetworkError);
	void imgLoaded();
private:
	const QString getFileMd5(const QString &filePath);
	void loadPackageImg(const QString& udi);

	PackageInfoView * _pkgView;
	PackageDetailView * _detailView;
	UnusualInstrumentView * _unusualView;

	QStringList * _scannedCodes;
	QStringList * _unusualCodes;

	Package _package;

	XPicture * _pkgImg;
	XPicture * _insImg;
	int _row;
	int _step;

	int _imgType;
	QString _imgFilePath;
};
