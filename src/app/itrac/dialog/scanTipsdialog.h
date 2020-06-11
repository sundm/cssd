#pragma once

#include <QDialog>
#include <qscanner/qscanner.h>

class QLabel;

class ScanTipsDialog : public QDialog
{
	Q_OBJECT

public:
	ScanTipsDialog(const QString &msg, QWidget *parent);
	const QString &getCode() { return _code; };

private slots:
	void onTransponderReceviced(const QString& code);
	void onBarcodeReceviced(const QString& code);

private:
	QLabel *_tipsLabel;
	QString _code;
};