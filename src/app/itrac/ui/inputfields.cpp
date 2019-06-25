#include "inputfields.h"

#include <QRegExpValidator>

namespace Ui {

	FlatEdit::FlatEdit(const QString &placeHolder/* = QString()*/, QWidget *parent/* = nullptr*/)
		: QLineEdit(parent) {
		if (!placeHolder.isEmpty()) {
			setPlaceholderText(placeHolder);
		}
	}

	FlatEdit::~FlatEdit() {
	}

	void FlatEdit::setInputValidator(InputValitor val) {
		setValidator(nullptr); // remove the old validator
		if (Ui::InputValitor::NoLimit == val) {
			return;
		}

		auto regValidator = new QRegExpValidator(this);
		switch (val) {
		case Ui::InputValitor::LetterOnly:
			regValidator->setRegExp(QRegExp("[a-zA-Z]+"));
			break;
		case Ui::InputValitor::NumberOnly:
			regValidator->setRegExp(QRegExp("\\d+"));
			break;
		case Ui::InputValitor::LetterAndNumber:
			regValidator->setRegExp(QRegExp("[a-zA-Z0-9]+"));
			break;
		case Ui::InputValitor::Email:
			break;
		}
		setValidator(regValidator);
	}

	QSize FlatEdit::minimumSizeHint() const {
		QSize sz = QLineEdit::minimumSizeHint();
		sz.setHeight(32);
		return sz;
	}

} // namespace Ui
