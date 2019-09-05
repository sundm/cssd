#include "addinstrumentdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

AddInstrumentDialog::AddInstrumentDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _pinyinEdit(new Ui::FlatEdit)
	, _checkBox(new QCheckBox("贵重器械"))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新器械");

	_isModify = false;

	_pinyinEdit->setInputValidator(Ui::InputValitor::LetterOnly);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddInstrumentDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("新器械名称"), 0, 0);
	mainLayout->addWidget(new QLabel("拼音码"), 1, 0);
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_pinyinEdit, 1, 1);
	mainLayout->addWidget(_checkBox, 2, 0, 1, 2);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 3 : 360, height());
}

void AddInstrumentDialog::setInfo(const QString &id, const QString &name, const QString &pinyin, const bool isVIP)
{
	setWindowTitle("修改新器械");
	_isModify = true;
	_instrumentId = id;

	_nameEdit->setText(name);
	_pinyinEdit->setText(pinyin);
	_nameEdit->setReadOnly(_isModify);
	//_pinyinEdit->setReadOnly(_isModify);

	_checkBox->setChecked(isVIP);
}

void AddInstrumentDialog::accept() {
	QString name = _nameEdit->text();
	QString pinyin = _pinyinEdit->text().toUpper();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (pinyin.isEmpty()) {
		_pinyinEdit->setFocus();
		return;
	}

	QVariantMap vmap;
	vmap.insert("instrument_name", name);
	vmap.insert("instrument_type", "1");
	vmap.insert("is_vip_instrument", Qt::Checked == _checkBox->checkState() ? "1" : "0");
	vmap.insert("pinyin_code", pinyin);

	_waiter->start();
	if (_isModify)
	{
		vmap.insert("instrument_id", _instrumentId);
		post(url(PATH_INSTRUMENT_MODIFY), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("修改器械失败: ").append(resp.errorString()));
			}
			else {
				QDialog::accept();
			}
		});
	}
	else
	{
		post(url(PATH_INSTRUMENT_ADD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加器械失败: ").append(resp.errorString()));
			}
			else {
				QDialog::accept();
			}
		});
	}
	
}

