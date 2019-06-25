#include "operatorchooser.h"
#include "barcode.h"
#include "core/user.h"
#include "ui/ui_commons.h"
#include "ui/buttons.h"
#include <qscanner/qscanner.h>
#include <QtWidgets/QtWidgets>

OperatorChooser::OperatorChooser(Scanable* candidate, QWidget *parent)
	:QDialog(parent)
{
	if (candidate) {
		setCandidate(candidate);
	}

	setWindowTitle("操作员确认");
	QVBoxLayout *layout = new QVBoxLayout(this);

	const QString tip =
		"如使用当前登录用户，直接扫描下方条码，或点击\"使用当前用户\"\n"
		"如需使用其他用户，请扫描其他用户的工牌";
	QLabel *tipLabel = new QLabel(tip);
	tipLabel->setWordWrap(true);
	layout->addWidget(tipLabel);

	QLabel *qr = new QLabel;
	qr->setPixmap(QPixmap(":/res/910108.png"));
	layout->addWidget(qr);

	layout->addWidget(Ui::createSeperator(Qt::Horizontal));

	Ui::PrimaryButton *button = new Ui::PrimaryButton("使用当前用户");
	connect(button, &QPushButton::clicked, this, &OperatorChooser::applyCurrent);
	layout->addWidget(button);
}

int OperatorChooser::get(Scanable* candidate, QWidget *parent) {
	OperatorChooser chooser(candidate, parent);
	return (QDialog::Accepted == chooser.exec()) ? chooser._opId : 0;
}

void OperatorChooser::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::User) {
		_opId = code.toInt();
		QDialog::accept();
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		applyCurrent();
	}
}

void OperatorChooser::applyCurrent() {
	Core::User &user = Core::currentUser();
	_opId = user.id;
	QDialog::accept();
}
