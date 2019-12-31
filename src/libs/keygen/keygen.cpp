#include "keygen.h"
#include "../libs/des/des3.h"

keygen::keygen(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
}


void keygen::onGenClicked()
{
	QString src = ui.keyEdit->text();
	src = src.remove("-");
	if (src.isEmpty() || src.length() < 16)
	{
		ui.codeEdit->clear();
		ui.keyEdit->setFocus();
		return;
	}

	DES3 des(0x2BD6459F82C5B300, 0x952C49104881FF48, 0x2BD6459F82C5B300);
	
	bool isOK = false;
	ui64 input = src.toULongLong(&isOK, 16);
	if (isOK)
	{
		ui64 result = des.encrypt(input);

		QString res = QString::number(result, 16);

		ui.codeEdit->setText(res);
	}
	else
	{
		ui.codeEdit->clear();
		return;
	}

}