#include "loginpage.h"
#include "ui/image.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include "ui/labels.h"
#include "ui/container.h"
#include "ui/ui_commons.h"

#include "core/net/url.h"
#include "core/user.h"
#include "core/constants.h"
#include "core/application.h"
#include "core/barcode.h"

#include <QVBoxLayout>
#include <QAction>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QProcess>

namespace Widget {

	LoginPanel::LoginPanel(Ui::Container *container /*= nullptr*/)
		: Inner(container)
		, userEdit(new Ui::FlatEdit(this, "工号"))
		, pwdEdit(new Ui::FlatEdit(this, "密码"))
		, error(new Ui::ErrorLabel("用户名或密码不正确")) {

		QVBoxLayout *layout = new QVBoxLayout(this);

		Ui::Title *titleLabel = new Ui::Title("欢迎使用 Winstrac 桌面");
		//Ui::Description *descLabel = new Ui::Description("请输入工号和密码登录系统，或者直接扫描您的工牌");
		Ui::Description *descLabel = new Ui::Description("我们致力于更便捷的数字化消毒供应室信息追溯管理");
		layout->addWidget(titleLabel);
		layout->addWidget(descLabel);

		Ui::addSpacer(layout, Qt::Vertical);

		userEdit->setInputValidator(Ui::InputValitor::LetterAndNumber);
		pwdEdit->setEchoMode(QLineEdit::Password);
		userEdit->setMinimumHeight(Constant::fixedLineEditHeight);
		pwdEdit->setMinimumHeight(Constant::fixedLineEditHeight);
		layout->addWidget(userEdit);
		layout->addWidget(pwdEdit);

		error->hide();
		layout->addWidget(error);

		Ui::addSpacer(layout, Qt::Vertical);

		Ui::PrimaryButton *button = new Ui::PrimaryButton("登  录", Ui::BtnSize::Large);
		connect(button, &QAbstractButton::clicked, this, &Inner::submit);
		layout->addWidget(button);

		layout->setSpacing(15);

		//Ui::addPrimaryShortcut(this, Qt::Key_Return, SLOT(submit()));
		//Ui::addPrimaryShortcut(this, Qt::Key_Enter, SLOT(submit()));
		connect(userEdit, SIGNAL(returnPressed()), this, SLOT(submit()));
		connect(pwdEdit, SIGNAL(returnPressed()), this, SLOT(submit()));

		//version();
	}

	void LoginPanel::submit() {
		QString user(userEdit->text());
		QString pwd(pwdEdit->text());
		if (user.isEmpty()) {
			userEdit->setFocus();
			return;
		}
		if (pwd.isEmpty()) {
			pwdEdit->setFocus();
			return;
		}
		login(user, pwd);
	}

	void LoginPanel::handleBarcode(const QString &code) {
		Barcode bc(code);
		if (bc.type() == Barcode::User) {
			login(code, "");
		}
	}

	void LoginPanel::version()
	{
		Core::app()->startWaitingOn(this);

		Url::post(Url::PATH_VERSION, "{}", [=](QNetworkReply *reply) {
			Core::app()->stopWaiting();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				error->shake(QString("获取版本号错误：%1").arg(resp.errorString()));
				updateSize();
			}
			else
			{
				const QString code = resp.getAsString("code");
				if (!code.compare("9000")) {
					const QString _version = resp.getAsString("version");
					if (_version.compare(Url::Version)) {
						QProcess *pro = new QProcess(this);
						QStringList args(Url::PATH_BASE);
#ifdef _DEBUG
						pro->startDetached("updated.exe", args);
#else
						pro->startDetached("update.exe", args);
#endif // DEBUG
						qApp->quit();
						
					}
				}
				else {
					error->shake(QString("获取版本号错误：%1").arg(resp.getAsString("msg")));
					updateSize();
				}
			}
		});
	}

	void LoginPanel::login(const QString &account, const QString &pwd) {
		Core::app()->startWaitingOn(this);

		QVariantMap vmap;

		if (!pwd.isEmpty()) {
			vmap.insert("account", account);
			vmap.insert("pwd", pwd);
		}
		else {
			vmap.insert("operator_id", account);
		}

		vmap.insert("sign_flag", "O");

		/*Core::User &user = Core::currentUser();
		user.role = Core::User::Admin;
		user.deptId = 12000021;
		user.id = 11000008;
		Core::app()->stopWaiting();
		container()->accept();
		return;*/

		Url::post(Url::PATH_USER_LOGIN, vmap, [this](QNetworkReply *reply) {
			Core::app()->stopWaiting();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				error->shake(resp.errorString());
				updateSize();
			}
			else {
				Core::User &user = Core::currentUser();
				user.id = resp.getAsInt("operator_id");
				user.deptId = resp.getAsInt("dept_id");
				user.role = resp.getAsInt("role_id");
				user.gender = resp.getAsString("gender") == "M" ? Core::User::Male : Core::User::Female;
				user.name = resp.getAsString("name");
				user.deptName= resp.getAsString("dept_name");
				user.loginTime = resp.getAsString("last_login");
				container()->accept();
			}
		});
	}

	LoginPage::LoginPage(QWidget *parent/* = nullptr*/) : Ui::Source(parent)
		, _headPic(new Ui::Picture(":/res/login-bg.jpg"))
		, _container(new Ui::Container(nullptr)) {
		_headPic->setFixedHeight(260);
		_headPic->setBgFit(Ui::BgFit::Fill);
		//_headPic->setStyleSheet("background:red");

		_container->setInner(new LoginPanel(_container));
		connect(_container, &Ui::Container::accepted, this, &LoginPage::permitted);

		QVBoxLayout *_hLayout = new QVBoxLayout(this);
		_hLayout->setContentsMargins(0, 0, 0, 0);
		_hLayout->addWidget(_headPic);
		_hLayout->addWidget(_container);
	}


	//void LoginPage::hideAnimated() {
	//	const int duration = 300;

	//	//QWidget *target = _container->inner();
	//	QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
	//	this->setGraphicsEffect(eff);

	//	QPropertyAnimation *anim1 = new QPropertyAnimation(eff, "opacity");
	//	anim1->setDuration(duration);
	//	anim1->setStartValue(qreal(1));
	//	anim1->setEndValue(qreal(0));
	//	anim1->start(QPropertyAnimation::DeleteWhenStopped);

	//	/*QPropertyAnimation *anim2 = new QPropertyAnimation(target, "pos");
	//	anim2->setDuration(duration);
	//	QPoint pos = target->pos();
	//	anim2->setStartValue(pos);
	//	anim2->setEasingCurve(QEasingCurve::InBack);
	//	anim2->setEndValue(QPoint(pos.x(), pos.y() + target->height()));

	//	QPropertyAnimation *anim3 = new QPropertyAnimation(_headPic, "pos");
	//	anim3->setDuration(duration);
	//	pos = _headPic->pos();
	//	anim3->setStartValue(pos);
	//	anim3->setEasingCurve(QEasingCurve::InBack);
	//	anim3->setEndValue(QPoint(pos.x(), pos.y() - _headPic->height()));

	//	QParallelAnimationGroup *group = new QParallelAnimationGroup();
	//	group->addAnimation(anim1);
	//	group->addAnimation(anim2);
	//	group->addAnimation(anim3);
	//	group->start(QAbstractAnimation::DeleteWhenStopped);*/

	//	connect(anim1, SIGNAL(finished()), this, SIGNAL(hidden()));
	//}
}
