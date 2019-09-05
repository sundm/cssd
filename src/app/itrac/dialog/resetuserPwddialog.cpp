#include "resetuserPwddialog.h"
#include "core/user.h"
#include "core/application.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

ResetUserPWDialog::ResetUserPWDialog(const Core::User &user, QWidget *parent)
	: QDialog(parent)
	, _waiter(new WaitingSpinner(this))
{	
	_userId = user.id;
	QLabel *userLabel = new QLabel;
	userLabel->setPixmap(QPixmap(":/res/user_female_96.png"));
	userLabel->setAlignment(Qt::AlignCenter);

	Ui::Title *title = new Ui::Title(QString("%1 (%2)").arg(user.name, user.deptName));

	QString descrip;
	if (user.role == 1 || user.role == 2)
		descrip = QString("管理");
	else
		descrip = QString("普通");

	Ui::Description *account = new Ui::Description(
		QString("工号 %1, %2权限").arg(user.id).arg(descrip));
	Ui::Description *loginTime = new Ui::Description("登录于 " + user.loginTime);

	QPushButton *submitPwdButton = new QPushButton("重置密码");
	submitPwdButton->setIcon(QIcon(":/res/write-24.png"));
	submitPwdButton->setDefault(true);
	connect(submitPwdButton, &QPushButton::clicked, this, &ResetUserPWDialog::resetPwd);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(20);
	mainLayout->addWidget(userLabel);
	mainLayout->addWidget(title);
	mainLayout->addWidget(account);
	mainLayout->addWidget(loginTime);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal));
	mainLayout->addWidget(submitPwdButton, 0, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(parent ? parent->width() / 2 : 360, height());
}

void ResetUserPWDialog::resetPwd() {
	QMessageBox *messageBox = new QMessageBox(this);
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setWindowTitle("提示");
	messageBox->setText("是否重置当前用户密码？");
	messageBox->addButton("取消", QMessageBox::RejectRole);
	messageBox->addButton("确定", QMessageBox::AcceptRole);
	if (messageBox->exec() == QDialog::Accepted) {
		QVariantMap vmap;
		vmap.insert("operator_id", _userId);
		_waiter->start();
		post(url(PATH_USER_RESET_PWD), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("重置失败: ").append(resp.errorString()));
			}
			else {
				XNotifier::warn(QString("重置成功!"));
			}
		});
	}
}

