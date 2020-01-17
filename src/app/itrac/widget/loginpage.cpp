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
#include "ftpmanager.h"
#include "../libs/rfidreader/rfidreader.h"
#include "dialog/rfidreaderconfigerdialog.h"
#include "dialog/rfidconfigerdialog.h"
#include "dialog/registerdialog.h"
#include "rdao/dao/operatordao.h"
#include "rdao/dao/verdao.h"
#include "rdao/entity/operator.h"
#include "des/des3.h"
#include <QMessageBox>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QAction>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>
#include <QProcess>
#include <QDir>
#include <QSettings>
#include <QNetworkReply>
#include "startupthread.h"

const QString updateFilePath = "./update/update.exe";

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

	Ui::PrimaryButton *settingsButton = new Ui::PrimaryButton("配置RFID扫描枪", Ui::BtnSize::Small);
	connect(settingsButton, SIGNAL(clicked()), this, SLOT(showRfidConfiger()));
	layout->addWidget(settingsButton);
	layout->setSpacing(15);

	//Ui::addPrimaryShortcut(this, Qt::Key_Return, SLOT(submit()));
	//Ui::addPrimaryShortcut(this, Qt::Key_Enter, SLOT(submit()));
	connect(userEdit, SIGNAL(returnPressed()), this, SLOT(submit()));
	connect(pwdEdit, SIGNAL(returnPressed()), this, SLOT(submit()));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	connect(FtpManager::getInstance(), SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
	connect(FtpManager::getInstance(), SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
	connect(FtpManager::getInstance(), SIGNAL(downloadFinished()), this, SLOT(downloadFinished()));

	//todo
	//if (TSL1128Readers.count() > 0)
	//{
	//	TSL1128Readers[0]->connect();
	//}
	
	QTimer::singleShot(100, this, &LoginPanel::registerApp);
}

void LoginPanel::showRfidConfiger() {
	ConfigRfidDialog d(this);
	d.exec();
}

void LoginPanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	
	Barcode bc(code);
	if (bc.type() == Barcode::User) {
		login(bc.intValue(), "123456"); //todo
	}
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
	login(user.toInt(), pwd);
}

void LoginPanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::User) {
		login(bc.intValue(), "123456"); //todo
	}
}

const QString LoginPanel::getBioSn()
{
	return getWMIC("wmic bios get serialnumber");
}

const QString LoginPanel::getCpuId()
{
	return getWMIC("wmic cpu get processorid");
}

const QString LoginPanel::getWMIC(const QString &cmd)
{
	QProcess p;
	p.start(cmd);
	p.waitForFinished();
	QString result = QString::fromLocal8Bit(p.readAllStandardOutput());
	QStringList list = cmd.split(" ");
	result = result.remove(list.last(), Qt::CaseInsensitive);
	result = result.replace("\r", "");
	result = result.replace("\n", "");
	result = result.simplified();
	return result;
}

void LoginPanel::showRegisterDialog(const QString &code)
{
	RegisterDialog d(code, this);
	if (d.exec() != QDialog::Accepted)
	{
		QMessageBox::StandardButton rb = QMessageBox::critical(this, "验证失败", "验证失败，系统即将退出。", QMessageBox::Yes | QMessageBox::Yes);
		
		Core::app()->quit();
		
	}
}

void LoginPanel::registerApp() {
	QString cpuId = getCpuId();
	QString bioSn = getBioSn();
	QString src = bioSn.append(cpuId);
	QByteArray ba = QCryptographicHash::hash(src.toUtf8(), QCryptographicHash::Md5);
	QString head = QString(ba.toHex().constData());
	if (head.length() > 16)
	{
		head = head.left(16);
	}

	QString code = REGIST_CODE.remove("-");
	if (code.isEmpty() || code.length() < 16)
	{
		showRegisterDialog(head);
		return;
	}

	DES3 des(0x2BD6459F82C5B300, 0x952C49104881FF48, 0x2BD6459F82C5B300);

	bool isOK = false;
	ui64 uCode = code.toULongLong(&isOK, 16);
	ui64 uCPUid = head.toULongLong(&isOK, 16);
	if (isOK)
	{
		ui64 result = des.encrypt(uCPUid);
		if (result == uCode) {

			version();
			return;
		}
		showRegisterDialog(head);
	}
	else
	{
		showRegisterDialog(head);
		return;
	}
	
}

void LoginPanel::downloadError(QNetworkReply::NetworkError e)
{
	qDebug() << e;

	error->shake(QString("获取软件更新包错误：%1").arg(e));
}

void LoginPanel::downloadFinished()
{
	QProcess *myProcess = new QProcess(this);
	myProcess->startDetached(updateFilePath);
	qApp->quit();
	return;
}

void LoginPanel::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if (bytesReceived > 0 && bytesTotal > 0)
	{
		bar->setHidden(false);
		bar->setMaximum(bytesTotal);
		bar->setValue(bytesReceived);

		double dProgress = bytesReceived * 100.0 / bytesTotal;
		bar->setFormat(QString("正在下载软件更新包，当前进度为：%1%").arg(QString::number(dProgress, 'f', 1)));
	}
}

void LoginPanel::version()
{
	QString ver;
	QString md5;
	VerDao dao;
	result_t resp = dao.getVersion(1, &ver, &md5);
	if (resp.isOk() && !md5.isEmpty())
	{
		QFile updateFile(updateFilePath);
		if (updateFile.exists())
		{
			QString fileMd5 = getFileMd5(updateFilePath);
			if (fileMd5.compare(md5) == 0)
			{
				return;
			}
				
		}

		FtpManager::getInstance()->get(updateFilePath, updateFilePath);
	}
	
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
	if (theFile.exists())
	{
		theFile.open(QIODevice::ReadOnly);
		QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5);
		theFile.close();
		return QString(ba.toHex().constData());
	}
	else
	{
		return QString("");
	}
}

void LoginPanel::login(const int account, const QString &pwd) {
	OperatorDao dao;
	Operator op;
	result_t resp = dao.login(account, pwd, &op);
	if (resp.isOk())
	{
		Core::User &user = Core::currentUser();
		user.id = op.id;
		user.name = op.name;
		user.gender = op.gender == Rt::Gender::Female ? Core::User::Female : Core::User::Male;
		user.deptId = op.dept.id;
		user.deptName = op.dept.name;
		user.role = op.role;
		user.loginTime = op.lastLoginTime.toString("yyyy-MM-dd HH:mm:ss");
		
		container()->accept();
	}
	else
	{
		error->shake(resp.msg());
		updateSize();
	}
	

/*	_waiter->start();

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
	Core::User &user = Core::currentUser();
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
