#pragma once

#include <QDialog>
#include <QNetworkReply>
#include "rdao/dao/PackageDao.h"

namespace Ui {
	class FlatEdit;
	class NormalLabel;
	class PrimaryButton;
}

class TableView;
class QCheckBox;
class QSpinBox;
class QComboBox;
class DeptEdit;
class InstrumentEdit;
class XPicture;
class QFile;
class QStandardItemModel;
class QItemSelectionModel;
class QHttpMultiPart;

class AddPackageDialog : public QDialog
{
	Q_OBJECT

public:
	AddPackageDialog(QWidget *parent);
	void setInfo(const QString& pkg_type_id);

protected:
	void accept() override;

private slots:
	void addEntry();
	void removeEntry();
	void typeBoxChanaged(int);

	void loadImg();
	void imgError(QNetworkReply::NetworkError);
	void imgUploaded();
private:
	void initData();
	void initPackageInfo();

	void initInstrumentView();
	void getOrders();
	int findRow(int insId);

	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg();
	const QString getFileMd5(const QString &filePath);

	Ui::FlatEdit * _pkgNameEdit;
	Ui::FlatEdit * _pkgPYCodeEdit;
	QComboBox * _pkgtypeBox;
	QComboBox * _picktypeBox;
	QCheckBox * _importBox;
	QComboBox * _stertypeBox;
	DeptEdit * _deptEdit;
	InstrumentEdit * _insEdit;

	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;
	QList<PackageType::DetailItem> _orders;

	Ui::PrimaryButton *_commitButton;
	bool _isModfy;
	int _package_type_id;

	Ui::PrimaryButton *_loadImgButton;
	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;
};
