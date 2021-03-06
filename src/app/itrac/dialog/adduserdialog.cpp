#include "adduserdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include "widget/controls/idedit.h"
#include "widget/controls/combos.h"
#include <QtWidgets/QtWidgets>

AddUserDialog::AddUserDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _phoneEdit(new Ui::FlatEdit)
	, _deptEdit(new DeptEdit)
	, _genderCombo(new GenderComboBox)
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新用户");
	_phoneEdit->setInputValidator(Ui::InputValitor::NumberOnly);

	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow("姓名", _nameEdit);
	formLayout->addRow("电话", _phoneEdit);
	formLayout->addRow("科室", _deptEdit);
	formLayout->addRow("性别", _genderCombo);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddUserDialog::accept);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(15);
	mainLayout->addLayout(formLayout);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal));
	mainLayout->addWidget(submitButton, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 3 : 360, height());

	_deptEdit->load(DeptEdit::ALL);

	_isModify = false;
}

void AddUserDialog::setUserInfo(const Core::User &user)
{
	setWindowTitle("修改当前用户");
	_userId = user.id;

	_nameEdit->setText(user.name);
	_nameEdit->setReadOnly(true);

	_phoneEdit->setText(user.phone);
	_deptEdit->setCurrentIdPicked(user.deptId, user.deptName);
	_genderCombo->setCurrentIndex(user.gender);

	_isModify = true;
}

void AddUserDialog::accept() {
	QString name = _nameEdit->text();
	QString phone = _phoneEdit->text().toUpper();
	int deptId = _deptEdit->currentId();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (phone.isEmpty()) {
		_phoneEdit->setFocus();
		return;
	}
	if (0 == deptId) {
		_deptEdit->setFocus();
		return;
	}
	if (_genderCombo->currentIndex() == -1) {
		_genderCombo->showPopup();
		return;
	}

	QVariantMap vmap;
	vmap.insert("name", name);
	vmap.insert("phone", phone);
	vmap.insert("gender", _genderCombo->currentData());
	vmap.insert("dept_id", deptId);

	if (!_isModify){
		_waiter->start();
		post(url(PATH_USER_ADD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加用户失败: ").append(resp.errorString()));
				return;
			}
			else {
				QDialog::accept();
			}
		});
	}
	else {
		vmap.insert("operator_id", _userId);

		_waiter->start();
		post(url(PATH_USER_MODIFY), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("修改用户失败: ").append(resp.errorString()));
				return;
			}
			else {
				QDialog::accept();
			}
		});
	}
}

