#include "loginpage.h"
#include "ui/image.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include "ui/labels.h"
#include "ui/container.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"

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

#include <printer/labelprinter.h>
#include "util/printermanager.h"

void testPrinter()
{
	LabelPrinter *printer = PrinterManager::currentPrinter();
	if (0 != printer->open(LABEL_PRINTER)) {
		return;
	}
	printer->setInversion(true);
	
	PackageLabel label;
	label.packageId = "100000861904230005";
	label.packageName = "胸腔器械（每份84把共1份）";
	label.packageFrom = "手术室";
	label.packageType = "棉布";
	label.disinDate = "2019-04-23";
	label.expiryDate = "2019-04-30";
	label.operatorName = "张爱琴";
	label.assessorName = "李建";
	label.count = 54;

	printer->printPackageLabel(label);
/*
	ClinicLabel label;
	label.packageId = "100000861904230001";
	label.packageName = "清创器械";
	label.packageFrom = "外科";
	label.disinDate = "2019-04-23";
	label.expiryDate = "2019-04-30";
	label.operatorName = "张爱琴";
	label.assessorName = "李建";

	printer->printClinicLabel(label);
	
	SterilizeLabel label;
	label.sterilizeId = "100000861904230002";
	label.sterilizeName = "灭菌器001号";
	label.sterilizeDate = "2019-04-23";
	label.sterilizeTime = "18:47:02";
	label.panNum = 1;
	label.packageNum = 3;
	printer->printSterilizedLabel(label);
	*/
	printer->close();
}

namespace Widget {

	LoginPanel::LoginPanel(Ui::Container *container /*= nullptr*/)
		: Inner(container)
		, userEdit(new Ui::FlatEdit("工号", this))
		, pwdEdit(new Ui::FlatEdit("密码", this))
		, error(new Ui::ErrorLabel("用户名或密码不正确", this))
		, _waiter(new WaitingSpinner(this))
	{
		QVBoxLayout *layout = new QVBoxLayout(this);

		Ui::Title *titleLabel = new Ui::Title("欢迎使用 Winstrac 桌面");
		//Ui::Description *descLabel = new Ui::Description("请输入工号和密码登录系统，或者直接扫描您的工牌");
		Ui::Description *descLabel = new Ui::Description("我们致力于更便捷的数字化消毒供应室信息追溯管理");
		descLabel->setMinimumWidth(300);
		layout->addWidget(titleLabel);
		layout->addWidget(descLabel);

		Ui::addSpacer(layout, Qt::Vertical);

		userEdit->setInputValidator(Ui::InputValitor::NumberOnly);
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

		version();
	}

	void LoginPanel::submit() {
		//LabelPrinter *printer = PrinterManager::currentPrinter();
		//printer->open(COMMON_PRINTER);
		//Issues issues;
		//issues.deptName = "手术室";
		//issues.date = "2019/08/08";

		//package p;
		//p.name = "成人体循器械（每份112把）";
		//p.count = 10;
		//QList<package> list;
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//list.append(p);
		//issues.packages = list;

		//issues.operName = "周玲玲";
		//issues.orderId = "17000001";

		//printer->printIssue(issues, PaperType::recycle);
		//return;
		
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
		_waiter->start();

		post(url(PATH_VERSION), "{}", [=](QNetworkReply *reply) {
			_waiter->stop();
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
					if (_version.compare(Constant::Version)) {
						QProcess *pro = new QProcess(this);
						QStringList args(PATH_BASE);
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
		_waiter->start();

		QVariantMap vmap;
		
		if (!pwd.isEmpty()) {
			vmap.insert("account", account);
			vmap.insert("pwd", pwd);
		}
		else {
			vmap.insert("operator_id", account);
		}

		vmap.insert("sign_flag", "C");


		/*Core::User &user = Core::currentUser();
		user.role = Core::User::Admin;
		user.deptId = 12000021;
		user.id = 11000008;
		_waiter->stop();
		container()->accept();
		return;*/

		post(url(PATH_USER_LOGIN), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
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
