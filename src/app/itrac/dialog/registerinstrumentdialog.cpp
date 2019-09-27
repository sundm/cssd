#include "registerinstrumentdialog.h"
#include "core/application.h"
#include "core/net/url.h"
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
	, _lostNumber(new QSpinBox())
	, _brokenInfo(new QTextEdit)
{
	setWindowTitle("器械异常登记");

	_nameEdit->setReadOnly(true);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &RegisterInstrumentDialog::accept);

	_lostNumber->setEnabled(false);
	_brokenInfo->setEnabled(false);

	connect(_checkLostBox, SIGNAL(stateChanged(int)), this, SLOT(checklostChanged(int)));
	connect(_checkBrokenBox, SIGNAL(stateChanged(int)), this, SLOT(ckeckBrokenChanged(int)));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("器械名称"), 0, 0);
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_checkLostBox, 1, 0);
	mainLayout->addWidget(_lostNumber, 1, 1);
	mainLayout->addWidget(_checkBrokenBox, 2, 0);
	mainLayout->addWidget(_brokenInfo, 2, 1);

	mainLayout->addWidget(submitButton, 3, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 2 : 480, sizeHint().height());
}

void RegisterInstrumentDialog::checklostChanged(int state)
{
	state == Qt::Checked ? _lostNumber->setEnabled(true) : _lostNumber->setEnabled(false);
}

void RegisterInstrumentDialog::ckeckBrokenChanged(int state)
{
	state == Qt::Checked ? _brokenInfo->setEnabled(true) : _brokenInfo->setEnabled(false);
}

void RegisterInstrumentDialog::accept() {
	QString name = _nameEdit->text();

	QVariantMap vmap;
	vmap.insert("instrument_name", name);
	post(url(PATH_INSTRUMENT_MODIFY), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("修改器械失败: ").append(resp.errorString()));
		}
		else {
			return QDialog::accept();
		}
	});
}