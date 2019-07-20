#include "tips.h"

#include <QtWidgets/QtWidgets>

Tip::Tip(QWidget *parent) : QGroupBox(parent) {
	setupUi();
}

Tip::Tip(const QString &text, QWidget *parent) : QGroupBox(parent) {
	setupUi(text);
}

Tip::~Tip() {
}

void Tip::addButton(QAbstractButton *button) {
	_layout->insertWidget(_layout->count() - 1, button);
}

void Tip::addQr() {
	QLabel *qr = new QLabel;
	qr->setPixmap(QPixmap(":/res/910108.png"));
	_layout->insertWidget(_layout->count() - 1, qr);
}

void Tip::setupUi(const QString &text) {
	_layout = new QVBoxLayout(this);

	QLabel *iconLabel = new QLabel;
	iconLabel->setPixmap(QPixmap(":/res/tip.png"));

	QLabel *tipLabel = new QLabel(text);
	tipLabel->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignTop);
	tipLabel->setWordWrap(true);



	_layout->addWidget(iconLabel);
	_layout->addWidget(tipLabel);
	
	_layout->addStretch();
	_layout->addStretch();
}
