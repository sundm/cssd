#include "regexpinputdialog.h"
#include "ui/inputfields.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QRegExpValidator>

RegExpInputDialog::RegExpInputDialog(QWidget *parent, Qt::WindowFlags flags) :
	QDialog(parent)
{
	if (flags != 0) setWindowFlags(flags);

	QVBoxLayout *l = new QVBoxLayout(this);

	label = new QLabel;

	regExp = QRegExp("*");
	regExp.setPatternSyntax(QRegExp::Wildcard);
	validator = new QRegExpValidator(regExp);

	text = new Ui::FlatEdit(QString(), this);
	text->setValidator(validator);
	//connect(text, SIGNAL(textChanged(QString)), this, SLOT(checkValid(QString)));

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	l->addWidget(label);
	l->addWidget(text);
	l->addWidget(buttonBox);

	//setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed); // this doesn't work
	//l->setSizeConstraint(QLayout::SetFixedSize); // got fixed size based on its contents
	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 3 : 360, height());
}

void RegExpInputDialog::setTitle(const QString &title) { setWindowTitle(title); }
void RegExpInputDialog::setLabelText(const QString &label) { this->label->setText(label); }
void RegExpInputDialog::setText(const QString &text) { this->text->setText(text); }
QString RegExpInputDialog::getText() const { return text->text(); }

void RegExpInputDialog::setRegExp(const QRegExp &regExp) {
	validator->setRegExp(regExp);
	//checkValid(text->text());
}

//void RegExpInputDialog::checkValid(const QString &text) {
//	QString _text = QString(text);
//	int pos = 0;
//	bool valid = validator->validate(_text, pos) == QRegExpValidator::Acceptable;
//	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(valid);
//}

QString RegExpInputDialog::getText(QWidget *parent, const QString &title, const QString &label, const QString &text, const QRegExp &regExp, bool *ok, Qt::WindowFlags flags) {
	RegExpInputDialog *r = new RegExpInputDialog(parent, flags);
	r->setTitle(title);
	r->setLabelText(label);
	r->setText(text);
	r->setRegExp(regExp);
	*ok = r->exec() == QDialog::Accepted;
	if (*ok) return r->getText();
	else return QString();
}