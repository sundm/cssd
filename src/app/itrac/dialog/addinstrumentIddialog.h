#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class InstrumentEdit;
class QCheckBox;
class WaitingSpinner;
class XPicture;
class QHttpMultiPart;
class QFile;

class AddInstrumentIdDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddInstrumentIdDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString &id);
protected:
	void accept() override;
signals:
	void reload();

private slots:
	void loadImg();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);
private:
	void resetView();

	Ui::FlatEdit *_idEdit;
	Ui::FlatEdit *_aliasEdit;
	InstrumentEdit * _insEdit;
	WaitingSpinner *_waiter;

	bool _isModify;
	QString _instrumentId;

	QHttpMultiPart *_multiPart;
	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;

	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg(const QString&);
};
