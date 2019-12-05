#include "clinicpage.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "model/itemdelegate.h"
#include "widget/overlays/tips.h"
#include "widget/controls/idedit.h"
#include "widget/mainwindow.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>

namespace {
	constexpr QMargins PanelMargins = QMargins(200, 100, 200, 100);
}

OrderPanel::OrderPanel(QWidget *parent)
	: QWidget(parent)
	, _orderView(new TableView(this))
	, _pkgEdit(new PackageEdit)
{
	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);

	hLayout->addWidget(new QLabel("选择物品"));
	hLayout->addWidget(_pkgEdit);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加物品");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/minus-24.png", "删除选中物品");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), _orderView, SLOT(removeSeletedRows()));

	Ui::IconButton *plateButton = new Ui::IconButton(":/res/delete-24.png", "清空");
	hLayout->addWidget(plateButton);
	connect(plateButton, SIGNAL(clicked()), _orderView, SLOT(clear()));

	hLayout->addStretch();

	// setup package view info
	_model = new QStandardItemModel(0, 2, _orderView);
	_model->setHeaderData(0, Qt::Horizontal, "待申领物品");
	_model->setHeaderData(1, Qt::Horizontal, "物品数量");
	_orderView->setModel(_model);
	_orderView->setSelectionBehavior(QAbstractItemView::SelectRows);
	_orderView->setItemDelegate(new SpinBoxDelegate(
		1, Constant::maxPackageCount, Constant::minPackageCount, _orderView));

	const QString text = "科室每日申领限2次，订单由CSSD确认接收后无法修改，请谨慎操作";
	Tip *tip = new Tip(text);
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("提交订单");
	tip->addButton(commitButton);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(commit()));

	QGridLayout *layout = new QGridLayout(this);
	layout->setContentsMargins(PanelMargins);
	layout->addLayout(hLayout, 0, 0);
	layout->addWidget(_orderView, 1, 0);
	layout->addWidget(tip, 1, 1);

	_pkgEdit->loadForDepartment(Core::currentUser().deptId);
}

void OrderPanel::addEntry() {
	int pkgId = _pkgEdit->currentId();
	if (pkgId == 0) return;

	int existRow = _orderView->findRow(0, Qt::UserRole + 1, pkgId);
	if (-1 == existRow) {
		QList<QStandardItem *> items;
		QStandardItem *pkgItem = new QStandardItem(_pkgEdit->currentName());
		pkgItem->setData(_pkgEdit->currentId());
		items << pkgItem << new QStandardItem("1");
		_model->appendRow(items);
	}
	else {
		QStandardItem *countItem = _model->item(existRow, 1);
		int count = countItem->text().toInt();
		if (count < Constant::maxPackageCount) {
			countItem->setText(QString::number(count + 1));
			_orderView->selectRow(existRow);
		}
	}
}

void OrderPanel::commit() {
	if (0 == _model->rowCount()) return;

	Core::User &user = Core::currentUser();

	QVariantList packages;
	for (int i = 0; i != _model->rowCount(); i++) {
		QVariantMap package;
		package.insert("package_id", _model->item(i, 0)->data());
		package.insert("package_num", _model->item(i, 1)->text().toInt());
		packages << package;
	}

	QVariantMap vmap;
	vmap.insert("dept_id", user.deptId);
	vmap.insert("s_operator_id", user.id);
	vmap.insert("package_ids", packages);

	post(url(PATH_ORDER_ADD), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("提交订单失败: ").append(resp.errorString()));
		}
		else {
			XNotifier::warn("订单已提交!");
			_orderView->clear();
		}
	});
}


HistoryOrderPanel::HistoryOrderPanel(QWidget *parent)
	: QWidget(parent)
	, _orderView(new TableView(this))
	, _timeWidget(new QWidget)
	, _periodCombo(new QComboBox)
	, _fromDateEdit(new QDateEdit)
	, _toDateEdit(new QDateEdit)
	, _orderEdit(new Ui::FlatEdit)
{
	// setup package view info
	_model = new QStandardItemModel(0, 5, _orderView);
	_model->setHeaderData(0, Qt::Horizontal, "订单号");
	_model->setHeaderData(1, Qt::Horizontal, "订单生成时间");
	_model->setHeaderData(2, Qt::Horizontal, "科室");
	_model->setHeaderData(3, Qt::Horizontal, "提交人");
	_model->setHeaderData(4, Qt::Horizontal, "订单状态");
	_orderView->setModel(_model);
	_orderView->setSelectionBehavior(QAbstractItemView::SelectRows);
	_orderView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	// create search group on the right hand
	QGroupBox *searchGroup = new QGroupBox(this);
	QVBoxLayout *vLayout = new QVBoxLayout(searchGroup);

	QTabBar *tabBar = new QTabBar(searchGroup);
	tabBar->addTab("按时间查询");
	tabBar->addTab("按订单号查询");
	connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(changeSearchMethod(int)));

	QWidget *searchPanel = new QWidget(searchGroup);
	QVBoxLayout *panelLayout = new QVBoxLayout(searchPanel);
	panelLayout->setSpacing(20);

	QHBoxLayout *timeLayout = new QHBoxLayout(_timeWidget);
	timeLayout->setContentsMargins(0, 0, 0, 0);
	_periodCombo->addItem("今天", XPeriod::Today);
	_periodCombo->addItem("本周", XPeriod::ThisWeek);
	_periodCombo->addItem("本月", XPeriod::ThisMonth);
	_periodCombo->addItem("自定义", XPeriod::Custom);
	connect(_periodCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onPeriodChanged(int)));
	_fromDateEdit = new QDateEdit;
	_toDateEdit = new QDateEdit;
	timeLayout->addWidget(_periodCombo);
	timeLayout->addWidget(_fromDateEdit);
	timeLayout->addWidget(_toDateEdit);

	_orderEdit = new Ui::FlatEdit;
	_orderEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_orderEdit->hide();
	Ui::PrimaryButton *searchButton = new Ui::PrimaryButton("查询");
	connect(searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

	panelLayout->addStretch();
	panelLayout->addWidget(_timeWidget);
	panelLayout->addWidget(_orderEdit);
	panelLayout->addWidget(searchButton);
	panelLayout->addStretch();

	vLayout->addWidget(tabBar);
	vLayout->addWidget(searchPanel);

	// install the main layout
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(PanelMargins);
	layout->addWidget(_orderView);
	layout->addWidget(searchGroup);
	//layout->setStretch(0, 1);
}

void HistoryOrderPanel::changeSearchMethod(int index) {
	bool byOrder = 1 == index;
	_orderEdit->setVisible(byOrder);
	_timeWidget->setVisible(!byOrder);
}

void HistoryOrderPanel::onPeriodChanged() {
	bool enable = XPeriod::Custom == _periodCombo->currentData();
	_fromDateEdit->setEnabled(enable);
	_toDateEdit->setEnabled(enable);
}

void HistoryOrderPanel::doSearch() {

}

ClinicPage::ClinicPage(QWidget *parent)
	: QTabWidget(parent)
{
	this->addTab(new OrderPanel, "申领");
	this->addTab(new HistoryOrderPanel, "查询");
}
