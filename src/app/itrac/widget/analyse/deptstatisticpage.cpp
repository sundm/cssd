#include "deptstatisticpage.h"
#include "core/net/url.h"
#include "ui/views.h"
#include "ui/composite/waitingspinner.h"
#include "xnotifier.h"
#include "widget/controls/idedit.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>
#include <QtCharts/QtCharts>

DeptStatisticsPage::DeptStatisticsPage(QWidget *parent)
	: QWidget(parent)
	, _yearCombo(new QComboBox)
	, _monthCombo(new QComboBox)
	, _deptEdit(new DeptEdit)
	, _view(new QChartView(this))
	, _chart(nullptr)
	, _waiter(new WaitingSpinner(this))
{
	for (int i = 1; i != 13; ++i) {
		_monthCombo->addItem(QString::number(i));
	}
	for (int i = QDate::currentDate().year(), j = 3; j > 0; --i, --j) {
		_yearCombo->addItem(QString::number(i));
	}
	QDate today = QDate::currentDate();
	_yearCombo->setCurrentIndex(_yearCombo->findText(QString::number(today.year())));
	_monthCombo->setCurrentIndex(_monthCombo->findText(QString::number(today.month())));

	QGroupBox *groupBox = new QGroupBox(this);
	QPushButton *searchButton = new QPushButton("查询", groupBox);
	connect(searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

	QHBoxLayout *hlayout = new QHBoxLayout(groupBox);
	hlayout->addWidget(new QLabel("科室:"));
	hlayout->addWidget(_deptEdit);
	hlayout->addWidget(new QLabel("查询月份:"));
	hlayout->addWidget(_yearCombo);
	hlayout->addWidget(new QLabel("年"));
	hlayout->addWidget(_monthCombo);
	hlayout->addWidget(new QLabel("月"));
	hlayout->addWidget(searchButton);
	hlayout->addStretch();

	_view->setRenderHint(QPainter::Antialiasing);
	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->addWidget(groupBox);
	vlayout->addWidget(_view);

	_deptEdit->load(DeptEdit::ALL);
}

DeptStatisticsPage::~DeptStatisticsPage()
{
}

void DeptStatisticsPage::doSearch()
{
	int deptId = _deptEdit->currentId();
	if (-1 == deptId) return;

	QVariantMap data;
	XDateScope ds = getMonthDateScope(_yearCombo->currentText().toInt(), _monthCombo->currentText().toInt());
	data["start_time"] = ds.from;
	data["end_time"] = ds.to;
	data["department_id"] = deptId;
	_waiter->start();
	_http.post(url(PATH_DEPT_STATISTICS), data, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取科室用包统计数据: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> datas = resp.getAsList("department_or_pkg_statistics");
		if (datas.size()) {
			updateChart(datas);
		}
	});
}

void DeptStatisticsPage::updateChart(const QList<QVariant> &data)
{
	QChart *chart = new QChart();

	QBarSeries *series = new QBarSeries();
	for (int i = 0; i != data.count(); ++i) {
		QVariantMap map = data[i].toMap();
		QString pkgName = map["package_type_name"].toString();
		int pkgNum = map["used_num"].toInt();
		QBarSet *set = new QBarSet(pkgName);
		*set << pkgNum;
		series->append(set);
	}

	chart->addSeries(series);
	chart->setTitle(_deptEdit->currentName()+"用包统计");
	chart->setAnimationOptions(QChart::SeriesAnimations);

	_view->setChart(chart);
	delete _chart;
	_chart = chart;
}
