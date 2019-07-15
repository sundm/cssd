#include "historypage.h"
#include "filterwidget.h"
#include "filtergroup.h"
#include "filter.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/composite/qpaginationwidget.h"
#include <QtWidgets/QtWidgets>

HistoryPage::HistoryPage(QWidget *parent)
	: QWidget(parent)
	, _filterButton(new QPushButton("筛选"))
	, _paginator(new QPaginationWidget)
	, _view(new QTableView(this))
	, _filterWidget(nullptr)
	, _filter(nullptr)
	, _visibleCount(10)
{
	setStyleSheet("QTableView{border-style:solid;border-width:1px 0 0 0}");
	
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	
	QWidget *toolBar = new QWidget(this);
	QHBoxLayout *hlayout = new QHBoxLayout(toolBar);
	/*QLabel *searchIcon = new QLabel(toolBar);
	searchIcon->setPixmap(QPixmap(":/res/search.png"));
	hlayout->addWidget(searchIcon);*/

	_filterButton->setIcon(QIcon(":/res/filter-24.png"));
	_filterButton->setCheckable(true);
	hlayout->addWidget(_filterButton);
	connect(_filterButton, &QAbstractButton::toggled, this, &HistoryPage::showFilterPopup);

	QPushButton *refreshButton = new QPushButton("刷新", toolBar);
	refreshButton->setIcon(QIcon(":/res/refresh-24.png"));
	hlayout->addWidget(refreshButton);
	hlayout->addStretch();
	
	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &HistoryPage::doSearch);
	hlayout->addWidget(_paginator);

	mainLayout->addWidget(toolBar);

	_view = new QTableView(this);
	_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mainLayout->addWidget(_view);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->setSectionResizeMode(QHeaderView::Stretch);
}

HistoryPage::~HistoryPage() {
	delete _filter;
}

void HistoryPage::onFilterChanged(Filter &f) {
	if (nullptr == _filter) {
		_filter = new Filter;
	}
	*_filter = std::move(f);

	_filterButton->setChecked(false);
	doSearch(1);
}

void HistoryPage::showFilterPopup(bool show) {
	if (show) {
		if (!_filterWidget) {
			_filterWidget = new FilterWidget(createFilterGroup(), this);
			connect(_filterWidget, &FilterWidget::filterReady, this, &HistoryPage::onFilterChanged);

			QSize minSize = _filterWidget->minimumSizeHint();
			QRect rc = _view->geometry();
			rc.setHeight(minSize.height());
			if (minSize.width() > rc.width())
				rc.setWidth(minSize.width());

			_filterWidget->setGeometry(rc);
		}
		_filterWidget->show();

		QPoint dp = _view->pos();
		dp.ry()++;
		QPropertyAnimation *pAnimation = new QPropertyAnimation(_filterWidget, "pos");
		pAnimation->setDuration(500);
		pAnimation->setEasingCurve(QEasingCurve::OutBack);
		pAnimation->setStartValue(QPoint(0, 0));
		pAnimation->setEndValue(dp);

		pAnimation->start(QAbstractAnimation::DeleteWhenStopped);
	}
	else {
		_filterWidget->hide();
	}
}

RecycleHistoryPage::RecycleHistoryPage(QWidget *parent) :HistoryPage(parent) {
	_historyModel = new QStandardItemModel(0, 7, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "包ID");
	_historyModel->setHeaderData(1, Qt::Horizontal, "包名");
	_historyModel->setHeaderData(2, Qt::Horizontal, "回收人员");
	_historyModel->setHeaderData(3, Qt::Horizontal, "回收时间");
	_historyModel->setHeaderData(4, Qt::Horizontal, "回收方式");
	_historyModel->setHeaderData(5, Qt::Horizontal, "来源科室");
	_historyModel->setHeaderData(6, Qt::Horizontal, "清洗盘");
	_view->setModel(_historyModel);

	doSearch();
}

FilterGroup * RecycleHistoryPage::createFilterGroup()
{
	return new RecycleFilterGroup(this);
}

void RecycleHistoryPage::doSearch(int page)
{
	_historyModel->removeRows(0, _historyModel->rowCount());
	Core::app()->startWaitingOn(this);

	QVariantMap vmap;
	if (_filter) {
		vmap.insert("start_time", _filter->condition(FilterFlag::StartDate).toDate());
		vmap.insert("end_time", _filter->condition(FilterFlag::EndDate).toDate());
	}
	else {
		vmap.insert("start_time", QDate::currentDate());
		vmap.insert("end_time", QDate::currentDate());
	}
	vmap.insert("page", page);
	vmap.insert("page_count", _visibleCount);

	Url::post(Url::PATH_RECYCLE_SEARCH, vmap, [this, page](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("items_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));
		
		QList<QVariant> packages = resp.getAsList("packages");
		_historyModel->insertRows(0, packages.count());
		for (int i = 0; i != packages.count(); ++i) {
			QVariantMap map = packages[i].toMap();
			_historyModel->setData(_historyModel->index(i, 0), map["package_id"]);
			_historyModel->setData(_historyModel->index(i, 1), map["package_type_name"]);
			_historyModel->setData(_historyModel->index(i, 2), map["operator_name"]);
			_historyModel->setData(_historyModel->index(i, 3), map["operation_time"]);
			_historyModel->setData(_historyModel->index(i, 4), map["recycle_reason"]);
			_historyModel->setData(_historyModel->index(i, 5), map["department_name"]);
			_historyModel->setData(_historyModel->index(i, 6), map["plate_name"]);
			_historyModel->setHeaderData(i, Qt::Vertical, (page - 1)*_visibleCount + 1 + i);
		}
	});
}

WashHistoryPage::WashHistoryPage(QWidget *parent) :HistoryPage(parent) {
	_historyModel = new QStandardItemModel(0, 8, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "清洗机");
	_historyModel->setHeaderData(1, Qt::Horizontal, "锅次");
	_historyModel->setHeaderData(2, Qt::Horizontal, "总锅次");
	_historyModel->setHeaderData(3, Qt::Horizontal, "清洗程序");
	_historyModel->setHeaderData(4, Qt::Horizontal, "清洗时间");
	_historyModel->setHeaderData(5, Qt::Horizontal, "清洗人");
	_historyModel->setHeaderData(6, Qt::Horizontal, "清洗审核人");
	_historyModel->setHeaderData(7, Qt::Horizontal, "审核结果");
	//_historyModel->setHeaderData(, Qt::Horizontal, "清洗盘");
	_view->setModel(_historyModel);
	doSearch();
}

FilterGroup * WashHistoryPage::createFilterGroup()
{
	//return new WashFilterGroup(this);
	return nullptr;
}

void WashHistoryPage::doSearch(int page) {
	_historyModel->removeRows(0, _historyModel->rowCount());
	Core::app()->startWaitingOn(this);

	QVariantMap vmap;
	if (_filter) {
		vmap.insert("start_time", _filter->condition(FilterFlag::StartDate).toDate());
		vmap.insert("end_time", _filter->condition(FilterFlag::EndDate).toDate());
	}
	else {
		vmap.insert("start_time", QDate::currentDate());
		vmap.insert("end_time", QDate::currentDate());
	}
	vmap.insert("page", page);
	vmap.insert("page_count", _visibleCount);

	Url::post(Url::PATH_WASH_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));

		QList<QVariant> washes = resp.getAsList("items");
		for (auto &wash : washes) {
			QVariantMap map = wash.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_name"].toString()));
			rowItems.append(new QStandardItem(map["device_cycle"].toString()));
			rowItems.append(new QStandardItem(map["total_cycle"].toString()));
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["wash_time"].toString()));
			rowItems.append(new QStandardItem(map["operator_name"].toString()));
			rowItems.append(new QStandardItem(map["operator_name"].toString()));
			rowItems.append(new QStandardItem(map["check_result"].toString()));
			_historyModel->appendRow(rowItems);
		}
	});
}

PackHistoryPage::PackHistoryPage(QWidget *parent) :HistoryPage(parent) {
	_historyModel = new QStandardItemModel(0, 6, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "包ID");
	_historyModel->setHeaderData(1, Qt::Horizontal, "包名");
	_historyModel->setHeaderData(2, Qt::Horizontal, "打包时间");
	_historyModel->setHeaderData(3, Qt::Horizontal, "打包方式");
	_historyModel->setHeaderData(4, Qt::Horizontal, "配包人");
	_historyModel->setHeaderData(5, Qt::Horizontal, "审核人");
	_view->setModel(_historyModel);

	doSearch();
}

FilterGroup * PackHistoryPage::createFilterGroup() {
	return 0;
}

void PackHistoryPage::doSearch(int page /*= 1*/) {
	_historyModel->removeRows(0, _historyModel->rowCount());
	Core::app()->startWaitingOn(this);

	QVariantMap vmap;
	if (_filter) {
		vmap.insert("start_time", _filter->condition(FilterFlag::StartDate).toDate());
		vmap.insert("end_time", _filter->condition(FilterFlag::EndDate).toDate());
	}
	else {
		vmap.insert("start_time", QDate::currentDate());
		vmap.insert("end_time", QDate::currentDate());
	}
	vmap.insert("page", page);
	vmap.insert("page_count", _visibleCount);

	Url::post(Url::PATH_PACK_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));

		QList<QVariant> washes = resp.getAsList("pack_info");
		for (auto &wash : washes) {
			QVariantMap map = wash.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["package_id"].toString()));
			rowItems.append(new QStandardItem(map["package_name"].toString()));
			rowItems.append(new QStandardItem(map["pack_time"].toString()));
			rowItems.append(new QStandardItem(map["pack_type_name"].toString()));
			rowItems.append(new QStandardItem(map["operator_name"].toString()));
			rowItems.append(new QStandardItem(map["check_operator_name"].toString()));
			_historyModel->appendRow(rowItems);
		}
	});
}

SterileHistoryPage::SterileHistoryPage(QWidget *parent) :HistoryPage(parent) {
	_historyModel = new QStandardItemModel(0, 16, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "灭菌器");
	_historyModel->setHeaderData(1, Qt::Horizontal, "锅次");
	_historyModel->setHeaderData(2, Qt::Horizontal, "总锅次");
	_historyModel->setHeaderData(3, Qt::Horizontal, "灭菌程序");
	_historyModel->setHeaderData(4, Qt::Horizontal, "灭菌时间");
	_historyModel->setHeaderData(5, Qt::Horizontal, "灭菌员");
	_historyModel->setHeaderData(6, Qt::Horizontal, "包数量");
	_historyModel->setHeaderData(7, Qt::Horizontal, "物理监测审核人");
	_historyModel->setHeaderData(8, Qt::Horizontal, "物理监测审核时间");
	_historyModel->setHeaderData(9, Qt::Horizontal, "物理监测审核结果");
	_historyModel->setHeaderData(10, Qt::Horizontal, "化学监测审核人");
	_historyModel->setHeaderData(11, Qt::Horizontal, "化学监测审核时间");
	_historyModel->setHeaderData(12, Qt::Horizontal, "化学监测审核结果");
	_historyModel->setHeaderData(13, Qt::Horizontal, "生物监测审核人");
	_historyModel->setHeaderData(14, Qt::Horizontal, "生物监测审核时间");
	_historyModel->setHeaderData(15, Qt::Horizontal, "生物监测审核结果");
	_view->setModel(_historyModel);

	doSearch();
}

FilterGroup * SterileHistoryPage::createFilterGroup() {
	return 0;
}

void SterileHistoryPage::doSearch(int page /*= 1*/) {
	_historyModel->removeRows(0, _historyModel->rowCount());
	Core::app()->startWaitingOn(this);

	QVariantMap vmap;
	if (_filter) {
		vmap.insert("start_time", _filter->condition(FilterFlag::StartDate).toDate());
		vmap.insert("end_time", _filter->condition(FilterFlag::EndDate).toDate());
	}
	else {
		vmap.insert("start_time", QDate::currentDate());
		vmap.insert("end_time", QDate::currentDate());
	}
	vmap.insert("page", page);
	vmap.insert("page_count", _visibleCount);

	Url::post(Url::PATH_STERILE_SEARCH, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));

		QList<QVariant> items = resp.getAsList("sterilize_info_list");
		for (auto &item : items) {
			QVariantMap map = item.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["ste_device_name"].toString()));
			rowItems.append(new QStandardItem(map["ste_cycle"].toString()));
			rowItems.append(new QStandardItem(map["total_cycle"].toString()));
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["sterilize_time"].toString()));
			rowItems.append(new QStandardItem(map["operator_id"].toString())); // TODO
			rowItems.append(new QStandardItem(map["chemistry_test_operator"].toString()));
			rowItems.append(new QStandardItem(map["physical_test_time"].toString()));
			rowItems.append(new QStandardItem(map["physical_test_result"].toString()));
			rowItems.append(new QStandardItem(map["chemistry_test_operator"].toString()));
			rowItems.append(new QStandardItem(map["chemistry_test_time"].toString()));
			rowItems.append(new QStandardItem(map["chemistry_test_result"].toString()));
			rowItems.append(new QStandardItem()); // TODO
			rowItems.append(new QStandardItem()); // TODO
			rowItems.append(new QStandardItem(map["biology_test_result"].toString()));
			_historyModel->appendRow(rowItems);
		}
	});
}

IssueHistoryPage::IssueHistoryPage(QWidget *parent /*= Q_NULLPTR*/) {
	_historyModel = new QStandardItemModel(0, 5, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "部门名称");
	_historyModel->setHeaderData(1, Qt::Horizontal, "操作员");
	_historyModel->setHeaderData(2, Qt::Horizontal, "患者编号");
	_historyModel->setHeaderData(3, Qt::Horizontal, "使用包数量");
	_historyModel->setHeaderData(4, Qt::Horizontal, "使用时间");
	_view->setModel(_historyModel);

	doSearch();
}

FilterGroup * IssueHistoryPage::createFilterGroup() {
	return 0;
}

void IssueHistoryPage::doSearch(int page /*= 1*/) {

}
