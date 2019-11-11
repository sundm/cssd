#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
	class NormalLabel;
	class PrimaryButton;
}

class TableView;
class QSpinBox;
class QComboBox;
class DeptEdit;
class InstrumentEdit;
class XPicture;
class QFile;
class QStandardItemModel;
class QItemSelectionModel;
class QHttpMultiPart;

struct PackageInfo
{
	QString package_rfid;
	QString package_type_id;
	QString package_name;			
	QString dtp_id;
	QString dtp_name;
	QString pack_type;
	QString package_category;
	//QString package_pinyin;
	int steType;

};

class ModifyPackageDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	ModifyPackageDialog(QWidget *parent, const PackageInfo info, const QList<QVariant> orders);

protected:
	void accept() override;

private slots:
	void addEntry();
	void removeEntry();
	void loadImg();

private:
	void initInstrumentView();
	void initData();
	void loadData();
	int findRow(int insId);
	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg();

	Ui::FlatEdit * _pkgNameEdit;
	Ui::FlatEdit * _pkgRFIDCodeEdit;
	QComboBox * _pkgtypeBox;
	QComboBox * _picktypeBox;
	QComboBox * _stertypeBox;
	DeptEdit * _deptEdit;
	InstrumentEdit * _insEdit;

	
	TableView *_view;
	QItemSelectionModel *_theSelectionModel;
	QStandardItemModel *_model;

	XPicture* _imgLabel;
	Ui::PrimaryButton *_loadImgButton;

	Ui::PrimaryButton *_commitButton;

	QHttpMultiPart *_multiPart;
	QFile *_imgFile;
	QString _imgFilePath;
	PackageInfo _info;
	QList<QVariant> _orders;
	int _package_type_id;
};
