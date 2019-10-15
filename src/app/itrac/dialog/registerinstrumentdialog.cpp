#include "registerinstrumentdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "core/user.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <xui/images.h>
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

RegisterInstrumentDialog::RegisterInstrumentDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _checkLostBox(new QCheckBox("器械丢失"))
	, _checkBrokenBox(new QCheckBox("器械损坏"))
{
	setWindowTitle("器械异常登记");
	
	_nameEdit->setReadOnly(true);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &RegisterInstrumentDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("器械名称"), 0, 0);
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_checkLostBox, 1, 0);
	mainLayout->addWidget(_checkBrokenBox, 1, 1);

	mainLayout->addWidget(submitButton, 2, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 2 : 480, sizeHint().height());
}

void RegisterInstrumentDialog::setInfo(const QString&card_id, const QString&pkg_id, const QString&ins_name, const QString&ins_id, int state)
{
	_pkg_id = pkg_id;
	_card_id = card_id;
	_ins_id = ins_id;
	_state = state;

	_nameEdit->setText(ins_name);

	_state_f = 0;
	_state_e = 0;
	switch (state)
	{
	case 1:
		_checkBrokenBox->setChecked(true);
		_state_e = 1;
		break;
	case 2:
		_checkLostBox->setChecked(true);
		_state_f = 1;
		break;
	case 3:
		_checkLostBox->setChecked(true);
		_checkBrokenBox->setChecked(true);
		_state_e = 1;
		_state_f = 1;
		break;
	default:
		break;
	}
}

void RegisterInstrumentDialog::accept() {
	int state = 0;
	int state_f = 0;
	int state_e = 0;
	if (_checkBrokenBox->isChecked()) {
		state = state + 1;
		state_e = 1;
	}
		
	if (_checkLostBox->isChecked()) {
		state = state + 2;
		state_f = 1;
	}

	if (_state == state) return QDialog::accept();
	else _state = state;

	QVariantMap vmap;
	vmap.insert("card_id", _card_id);
	if (_state_f != state_f)
		vmap.insert("state_f", state_f);
	if (_state_e != state_e)
		vmap.insert("state_e", state_e);
	vmap.insert("pkg_id", _pkg_id);
	vmap.insert("operator_id", Core::currentUser().id);
	vmap.insert("ins_id", _ins_id);

	post(url(PATH_CARD_MODIFY), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("登记失败: ").append(resp.errorString()));
		}
		else {
			int card_record = resp.getAsInt("card_record");
			emit sendData(card_record, _state);
			return QDialog::accept();
		}
	});
}