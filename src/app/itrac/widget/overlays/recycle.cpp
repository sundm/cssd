#include "recycle.h"
#include "tips.h"
#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "core/user.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "inliner.h"
#include "widget/controls/packageview.h"
#include "widget/controls/idedit.h"
#include "dialog/operatorchooser.h"
#include "dialog/regexpinputdialog.h"
#include "importextdialog.h"
#include "model/spinboxdelegate.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/entity/operator.h"
#include "rdao/dao/instrumentdao.h"
#include "rdao/dao/flowdao.h"
#include "xnotifier.h"
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <QtWidgets/QtWidgets>

NoBCRecyclePanel::NoBCRecyclePanel(QWidget *parent)
	: CssdOverlayPanel(parent)
	, _deptEdit(new DeptEdit)
	, _pkgEdit(new PackageEdit)
	, _pkgView(new TableView)
{
	//bind(this);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	hLayout->addWidget(new QLabel("科室"));
	hLayout->addWidget(_deptEdit);
	hLayout->addWidget(new QLabel("包类型"));
	hLayout->addWidget(_pkgEdit);
	connect(_deptEdit, SIGNAL(changed(int)), this, SLOT(loadPkg(int)));

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/plus-24.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/delete-24.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	Ui::IconButton *addPlateButton = new Ui::IconButton(":/res/fill-plate-24.png");
	addPlateButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addPlateButton);
	connect(addPlateButton, SIGNAL(clicked()), this, SLOT(addPlate()));

	hLayout->addStretch(0);

	// setup package view info
	initPackageView();

	const QString text = "手术室丢失条码的物品包及临床未提交申领的物品包请使用无码回收";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认回收");
	tip->addQr();
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	//layout->setVerticalSpacing(15);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 1, 1);

	QTimer::singleShot(500, [this] { _deptEdit->load(DeptEdit::OPERATING_ROOM); });
}

void NoBCRecyclePanel::loadPkg(int depId)
{
	//if (Constant::OperatingRoomId == depId)
	//	_pkgEdit->loadForCategary("0");
	//else
	_pkgEdit->loadForDepartment(depId);
}

bool NoBCRecyclePanel::accept() {
	if (0 == _pkgModel->rowCount())
		return true;
	return QMessageBox::Yes == QMessageBox::question(this, "确认", "确认放弃已录入的数据并返回？");
}

void NoBCRecyclePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		updatePlate(code);
	}
	else if (bc.type() == Barcode::Department)
	{
		updateDept(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void NoBCRecyclePanel::updateDept(const QString &deptId) {
	if (Constant::OperatingRoomId != deptId.toInt())
	{
		XNotifier::warn(QString("请确认扫描手术室条码"));
		return;
	}
	_deptEdit->setCurrentIdPicked(deptId.toInt(), QString("手术室"));
	/*QByteArray data("{\"department_id\":");
	data.append(deptId).append('}');
	post(url(PATH_DEPT_SEARCH), data, [deptId, this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取科室信息"));
			return;
		}

		QVariantList deptList = resp.getAsList("department_list");

		if (deptList.count() != 1) {
			XNotifier::warn("系统内部错误，无法生成对应数量的科室");
			return;
		}

		const QVariantMap &pkg = deptList.at(0).toMap();

		QString deptName = pkg["department_name"].toString();
		_deptEdit->setCurrentIdPicked(deptId.toInt(), deptName);
	});*/
}

void NoBCRecyclePanel::addPlate() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入网篮条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void NoBCRecyclePanel::addEntry() {
	int deptId = _deptEdit->currentId();
	int pkgId = _pkgEdit->currentId();

	if (pkgId == 0) return;

	int existRow = findRow(deptId, pkgId);
	if (-1 == existRow) {
		QList<QStandardItem *> items;
		QStandardItem *depItem = new QStandardItem(_deptEdit->currentName());
		depItem->setData(_deptEdit->currentId());
		QStandardItem *pkgItem = new QStandardItem(_pkgEdit->currentName());
		pkgItem->setData(_pkgEdit->currentId());
		QStandardItem *plateItem = new QStandardItem("-");
		plateItem->setData(0);
		items << depItem << pkgItem << new QStandardItem("1") << new QStandardItem("请扫描网篮条码");
		_pkgModel->appendRow(items);
	}
	else {
		QStandardItem *countItem = _pkgModel->item(existRow, 2);
		int count = countItem->text().toInt();
		if (count < Constant::maxPackageCount) {
			countItem->setText(QString::number(count + 1));
			_pkgView->selectRow(existRow);
		}
	}
	_pkgEdit->reset();
}

void NoBCRecyclePanel::removeEntry() {
	QItemSelectionModel *selModel = _pkgView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_pkgModel->removeRow(indexes.at(i - 1).row());
}

void NoBCRecyclePanel::commit() {
	if (0 == _pkgModel->rowCount()) {
		XNotifier::warn("请先添加需要回收的包");
		return;
	}


	int plateId = _pkgModel->item(0, 3)->data().toInt();
	if (0 == plateId) {
		XNotifier::warn("请添加网篮");
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantList pkgTypeList;
	for (int i = 0; i != _pkgModel->rowCount(); i++) {
		QVariantMap packageType;
		packageType.insert("package_type_id", _pkgModel->item(i, 1)->data());
		packageType.insert("department_id", _pkgModel->item(i, 0)->data());
		packageType.insert("add_num", _pkgModel->item(i, 2)->text().toInt());
		packageType.insert("add_type", 5);
		pkgTypeList << packageType;
	}

	QVariantMap vmap;
	vmap.insert("package_types", pkgTypeList);
	vmap.insert("plate_id", plateId);
	vmap.insert("operator_id", opId);

	post(url(PATH_RECYCLE_ADD), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("回收登记失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("回收成功!");
			reset();
		}
	});
}

void NoBCRecyclePanel::initPackageView() {
	_pkgModel = new QStandardItemModel(0, 4, _pkgView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "来源科室");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "数量");
	_pkgModel->setHeaderData(3, Qt::Horizontal, "装篮");
	_pkgView->setModel(_pkgModel);

	_pkgView->setItemDelegate(new SpinBoxDelegate(
		2, Constant::maxPackageCount, Constant::minPackageCount, _pkgView));

	QHeaderView *header = _pkgView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);
}

int NoBCRecyclePanel::findRow(int deptId, int pkgId) {
	// filter package first
	QModelIndexList matches = _pkgModel->match(_pkgModel->index(0, 1), Qt::UserRole + 1, pkgId, -1);
	for (const QModelIndex &index : matches) {
		if (_pkgModel->data(index.sibling(index.row(), 0), Qt::UserRole + 1).toInt() == deptId)
			return index.row();
	}
	return -1;
}

void NoBCRecyclePanel::updatePlate(const QString &plateId) {
	QByteArray data("{\"plate_id\":");
	data.append(plateId).append('}');
	post(url(PATH_PLATE_SEARCH), data, [plateId, this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取编号[%1]的网篮信息").arg(plateId));
			return;
		}

		QList<QVariant> plates = resp.getAsList("plates");
		if (plates.isEmpty()) {
			XNotifier::warn(QString("编号[%1]的网篮不在系统资产目录中").arg(plateId));
			return;
		}

		QVariantMap map = plates[0].toMap();
		bool idle = "1" == map["is_finished"].toString();

		if (!idle) {
			XNotifier::warn(QString("网篮<%1>正在使用，无法添加").arg(plateId));
			return;
		}

		for (int i = 0; i != _pkgModel->rowCount(); ++i) {
			QStandardItem *item = _pkgModel->item(i, 3);
			item->setText(map["plate_name"].toString());
			item->setData(plateId.toInt());
		}
	});
}

void NoBCRecyclePanel::reset() {
	_pkgModel->removeRows(0, _pkgModel->rowCount());
}

OrRecyclePanel::OrRecyclePanel(QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _pkgView(new PackageInfoView)
	, _detailView(new PackageDetailView)
	, _unusualView(new UnusualInstrumentView)
	, _unusualCodes(new QStringList)
	, _scannedCodes(new QStringList)
	, _pkgImg(new XPicture(this))
	, _insImg(new XPicture(this))
{
	const QString text = "1 扫描物品包ID\n2 扫描托盘内器械\n3 确认回收 \n说明\n灰色：实际数量\n绿色：通过数量\n黄色：剩余数量\n红色：异常数量";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认回收");
	Ui::PrimaryButton *resetButton = new Ui::PrimaryButton("重置");
	tip->addQr();
	tip->addButton(resetButton);
	tip->addButton(commitButton);
	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QVBoxLayout *aLayout = new QVBoxLayout;
	aLayout->addWidget(_pkgView);
	aLayout->addWidget(_detailView);
	aLayout->addWidget(_unusualView);
	aLayout->setStretch(1, 1);

	QString fileName = QString("./photo/timg.png");
	_pkgImg->setBgColor(QColor(245, 246, 247));
	_pkgImg->setImage(fileName);
	_pkgImg->setMinimumWidth(300);
	_insImg->setBgColor(QColor(245, 246, 247));
	_insImg->setImage(fileName);
	_insImg->setMinimumWidth(300);

	QVBoxLayout *imgLayout = new QVBoxLayout;
	imgLayout->addWidget(_pkgImg);
	imgLayout->addWidget(_insImg);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addLayout(aLayout);
	layout->addLayout(imgLayout);
	layout->addWidget(tip);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
	
	connect(_detailView, SIGNAL(scand(const QString&)), this, SLOT(onScanned(const QString&)));
	connect(_detailView, SIGNAL(unusual(const QString&)), this, SLOT(onUnusual(const QString&)));
	connect(_detailView, SIGNAL(onclick(const QString&)), this, SLOT(loadInstrumentImg(const QString&)));

	_step = 0;
}

void OrRecyclePanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package && 0 == _step)
	{
		PackageDao dao;
		result_t resp = dao.getPackage(code, &_package, true);
		if (resp.isOk())
		{
			if (_package.status == Rt::Recycled) {
				XNotifier::warn("该包已回收，请勿重复回收");
				return;
			}

			_unusualCodes->clear();
			_scannedCodes->clear();

			_pkgView->updatePackageInfo(code, _package.name, _package.instruments.size());
			_detailView->loadDetail(&_package.instruments);
			loadPackageImg(code);
			_step = 1;
		}
		
	}

	if (tc.type() == TranspondCode::Instrument && 1 == _step)
	{
		InstrumentDao dao;
		Instrument it;
		result_t resp = dao.getInstrument(code, &it);
		if (resp.isOk())
		{
			_detailView->scanned(code);
		}
	}
}

void OrRecyclePanel::onScanned(const QString& code)
{
	if (!_scannedCodes->contains(code))
	{
		_scannedCodes->append(code);
		_pkgView->scanned();
	}
}

void OrRecyclePanel::onUnusual(const QString& code)
{
	if (!_unusualCodes->contains(code))
	{
		_unusualCodes->append(code);
		_pkgView->unusualed();
		_unusualView->addUnusual(code);
	}
}

void OrRecyclePanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	if (code.compare("910108") == 0)
	{
		commit();
	}
}

void OrRecyclePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if ((bc.type() == Barcode::Package || bc.type() == Barcode::PkgCode)) {//&& !_pkgView->hasPackage(code)) {
		//_pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Plate) {
		//updatePlate(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void OrRecyclePanel::commit() {
	if (_package.udi.isEmpty())
	{
		XNotifier::warn("请先添加需要回收的包");
		return;
	}

	if (!_pkgView->isScanFinished())
	{
		XNotifier::warn("尚未完成器械检查，或存在异常器械");
		return;
	}

	FlowDao dao;
	Operator op;
	op.id = Core::currentUser().id;
	op.name = Core::currentUser().name;

	result_t resp = dao.addRecycle(_package, op);
	if (resp.isOk())
	{
		XNotifier::warn("回收成功!");
		reset();
	}
	else
	{
		XNotifier::warn(QString("回收登记失败: ").append(resp.msg()));
	}
	/*
	QVariantList packageIds;// = _pkgView->packageIds();
	QVariantList cardIds;// = _pkgView->cardIds();
	if (packageIds.isEmpty()) {
		XNotifier::warn("请先添加需要回收的包");
		return;
	}

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantList pkgList;
	for (int i = 0; i != packageIds.size(); i++) {
		QVariantMap package;
		package.insert("package_id", packageIds[i].toString());
		package.insert("card_id", cardIds[i].toInt());
		package.insert("recycle_reason", 1);
		pkgList << package;
	}

	QVariantMap vmap;
	vmap.insert("packages", pkgList);
	//vmap.insert("plate_id", plateId);
	vmap.insert("operator_id", opId);

	post(url(PATH_RECYCLE_ADD), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("回收登记失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("回收成功!");
			reset();
		}
	});
	*/
}

void OrRecyclePanel::reset()
{
	_pkgView->reset();
	_detailView->reset();
	_unusualView->reset();

	_scannedCodes->clear();
	_unusualCodes->clear();
	_step = 0;

	QString fileName = QString("./photo/timg.png");
	_pkgImg->setImage(fileName);
	_insImg->setImage(fileName);
}

void OrRecyclePanel::loadPackageImg(const QString& udi)
{
	QString imgPath = QString("./photo/package/%1.png").arg(udi);
	QFile file(imgPath);
	if (file.exists()) {
		_pkgImg->setImage(imgPath);
	}
	else {
		QString fileName = QString("./photo/timg.png");
		_pkgImg->setImage(fileName);
	}
}

void OrRecyclePanel::loadInstrumentImg(const QString& udi)
{
	QString imgPath = QString("./photo/instrument/%1.png").arg(udi);
	QFile file(imgPath);
	if (file.exists()) {
		_insImg->setImage(imgPath);
	}
	else
	{
		QString fileName = QString("./photo/timg.png");
		_insImg->setImage(fileName);
	}
}