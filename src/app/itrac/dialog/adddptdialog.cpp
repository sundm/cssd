#include "adddptdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

AddDptDialog::AddDptDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _pinyinEdit(new Ui::FlatEdit)
	, _phoneEdit(new Ui::FlatEdit)
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新科室");
	_isModify = false;

	_pinyinEdit->setInputValidator(Ui::InputValitor::LetterOnly);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddDptDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("新科室名"), 0, 0);
	mainLayout->addWidget(new QLabel("拼音码"), 1, 0);
	mainLayout->addWidget(new QLabel("联系方式"), 2, 0);
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_pinyinEdit, 1, 1);
	mainLayout->addWidget(_phoneEdit, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 3 : 360, height());
}

void AddDptDialog::setDtpInfo(const QString& id, const QString& name, const QString& pinyin, const QString& phone) {
	setWindowTitle("修改新科室");
	_isModify = true;
	_dptId = id.toInt();

	_nameEdit->setText(name);
	_nameEdit->setReadOnly(_isModify);

	_pinyinEdit->setText(pinyin);
	_phoneEdit->setText(phone);
}

void AddDptDialog::accept() {
	QString name = _nameEdit->text();
	QString pinyin = _pinyinEdit->text().toUpper();
	QString phone = _phoneEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (pinyin.isEmpty()) {
		_pinyinEdit->setFocus();
		return;
	}

	QVariantMap vmap;
	vmap.insert("department_name", name);
	vmap.insert("pinyin_code", pinyin);
	if (!phone.isEmpty())
	{
		vmap.insert("phone", phone);
	}
	
	if (!_isModify) {
		_waiter->start();
		post(url(PATH_DEPT_ADD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加科室失败: ").append(resp.errorString()));
			}
			else {
				QDialog::accept();
			}
		});
	}
	else
	{
		vmap.insert("department_id", _dptId);
		_waiter->start();
		post(url(PATH_DEPT_MODIFY), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("修改科室失败: ").append(resp.errorString()));
			}
			else {
				QDialog::accept();
			}
		});
	}
	
}

