#include "programdialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/inliner.h"
#include "core/net/url.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

AddProgramDialog::AddProgramDialog(QWidget *parent)
	: QDialog(parent)
	, _typeCombo(new QComboBox)
	, _nameEdit(new Ui::FlatEdit)
	, _descEdit(new Ui::FlatEdit)
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新的设备程序");

	_typeCombo->addItem("清洗设备程序", WASH_DEVICE);
	_typeCombo->addItem("灭菌设备程序", STERILE_DEVICE);
	_typeCombo->setCurrentIndex(-1);

	_descEdit->setPlaceholderText("选填");

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddProgramDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("程序类型"), 0, 0);
	mainLayout->addWidget(new QLabel("程序名称"), 1, 0);
	mainLayout->addWidget(new QLabel("程序描述"), 2, 0);
	mainLayout->addWidget(_typeCombo, 0, 1);
	mainLayout->addWidget(_nameEdit, 1, 1);
	mainLayout->addWidget(_descEdit, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void AddProgramDialog::accept() {
	if (_typeCombo->currentIndex() == -1) {
		_typeCombo->showPopup();
		return;
	}

	QString name = _nameEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}

	QVariantMap vmap;
	vmap.insert("program_name", name);
	vmap.insert("program_desc", _descEdit->text());
	vmap.insert("program_type", _typeCombo->currentData());

	_waiter->start();
	post(url(PATH_PROGRAM_ADD), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//return; // TODO
		}
		else {
			QDialog::accept();
		}
	});
}


