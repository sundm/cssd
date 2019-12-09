#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"
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

class AddPackageDialog : public QDialog, public JsonHttpClient
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
private:
	void initData();
	void initPackageInfo();

	void initInstrumentView();
	void getOrders();
	int findRow(int insId);

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
};
