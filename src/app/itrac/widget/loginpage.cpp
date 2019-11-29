#include "loginpage.h"
#include "ui/image.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include "ui/labels.h"
#include "ui/container.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <thirdparty/qjson/src/parser.h>
#include "core/net/url.h"
#include "core/user.h"
#include "core/constants.h"
#include "core/application.h"
#include "core/barcode.h"
#include "../libs/rfidreader/rfidreader.h"
#include <QVBoxLayout>
#include <QProgressBar>
#include <QAction>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QProcess>
#include <QDir>
#include <QNetworkReply>
#include "startupthread.h"

LoginPanel::LoginPanel(Ui::Container *container /*= nullptr*/)
	: Inner(container)
	, userEdit(new Ui::FlatEdit("工号", this))
	, pwdEdit(new Ui::FlatEdit("密码", this))
	, error(new Ui::ErrorLabel("用户名或密码不正确", this))
	, bar(new QProgressBar(this))
	, _waiter(new WaitingSpinner(this))
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	Ui::Title *titleLabel = new Ui::Title("欢迎使用 Winstrac 桌面");
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

	bar->hide();
	layout->addWidget(bar);

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
	/*
	if (REMEMBER_READER && COMPORT_LIST.contains(LAST_COM)) {
		if (RfidReader::getInstance()->connect(LAST_COM.toStdString())) {
			error->shake(QString("RFID读卡器连接成功!"));
		}
		else {
			error->shake(QString("RFID读卡器连接失败，请登录后重新配置!"));
		}
	}
	else {
		error->shake(QString("RFID读卡器尚未连接，请登录后进行配置!"));
	}
	*/
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
				else {
					//get package Img
					getPkgImgs();
				}
			}
			else {
				error->shake(QString("获取版本号错误：%1").arg(resp.getAsString("msg")));
				updateSize();
			}
		}
	});
}

void LoginPanel::getPkgImgs() {
	QString path("./photo/package");
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.jpg" << "*.png";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

	QVariantList codeList;
	for each (QString file in files)
	{
		QVariantMap code_map;

		QString filefullPath = QString("%1/%2").arg(path).arg(file);
		QFileInfo fi(filefullPath);
		QString base = fi.baseName();
		QString md5 = getFileMd5(filefullPath);

		code_map.insert("package_type_id", base);
		code_map.insert("md_hash_code", md5);
		codeList << code_map;
	}

	QVariantMap vmap;
	vmap.insert("codes", codeList);

	post(url(PATH_PKGTPYE_PKGIDS), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			error->shake(QString("获取包图片错误：%1").arg(resp.errorString()));
		}
		else
		{
			_pkg_ids = resp.getAsList("pkg_ids");
			getInsImgs();
		}

	});
}

void LoginPanel::getInsImgs() {
	QString path("./photo/instrument");
	QDir dir(path);
	QStringList nameFilters;
	nameFilters << "*.jpg" << "*.png";
	QStringList files = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);

	QVariantList codeList;
	for each (QString file in files)
	{
		QVariantMap code_map;

		QString filefullPath = QString("%1/%2").arg(path).arg(file);
		QFileInfo fi(filefullPath);
		QString base = fi.baseName();
		QString md5 = getFileMd5(filefullPath);

		code_map.insert("ins_id", base);
		code_map.insert("md_hash_code", md5);
		codeList << code_map;
	}

	QVariantMap vmap;
	vmap.insert("codes", codeList);

	post(url(PATH_INSTRUMENT_INSIDS), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			error->shake(QString("获取器械图片错误：%1").arg(resp.errorString()));
		}
		else
		{
			_ins_ids = resp.getAsList("ins_ids");

			downloadPkgImgs();
			downloadInsImgs();
		}

	});
}

void LoginPanel::downloadPkgImgs() {
	QString path("./photo/package");

	if (_pkg_ids.size() > 0)
	{
		bar->setRange(0, _pkg_ids.size());
		bar->setHidden(false);
	}
	else
		return;
	
	for (int i = 0; i != _pkg_ids.count(); ++i) {
		int package_type_id = _pkg_ids[i].toInt();

		QString req("{\"package_type_id\":%1 }");
		QByteArray data;
		data.append(req.arg(package_type_id));

		QByteArray bytes = post(url(PATH_PKGTPYE_DOWNLOAD_IMG), data);

		if (bytes != nullptr)
		{
			QString filefullPath = QString("%1/%2.png").arg(path).arg(package_type_id);
			QFile file(filefullPath);

			if (file.open(QIODevice::WriteOnly))
				file.write(bytes);

			file.close();

			bar->setValue(i + 1);
			double dProgress = (bar->value() - bar->minimum()) * 100.0
				/ (bar->maximum() - bar->minimum());
			bar->setFormat(QString("正在更新包图片，当前进度为：%1%").arg(QString::number(dProgress, 'f', 1)));
		}
		else
		{
			bar->hide();
			error->shake(QString("下载包图片出错:%1").arg(package_type_id));
		}
	}
}

void LoginPanel::downloadInsImgs() {
	QString path("./photo/instrument");

	if (_ins_ids.size() > 0)
	{
		bar->setRange(0, _ins_ids.size());
		bar->setHidden(false);
	}
	else
		return;

	for (int i = 0; i != _ins_ids.count(); ++i) {
		int instrument_id = _ins_ids[i].toInt();

		QString req("{\"instrument_id\":%1 }");
		QByteArray data;
		data.append(req.arg(instrument_id));

		QByteArray bytes = post(url(PATH_INSTRUMENT_DOWNLOAD_IMG), data);

		if (bytes != nullptr)
		{
			QString filefullPath = QString("%1/%2.png").arg(path).arg(instrument_id);
			QFile file(filefullPath);

			if (file.open(QIODevice::WriteOnly))
				file.write(bytes);

			file.close();

			bar->setValue(i + 1);
			double dProgress = (bar->value() - bar->minimum()) * 100.0
				/ (bar->maximum() - bar->minimum());
			bar->setFormat(QString("正在更新器械图片，当前进度为：%1%").arg(QString::number(dProgress, 'f', 1)));
		}
		else
		{
			bar->hide();
			error->shake(QString("下载器械图片出错:%1").arg(instrument_id));
		}
	}
}

const QString LoginPanel::getFileMd5(QString filePath)
{
	QFile theFile(filePath);
	theFile.open(QIODevice::ReadOnly);
	QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
	theFile.close();
	return QString(ba.toHex().constData());
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
	Core::User &user = Core::currentUser();
	user.id = 11000001;
	user.deptId = 12000035;
	user.role = 1;
	user.gender = Core::User::Male;
	user.name = "Admin";
	container()->accept();
	/*Core::User &user = Core::currentUser();
	user.role = Core::User::Admin;
	user.deptId = 12000021;
	user.id = 11000008;
	_waiter->stop();
	container()->accept();
	return;

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
*/
}

LoginPage::LoginPage(QWidget *parent/* = nullptr*/) : Ui::Source(parent)
	, _headPic(new Ui::Picture(":/res/login-bg.jpg"))
	, _container(new Ui::Container(nullptr))
	, _thread(new StartupThread(this)){
	_headPic->setFixedHeight(260);
	_headPic->setBgFit(Ui::BgFit::Fill);
	//_headPic->setStyleSheet("background:red");

	_container->setInner(new LoginPanel(_container));
	connect(_container, &Ui::Container::accepted, this, &LoginPage::permitted);

	QVBoxLayout *_hLayout = new QVBoxLayout(this);
	_hLayout->setContentsMargins(0, 0, 0, 0);
	_hLayout->addWidget(_headPic);
	_hLayout->addWidget(_container);

	connect(_thread, SIGNAL(message(const QString&, bool)), this, SLOT(showInfo(const QString&, bool)), Qt::QueuedConnection);
	connect(_thread, SIGNAL(finished()), this, SLOT(onThreadFinished()), Qt::QueuedConnection);

	_thread->start();
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
