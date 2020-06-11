#include "scanTipsdialog.h"
#include "barcode.h"
#include "core/net/url.h"
#include <QtWidgets/QtWidgets>

ScanTipsDialog::ScanTipsDialog(const QString &msg, QWidget *parent)
	: QDialog(parent)
	, _tipsLabel(new QLabel)
{
	_tipsLabel->setText(msg);

	QGridLayout *gLayout = new QGridLayout(this);
	gLayout->addWidget(_tipsLabel);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
}

void ScanTipsDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package || tc.type() == TranspondCode::Instrument)
	{
		_code = code;

		return QDialog::accept();
	}
}

void ScanTipsDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}