#pragma once

#include <QDialog>
#include <QNetworkReply>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;
class WaitingSpinner;
class XPicture;
class QHttpMultiPart;
class QFile;

class AddInstrumentDialog : public QDialog
{
	Q_OBJECT

public:
	AddInstrumentDialog(QWidget *parent = Q_NULLPTR);
	void setInfo(const QString &id);
protected:
	void accept() override;

private slots:
	void loadImg();
	void imgError(QNetworkReply::NetworkError);
	void imgUploaded();

private:
	Ui::FlatEdit *_nameEdit;
	Ui::FlatEdit *_pinyinEdit;
	QCheckBox *_checkVIPBox;
	//QCheckBox *_checkImplantBox;
	WaitingSpinner *_waiter;

	bool _isModify;
	QString _instrumentId;

	XPicture* _imgLabel;
	QString _imgFilePath;
	QFile *_imgFile;

	bool copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist);
	void uploadImg();
	const QString getFileMd5(const QString &filePath);
};
