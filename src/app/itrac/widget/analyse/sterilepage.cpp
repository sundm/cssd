#include "sterilepage.h"
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

SterilePage::SterilePage(QWidget *parent)
	: QWidget(parent)
	, _filterButton(new QPushButton("筛选"))
	, _paginator(new QPaginationWidget)
	, _view(new QTableView(this))
	, _model(new QStandardItemModel(0, 17, _view))
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
	connect(_filterButton, &QAbstractButton::toggled, this, &SterilePage::showFilterPopup);

	QPushButton *refreshButton = new QPushButton("刷新", toolBar);
	refreshButton->setIcon(QIcon(":/res/refresh-24.png"));
	connect(refreshButton, &QPushButton::clicked, this, &SterilePage::refresh);

	hlayout->addWidget(refreshButton);
	hlayout->addStretch();

	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &SterilePage::doSearch);
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

	_barchartView = new QChartView();
	_barchartView->setRenderHint(QPainter::Antialiasing);
	_barchartView->chart()->legend()->setAlignment(Qt::AlignRight);
	_barchartView->setAutoFillBackground(true);

	QHBoxLayout *h_chartlayout = new QHBoxLayout();
	h_chartlayout->addWidget(_chartView);
	h_chartlayout->addWidget(_barchartView);

	mainLayout->addLayout(h_chartlayout);
	mainLayout->setStretch(1, 1);
	mainLayout->setStretch(2, 1);

	_model->setHeaderData(0, Qt::Horizontal, "灭菌器");
	_model->setHeaderData(1, Qt::Horizontal, "灭菌程序");
	_model->setHeaderData(2, Qt::Horizontal, "灭菌时间");
	_model->setHeaderData(3, Qt::Horizontal, "灭菌人");
	_model->setHeaderData(4, Qt::Horizontal, "物理监测审核人");
	_model->setHeaderData(5, Qt::Horizontal, "物理监测审核时间");
	_model->setHeaderData(6, Qt::Horizontal, "物理监测审核结果");
	_model->setHeaderData(7, Qt::Horizontal, "化学监测审核人");
	_model->setHeaderData(8, Qt::Horizontal, "化学监测审核时间");
	_model->setHeaderData(9, Qt::Horizontal, "化学监测审核结果");
	_model->setHeaderData(10, Qt::Horizontal, "生物监测审核人");
	_model->setHeaderData(11, Qt::Horizontal, "生物监测审核时间");
	_model->setHeaderData(12, Qt::Horizontal, "生物监测审核结果");
	_model->setHeaderData(13, Qt::Horizontal, "是否包外湿包");
	_model->setHeaderData(14, Qt::Horizontal, "是否包内湿包");
	_model->setHeaderData(15, Qt::Horizontal, "是否存在飞标");
	_model->setHeaderData(16, Qt::Horizontal, "是否包内卡不合格");

	_view->setModel(_model);
	doSearch();
}

SterilePage::~SterilePage() {
	delete _filter;
}

void SterilePage::refresh() {
	doSearch(1);
}

void SterilePage::onFilterChanged(Filter &f) {
	if (nullptr == _filter) {
		_filter = new Filter;
	}
	*_filter = std::move(f);

	_filterButton->setChecked(false);
	doSearch(1);
}

void SterilePage::showFilterPopup(bool show) {
	if (show) {
		if (!_filterWidget) {
			_filterWidget = new FilterWidget(createFilterGroup(), this);
			connect(_filterWidget, &FilterWidget::filterReady, this, &SterilePage::onFilterChanged);

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

FilterGroup * SterilePage::createFilterGroup()
{
	return new SterileAbnormalFilterGroup(this);
}

QString SterilePage::toString(int v)
{
	switch (v)
	{
	case 0: return "不合格";
	case 1: return "合格";
	case 2: return "未审核";
	default: return QString();
	}
}

void SterilePage::doSearch(int page) {
	_model->removeRows(0, _model->rowCount());
	_barchartView->chart()->removeAllSeries();
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

	post(url(PATH_STERILE_STATISTICS), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法查询历史记录：").append(resp.errorString()));
			return;
		}

		int count = resp.getAsInt("total_count");
		_paginator->setTotalPages(count / _visibleCount + (count % _visibleCount > 0));

		QList<QVariant> washes = resp.getAsList("sterilize_info");
		for (auto &wash : washes) {
			QVariantMap map = wash.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_name"].toString()));
			rowItems.append(new QStandardItem(map["device_mod_name"].toString()));
			rowItems.append(new QStandardItem(map["sterilize_start_time"].toString()));
			rowItems.append(new QStandardItem(map["operator_name"].toString()));
			
			rowItems.append(new QStandardItem(map["physical_test_operator_name"].toString()));
			rowItems.append(new QStandardItem(map["physical_test_time"].toString()));
			rowItems.append(new QStandardItem(toString(map["physical_test_result"].toInt())));
			rowItems.append(new QStandardItem(map["chemistry_test_operator_name"].toString()));
			rowItems.append(new QStandardItem(map["chemistry_test_time"].toString()));
			rowItems.append(new QStandardItem(toString(map["chemistry_test_result"].toInt())));
			rowItems.append(new QStandardItem(map["biology_test_operator_name"].toString()));
			rowItems.append(new QStandardItem(map["biology_test_time"].toString()));
			rowItems.append(new QStandardItem(toString(map["biology_test_result"].toInt())));

			rowItems.append(new QStandardItem(map["outside_result"].toInt() == 1 ? QString("否") : QString("是")));
			rowItems.append(new QStandardItem(map["inside_result"].toInt() == 1 ? QString("否") : QString("是")));
			rowItems.append(new QStandardItem(map["label_off"].toInt() == 1 ? QString("否") : QString("是")));
			rowItems.append(new QStandardItem(map["inside_fail"].toInt() == 1 ? QString("否") : QString("是")));

			_model->appendRow(rowItems);
		}

		QVariantMap infos = resp.getAsDict("check_statistics");
		int successCount = infos["sterilize_count_s"].toInt();
		int totalCount = infos["total_count"].toInt();
		int abnormalCount = totalCount - successCount;

		
		if (0 < totalCount)
		{
			//绘制饼图
			QPieSlice *slice1 = new QPieSlice();

			slice1->setLabelVisible(true);
			slice1->setLabelPosition(QPieSlice::LabelOutside);
			slice1->setBrush(QColor(151, 95, 228));
			slice1->setExploded();
			slice1->setExplodeDistanceFactor(0.2);
			slice1->setLabel(QString("异常记录数:%1条").arg(abnormalCount));

			QPieSlice *slice2 = new QPieSlice();
			slice2->setLabelVisible(true);
			slice2->setLabelPosition(QPieSlice::LabelOutside);
			slice2->setBrush(QColor(59, 160, 255));
			slice2->setLabel(QString("合格记录数:%1条").arg(successCount));

			QPieSeries *series = new QPieSeries();

			series->setPieSize(0.65);
			series->append(slice1);
			series->append(slice2);

			qreal abnormal = abnormalCount * 1.0 / totalCount;
			qreal success = successCount * 1.0 / totalCount;

			slice1->setValue(abnormal);
			slice2->setValue(success);

			_chartView->chart()->addSeries(series);

			if(0 < abnormal){
				//绘制柱状图
				QBarSeries *barseries = new QBarSeries();
				barseries->setLabelsVisible(true);
				barseries->setLabelsPosition(QAbstractBarSeries::LabelsInsideBase);

				if (0 < infos["phy"].toInt())
				{
					QBarSet *set = new QBarSet("物理检测不合格");
					*set << infos["phy"].toInt();
					barseries->append(set);
				}

				if (0 < infos["che"].toInt())
				{
					QBarSet *set = new QBarSet("化学检测不合格");
					*set << infos["che"].toInt();
					barseries->append(set);
				}

				if (0 < infos["bio"].toInt())
				{
					QBarSet *set = new QBarSet("生物检测不合格");
					*set << infos["bio"].toInt();
					barseries->append(set);
				}

				if (0 < infos["outside"].toInt())
				{
					QBarSet *set = new QBarSet("包外湿包数");
					*set << infos["outside"].toInt();
					barseries->append(set);
				}

				if (0 < infos["inside"].toInt())
				{
					QBarSet *set = new QBarSet("包内湿包数");
					*set << infos["inside"].toInt();
					barseries->append(set);
				}

				if (0 < infos["label"].toInt())
				{
					QBarSet *set = new QBarSet("飞标数");
					*set << infos["label"].toInt();
					barseries->append(set);
				}

				if (0 < infos["inside_f"].toInt())
				{
					QBarSet *set = new QBarSet("包内检测不合格");
					*set << infos["inside_f"].toInt();
					barseries->append(set);
				}

				_barchartView->chart()->addSeries(barseries);
			}
		}
		
	});
}
