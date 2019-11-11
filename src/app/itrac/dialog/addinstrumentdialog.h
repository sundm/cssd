#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class WaitingSpinner;
class XPicture;
class QHttpMultiPart;
class QFile;

class AddInstrumentDialog : public QDialog, public JsonHttpClient
{
	Q_OBJECT

public:
	AddInstrumentDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString &id, const QString &name, const bool isVIP, const bool isImplant);
protected:
	void accept() override;

private slots:
	void loadImg();
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_rfidEdit;
	QCheckBox *_checkVIPBox;
	QCheckBox *_checkImplantBox;
	WaitingSpinner *_waiter;

	bool _isModify;
	QString _instrumentId;

	QHttpMultiPart *_multiPart;
	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;

	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg(int instrument_id);
};
