#include "recycle.h"
#include "tips.h"
#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "inliner.h"
#include "widget/controls/packageview.h"
#include "widget/controls/idedit.h"
#include "dialog/operatorchooser.h"
#include "dialog/regexpinputdialog.h"
#include "importextdialog.h"
#include "model/spinboxdelegate.h"
#include "xnotifier.h"

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
		if (_pkgModel->data(index.siblingAtColumn(0), Qt::UserRole + 1).toInt() == deptId)
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
	, _pkgView(new OrRecyclePackageView)
	, _detailView(new PackageDetailView)
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加包");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addWidget(Ui::createSeperator(Qt::Vertical));

	Ui::IconButton *addPlateButton = new Ui::IconButton(":/res/fill-plate-24.png", "手工添加网篮");
	hLayout->addWidget(addPlateButton);
	connect(addPlateButton, SIGNAL(clicked()), this, SLOT(addPlateEntry()));

	Ui::IconButton *extImportButton = new Ui::IconButton(":/res/add.png", "外部器械导入");
	hLayout->addWidget(extImportButton);
	connect(extImportButton, SIGNAL(clicked()), this, SLOT(chooseExt()));

	hLayout->addStretch();

	const QString text = "1 扫描或手工输入手术室物品包的条码\n2 扫描托盘进行装篮\n3 确认回收";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认回收");
	tip->addQr();
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->addWidget(_pkgView);
	bLayout->addWidget(_detailView);
	bLayout->setStretch(0, 2);
	bLayout->setStretch(1, 1);

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(hLayout, 0, 0);
	layout->addLayout(bLayout, 1, 0);
	layout->addWidget(tip, 1, 1);

	connect(_pkgView, SIGNAL(clicked(const QModelIndex &)), this, SLOT(showDetail(const QModelIndex &)));
	connect(_detailView, SIGNAL(sendData(int)), this, SLOT(updateRecord(int)));

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
}

void OrRecyclePanel::showDetail(const QModelIndex &index)
{
	_row = index.row();
	QString package_type_id = _pkgView->model()->data(_pkgView->model()->index(_row, 2), 260).toString();
	QString card_id = _pkgView->model()->data(_pkgView->model()->index(_row, 1)).toString();
	QString pkg_id = _pkgView->model()->data(_pkgView->model()->index(_row, 0)).toString();
	_detailView->loadDetail(pkg_id, package_type_id, card_id);
}

void OrRecyclePanel::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
}

void OrRecyclePanel::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void OrRecyclePanel::updateRecord(int pkg_record)
{
	_pkgView->model()->setData(_pkgView->model()->index(_row, 2), brushForSteType(pkg_record), Qt::BackgroundRole);
}

void OrRecyclePanel::chooseExt() {
	ImportExtDialog d(this);
	connect(&d, SIGNAL(extPkgImport(const QString&, const QString&, const QString&)), this, SLOT(setExtPkg(const QString&, const QString&, const QString&)));
	d.exec();
}

void OrRecyclePanel::setExtPkg(const QString& pkgId, const QString& pkgTypeId, const QString& pkgName) {
	_pkgView->addExtPackage(pkgId, pkgTypeId, pkgName);
}

void OrRecyclePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if ((bc.type() == Barcode::Package || bc.type() == Barcode::PkgCode) && !_pkgView->hasPackage(code)) {
		_pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Plate) {
		updatePlate(code);
	}
	else if (bc.type() == Barcode::Action && code == "910108") {
		commit();
	}
}

void OrRecyclePanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{18,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void OrRecyclePanel::addPlateEntry() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入网篮条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void OrRecyclePanel::removeEntry() {
	QItemSelectionModel *selModel = _pkgView->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_pkgView->model()->removeRow(indexes.at(i - 1).row());

	_detailView->clear();
}

void OrRecyclePanel::commit() {
	QVariantList packageIds = _pkgView->packageIds();
	QVariantList cardIds = _pkgView->cardIds();
	if (packageIds.isEmpty()) {
		XNotifier::warn("请先添加需要回收的包");
		return;
	}

	int plateId = _pkgView->plate();
	if (0 == plateId) {
		XNotifier::warn("请添加网篮");
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

void OrRecyclePanel::updatePlate(const QString &plateId) {
	_pkgView->updatePlate(plateId);
}

void OrRecyclePanel::reset() {
	_pkgView->clear();
	_detailView->clear();
}
