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

class AddPackageDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddPackageDialog(QWidget *parent);

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
	Ui::FlatEdit * _pkgPinYinCodeEdit;
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
	int _package_type_id;
};
