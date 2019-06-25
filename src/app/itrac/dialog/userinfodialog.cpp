#include "userinfodialog.h"
#include "core/user.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include <QtWidgets/QtWidgets>

UserInfoDialog::UserInfoDialog(QWidget *parent)
	: QDialog(parent)
	, _oldPwdEdit(new Ui::FlatEdit)
	, _newPwdEdit(new Ui::FlatEdit)
	, _newPwdConfirmEdit(new Ui::FlatEdit)
{
	Core::User &user = Core::currentUser();
	
	QLabel *userLabel = new QLabel;
	userLabel->setPixmap(QPixmap(":/res/user_female_96.png"));
	userLabel->setAlignment(Qt::AlignCenter);

	Ui::Title *title = new Ui::Title(QString("%1 (%2)").arg(user.name, user.deptName));
	Ui::Description *account = new Ui::Description(
		QString("工号 %1, %2权限").arg(user.id).arg(1 == user.role ? "普通":"管理"));
	Ui::Description *loginTime = new Ui::Description("登录于 " + user.loginTime);

	QWidget *pwdGroup = new QWidget;
	_oldPwdEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
	_oldPwdEdit->setEchoMode(QLineEdit::Password);
	_newPwdEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
	_newPwdEdit->setEchoMode(QLineEdit::Password);
	_newPwdConfirmEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
	_newPwdConfirmEdit->setEchoMode(QLineEdit::Password);
	QPushButton *submitPwdButton = new QPushButton("提交更改");
	submitPwdButton->setIcon(QIcon(":/res/write-24.png"));
	submitPwdButton->setDefault(true);
	connect(submitPwdButton, &QPushButton::clicked, this, &UserInfoDialog::changePwd);

	QGridLayout *gridLayout = new QGridLayout(pwdGroup);
	gridLayout->setVerticalSpacing(15);
	gridLayout->addWidget(new QLabel("原密码"), 0, 0);
	gridLayout->addWidget(new QLabel("新密码"), 1, 0);
	gridLayout->addWidget(new QLabel("新密码确认"), 2, 0);
	gridLayout->addWidget(_oldPwdEdit, 0, 1);
	gridLayout->addWidget(_newPwdEdit, 1, 1);
	gridLayout->addWidget(_newPwdConfirmEdit, 2, 1);
	gridLayout->addWidget(submitPwdButton, 3, 0, 1, 2);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(20);
	mainLayout->addWidget(userLabel);
	mainLayout->addWidget(title);
	mainLayout->addWidget(account);
	mainLayout->addWidget(loginTime);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal));
	mainLayout->addWidget(pwdGroup, 0, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 2 : 360, height());
}

void UserInfoDialog::changePwd() {
	QString oldPwd = _oldPwdEdit->text();
	QString newPwd = _newPwdEdit->text();
	QString newPwd2 = _newPwdConfirmEdit->text();
	if (oldPwd.isEmpty()) {
		_oldPwdEdit->setFocus();
		return;
	}
	if (newPwd.isEmpty()) {
		_newPwdEdit->setFocus();
		return;
	}
	if (newPwd2.isEmpty() || newPwd != newPwd2) {
		_newPwdConfirmEdit->setFocus();
		return;
	}

	QVariantMap vmap;
	vmap.insert("operator_id", Core::currentUser().id);
	vmap.insert("old_pwd", oldPwd);
	vmap.insert("pwd", newPwd);

	Core::app()->startWaitingOn(this);
	Url::post(Url::PATH_USER_MODIFY_PWD, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//return; // TODO
		}
		else {
			_oldPwdEdit->clear();
			_newPwdEdit->clear();
			_newPwdConfirmEdit->clear();
		}
	});
}

