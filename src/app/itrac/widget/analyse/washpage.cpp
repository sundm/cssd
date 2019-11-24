#include "washpage.h"
#include "ui/views.h"
#include "filterwidget.h"
#include "filtergroup.h"
#include "filter.h"
#include "core/net/url.h"
#include "ui/composite/qpaginationwidget.h"
#include "ui/composite/waitingspinner.h"
#include "xnotifier.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>
#include <QtCharts>

WashPage::WashPage(QWidget *parent)
	: QWidget(parent)
	, _filterButton(new QPushButton("筛选"))
	, _paginator(new QPaginationWidget)
	, _view(new QTableView(this))
	, _model(new QStandardItemModel(0, 7, _view))
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
	connect(_filterButton, &QAbstractButton::toggled, this, &WashPage::showFilterPopup);

	QPushButton *refreshButton = new QPushButton("刷新", toolBar);
	refreshButton->setIcon(QIcon(":/res/refresh-24.png"));
	connect(refreshButton, &QPushButton::clicked, this, &WashPage::refresh);

	hlayout->addWidget(refreshButton);
	hlayout->addStretch();

	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &WashPage::doSearch);
	hlayout->addWidget(_paginator);

	mainLayout->addWidget(toolBar);

	_view = new QTableView(this);
	_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	mainLayout->addWidget(_view);

	QHeaderView *header = _view->horizontalHeader();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);
	header->setStretchLastSection(true);	

	_chartView = new QChartView();
	_chartView->setRenderHint(QPainter::Antialiasing);
	_chartView->chart()->legend()->setAlignment(Qt::AlignRight);
	_chartView->setAutoFillBackground(true);

	mainLayout->addWidget(_chartView);
	mainLayout->setStretch(1, 1);
	mainLayout->setStretch(2, 1);

	_model->setHeaderData(0, Qt::Horizontal, "清洗机");\
	_model->setHeaderData(1, Qt::Horizontal, "清洗程序");
	_model->setHeaderData(2, Qt::Horizontal, "清洗操作人");
	_model->setHeaderData(3, Qt::Horizontal, "清洗时间");
	_model->setHeaderData(4, Qt::Horizontal, "清洗审核人");
	_model->setHeaderData(5, Qt::Horizontal, "审核时间");
	_model->setHeaderData(6, Qt::Horizontal, "异常原因描述");
	_view->setModel(_model);
	doSearch();
}

WashPage::~WashPage() {
	delete _filter;
}

void WashPage::refresh() {
	doSearch(1);
}

void WashPage::onFilterChanged(Filter &f) {
	if (nullptr == _filter) {
		_filter = new Filter;
	}
	*_filter = std::move(f);

	_filterButton->setChecked(false);
	doSearch(1);
}

void WashPage::showFilterPopup(bool show) {
	if (show) {
		if (!_filterWidget) {
			_filterWidget = new FilterWidget(createFilterGroup(), this);
			connect(_filterWidget, &FilterWidget::filterReady, this, &WashPage::onFilterChanged);

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

FilterGroup * WashPage::createFilterGroup()
{
	return new WashAbnormalFilterGroup(this);
}

void WashPage::doSearch(int page) {
	_model->removeRows(0, _model->rowCount());
	_chartView->chart()->removeAllSeries();
	_waiter->start();

	QVariantMap vmap;
	if (_filter) {
		vmap.insert("start_time", _filter->condition(FilterFlag::StartDate).toDate());
		vmap.insert("end_time", _filter->condition(FilterFlag::EndDate).toDate());
		vmap.insert("operator_id", _filter->condition(FilterFlag::Operator).toInt());
		vmap.insert("device_id", _filter->condition(FilterFlag::Device).toInt());
	}
	else {
		vmap.insert("start_time", QDate::currentDate());
		vmap.insert("end_time", QDate::currentDate());
	}
	vmap.insert("page", page);
	vmap.insert("page_count", _visibleCount);

	post(url(PATH_WASH_STATISTICS), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));

		QList<QVariant> washes = resp.getAsList("wash_info");
		for (auto &wash : washes) {
			QVariantMap map = wash.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["washing_device_name"].toString()));
			rowItems.append(new QStandardItem(map["washing_mod_name"].toString()));
			rowItems.append(new QStandardItem(map["operator_name"].toString()));
			rowItems.append(new QStandardItem(map["wash_start_time"].toString()));
			rowItems.append(new QStandardItem(map["check_operator_name"].toString()));
			rowItems.append(new QStandardItem(map["check_time"].toString()));
			rowItems.append(new QStandardItem(map["check_desc"].toString()));
			_model->appendRow(rowItems);
		}

		QVariantMap infos = resp.getAsDict("check_statistics");
		int abnormalCount = infos["wash_count_f"].toInt();
		int successCount = infos["wash_count_s"].toInt();
		int totalCount = infos["total_count"].toInt();

		if (0 < totalCount)
		{	
			//绘制饼图
			QPieSlice *slice1 = new QPieSlice();

			slice1->setLabelVisible(true);
			slice1->setLabelPosition(QPieSlice::LabelOutside);
			slice1->setBrush(QColor(151, 95, 228));
			slice1->setExploded();
			slice1->setExplodeDistanceFactor(0.2);
			slice1->setLabel(QString("异常数:%1次").arg(abnormalCount));

			QPieSlice *slice2 = new QPieSlice();
			slice2->setLabelVisible(true);
			slice2->setLabelPosition(QPieSlice::LabelOutside);
			slice2->setBrush(QColor(59, 160, 255));
			slice2->setLabel(QString("合格数:%1次").arg(successCount));

			QPieSeries *series = new QPieSeries();

			series->setPieSize(0.65);
			series->append(slice1);
			series->append(slice2);

			qreal abnormal = abnormalCount * 1.0 / totalCount;
			qreal success = successCount * 1.0 / totalCount;

			slice1->setValue(abnormal);
			slice2->setValue(success);

			_chartView->chart()->addSeries(series);
		}
		
	});
}
