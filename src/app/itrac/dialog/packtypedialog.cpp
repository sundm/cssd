#include "packtypedialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/inliner.h"
#include "core/net/url.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

AddPackTypeDialog::AddPackTypeDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _validBox(new QSpinBox)
	, _maxBox(new QSpinBox)
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新的打包类型");
	_isModify = false;

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddPackTypeDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("打包类型"), 0, 0);
	mainLayout->addWidget(new QLabel("当前有效期"), 1, 0);
	mainLayout->addWidget(new QLabel("最大有效期"), 2, 0);
	mainLayout->addWidget(_nameEdit, 0, 1);

	_validBox->setMinimum(0);
	_validBox->setMaximum(500);
	_maxBox->setMinimum(0);
	_maxBox->setMaximum(500);

	mainLayout->addWidget(_validBox, 1, 1);
	mainLayout->addWidget(_maxBox, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void AddPackTypeDialog::setInfo(const QString &name, const int &id, const int &valid, const int &max)
{
	_isModify = true;
	setWindowTitle("修改打包类型参数");
	_nameEdit->setText(name);
	_id = id;
	_nameEdit->setReadOnly(_isModify);
	_validBox->setValue(valid);
	_maxBox->setValue(max);
}

void AddPackTypeDialog::accept() {

	QString name = _nameEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}

	int valid = _validBox->value();
	if (valid <= 0) {
		_validBox->setFocus();
		return;
	}

	int max = _maxBox->value();
	if (max <= 0  || max < valid) {
		_maxBox->setFocus();
		return;
	}

	QVariantMap vmap;
	
	vmap.insert("valid_date", valid);
	vmap.insert("max_date", max);

	_waiter->start();
	if (_isModify){
		vmap.insert("pack_type_id", _id);

		post(url(PATH_PACKTYPE_MODIFY), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("修改打包类型参数失败:").append(resp.errorString()));
				return;
			}
			else {
				QDialog::accept();
			}
		});
	}
	else {
		vmap.insert("pack_type_name", name);

		post(url(PATH_PACKTYPE_ADD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加新的打包类型失败:").append(resp.errorString()));
				return;
			}
			else {
				QDialog::accept();
			}
		});
	}
	
}


