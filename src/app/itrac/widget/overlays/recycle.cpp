#include "recycle.h"
#include "tips.h"

#include "core/constants.h"
#include "core/barcode.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
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
	connect(_deptEdit, SIGNAL(changed(int)), _pkgEdit, SLOT(loadForDepartment(int)));

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/add.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/minus.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch(0);

	// setup package view info
	initPackageView();

	const QString text = "手术室丢失条码的物品包及临床未提交申领的物品包请使用无码回收";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认回收");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	//layout->setVerticalSpacing(15);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 1, 1);

	QTimer::singleShot(500, [this] { _deptEdit->load(DeptEdit::ALL); });
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
		items << depItem << pkgItem << new QStandardItem("1") << new QStandardItem("请扫描篮筐条码");
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
	if (0 == _pkgModel->rowCount()) return;

	int plateId = _pkgModel->item(0, 3)->data().toInt();
	if (0 == plateId) return;

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

	Url::post(Url::PATH_RECYCLE_ADD, vmap, [this](QNetworkReply *reply) {
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
	Plate::fetchOnce(plateId, [this](Plate* plate) {
		if (!plate->err.isEmpty()) {
			return;
		}

		if (!plate->idle) {
			XNotifier::warn(QString("该篮筐<%1>正在使用，无法添加").arg(plate->name));
			return;
		}

		for (int i = 0; i != _pkgModel->rowCount(); ++i) {
			QStandardItem *item = _pkgModel->item(i, 3);
			item->setText(plate->name);
			item->setData(plate->id);
		}
	});
}

void NoBCRecyclePanel::reset() {
	_pkgModel->removeRows(0, _pkgModel->rowCount());
}

OrRecyclePanel::OrRecyclePanel(QWidget *parent /*= nullptr*/)
	: CssdOverlayPanel(parent)
	, _pkgView(new OrRecyclePackageView)
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addWidget(Ui::createSeperator(Qt::Vertical));

	Ui::IconButton *extImportButton = new Ui::IconButton(":/res/fill-plate-24.png", "外部器械导入   ");
	hLayout->addWidget(extImportButton);
	connect(extImportButton, SIGNAL(clicked()), this, SLOT(chooseExt()));

	hLayout->addStretch();

	const QString text = "1 扫描或手工输入手术室物品包的条码\n2 扫描托盘进行装篮\n3 确认回收";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("确认回收");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_pkgView, 1, 0);
	layout->addWidget(tip, 1, 1);
}

void OrRecyclePanel::chooseExt() {
	ImportExtDialog d(this);
	connect(&d, SIGNAL(extPkgImport(const QString&,const QString&)), this, SLOT(setExtPkg(const QString&, const QString&)));
	d.exec();
}

void OrRecyclePanel::setExtPkg(const QString& pkgId, const QString& pkgName) {
	_pkgView->addExtPackage(pkgId, pkgName);
}

void OrRecyclePanel::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package && !_pkgView->hasPackage(code)) {
		_pkgView->addPackage(code);
	}
	else if (bc.type() == Barcode::Plate) {
		updatePlate(code);
	}
}

void OrRecyclePanel::addEntry() {
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包或篮筐条码", "", regExp, &ok);
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
}

void OrRecyclePanel::commit() {
	QVariantList packages = _pkgView->packages();
	if (packages.isEmpty()) return;

	int plateId = _pkgView->plate();
	if (0 == plateId) return;

	int opId = OperatorChooser::get(this, this);
	if (0 == opId) return;

	QVariantList pkgList;
	for (int i = 0; i != packages.size(); i++) {
		QVariantMap package;
		package.insert("package_id", packages[i]);
		package.insert("recycle_reason", 1);
		pkgList << package;
	}

	QVariantMap vmap;
	vmap.insert("packages", pkgList);
	vmap.insert("plate_id", plateId);
	vmap.insert("operator_id", opId);

	Url::post(Url::PATH_RECYCLE_ADD, vmap, [this](QNetworkReply *reply) {
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
}
