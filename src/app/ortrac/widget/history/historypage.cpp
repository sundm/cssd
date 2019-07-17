#include "historypage.h"
#include "filterwidget.h"
#include "filtergroup.h"
#include "filter.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/composite/qpaginationwidget.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

HistoryPage::HistoryPage(QWidget *parent)
	: QWidget(parent)
	, _filterButton(new QPushButton("筛选"))
	, _paginator(new QPaginationWidget)
	, _view(new QTableView(this))
	, _filterWidget(nullptr)
	, _filter(nullptr)
	, _waiter(new WaitingSpinner(this))
	, _visibleCount(10)
{
	setStyleSheet("QTableView{border-style:solid;border-width:1px 0 0 0}");
	
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	
	QWidget *toolBar = new QWidget(this);
	QHBoxLayout *hlayout = new QHBoxLayout(toolBar);

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

IssueHistoryPage::IssueHistoryPage(QWidget *parent) :HistoryPage(parent) {
	_historyModel = new QStandardItemModel(0, 4, _view);
	_historyModel->setHeaderData(0, Qt::Horizontal, "操作人");
	_historyModel->setHeaderData(1, Qt::Horizontal, "患者编号");
	_historyModel->setHeaderData(2, Qt::Horizontal, "使用数量");
	_historyModel->setHeaderData(3, Qt::Horizontal, "使用时间");
	_view->setModel(_historyModel);

	doSearch(1);
}

FilterGroup * IssueHistoryPage::createFilterGroup()
{
	return new IssueFilterGroup(this);
}

void IssueHistoryPage::doSearch(int page)
{
	_historyModel->removeRows(0, _historyModel->rowCount());
	_waiter->start();

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

	post(url(PATH_ISSUE_SEARCH), vmap, [this, page](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));
		
		QList<QVariant> packages = resp.getAsList("use_infos");
		_historyModel->insertRows(0, packages.count());
		for (int i = 0; i != packages.count(); ++i) {
			QVariantMap map = packages[i].toMap();
			_historyModel->setData(_historyModel->index(i, 0), map["nurse_name"]);
			_historyModel->setData(_historyModel->index(i, 1), map["patient_id"]);
			_historyModel->setData(_historyModel->index(i, 2), map["package_number"]);
			_historyModel->setData(_historyModel->index(i, 3), map["use_time"]);
			_historyModel->setHeaderData(i, Qt::Vertical, (page - 1)*_visibleCount + 1 + i);
		}
	});
}

