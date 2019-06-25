#include "groups.h"
#include <QFormLayout>

FormGroup::FormGroup(QWidget *parent)
	: QGroupBox(parent)
	, _layout(new QFormLayout(this))
{
}

void FormGroup::addRow(const QString &lable, QWidget *w) {
	_layout->addRow(lable, w);
}

