#include <QButtonGroup>
#include <qscanner/qscanner.h>
#include "dailyhome.h"
#include "Barcode.h"
#include "core/itracnamespace.h"

DailyHome::DailyHome(QWidget *parent)
    :QScrollArea(parent)
{
	setupUi(this);

	//recycleToolButton->setText(QChar(0xf1b8));

	_btns = new QButtonGroup(this);
	_btns->addButton(recycleButton, ITrac::Recycle);
	_btns->addButton(washButton, ITrac::Wash);
	_btns->addButton(washCheckButton, ITrac::WashCheck);
	_btns->addButton(sterileButton, ITrac::Sterile);
	_btns->addButton(sterileCheckButton, ITrac::SterileCheck);
	_btns->addButton(dispatchButton, ITrac::Dispatch);

	connect(_btns, SIGNAL(buttonClicked(QAbstractButton *)),
		this, SLOT(updateButtonGroupState(QAbstractButton *)));
	
	//bindScanner();
	//QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

DailyHome::~DailyHome()
{
}

void DailyHome::updateButtonGroupState(QAbstractButton *btn)
{
	//throw std::logic_error("The method or operation is not implemented.");
	int state = _btns->id(btn);
	emit flowBtnClicked(state);
}

void DailyHome::handleBarcode(const QString &bc)
{
	Barcode barcode(bc);
	if (Barcode::Action != barcode.type())
		return;

	switch (barcode.intValue())
	{
	case 910001:
		recycleButton->click();
		break;
	case 910002:
		washButton->click();
		break;
	case 910003:
		washCheckButton->click();
		break;
	case 910004:
		sterileButton->click();
		break;
	case 910005:
		sterileCheckButton->click();
		break;
	case 910006:
		dispatchButton->click();
		break;
	default:
		break;
	}
}
