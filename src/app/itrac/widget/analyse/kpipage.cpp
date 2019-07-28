#include "kpipage.h"
#include "ui/views.h"
#include "core/net/url.h"
#include "ui/composite/waitingspinner.h"
#include "xnotifier.h"
#include <xernel/xtimescope.h>
#include <QtWidgets/QtWidgets>

KpiPage::KpiPage(QWidget *parent)
	: QWidget(parent)
	, _yearCombo(new QComboBox)
	, _monthCombo(new QComboBox)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 7, _view))
	, _waiter(new WaitingSpinner(this))
{
	for (int i = 1; i != 13; ++i) {
		_monthCombo->addItem(QString::number(i));
	}
	for (int i = QDate::currentDate().year(), j=3; j>0; --i, --j) {
		_yearCombo->addItem(QString::number(i));
	}
	QDate today = QDate::currentDate();
	_yearCombo->setCurrentIndex(_yearCombo->findText(QString::number(today.year())));
	_monthCombo->setCurrentIndex(_monthCombo->findText(QString::number(today.month())));

	QGroupBox *groupBox = new QGroupBox(this);
	QPushButton *searchButton = new QPushButton("查询", groupBox);
	connect(searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

	QHBoxLayout *hlayout = new QHBoxLayout(groupBox);
	hlayout->addWidget(new QLabel("查询月份:"));
	hlayout->addWidget(_yearCombo);
	hlayout->addWidget(new QLabel("年"));
	hlayout->addWidget(_monthCombo);
	hlayout->addWidget(new QLabel("月"));
	hlayout->addWidget(searchButton);
	hlayout->addStretch();

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->addWidget(groupBox);
	vlayout->addWidget(_view);

	_model->setHeaderData(0, Qt::Horizontal, "姓名");
	_model->setHeaderData(1, Qt::Horizontal, "编号");
	_model->setHeaderData(2, Qt::Horizontal, "回收");
	_model->setHeaderData(3, Qt::Horizontal, "清洗");
	_model->setHeaderData(4, Qt::Horizontal, "打包");
	_model->setHeaderData(5, Qt::Horizontal, "灭菌");
	_model->setHeaderData(6, Qt::Horizontal, "发放");
	_model->setHeaderData(7, Qt::Horizontal, "");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	doSearch();
}

KpiPage::~KpiPage()
{
}

void KpiPage::doSearch()
{
	if (_model->rowCount()) {
		queryUserJobs();
		return;
	}

	// query user first
	_waiter->start();
	QByteArray data("{\"role_id\":");
	data.append(QString::number(5)).append('}'); // todo: CSSD = 5, define it as a constant
	_http.post(url(PATH_USER_SEARCH), data, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取用户列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> users = resp.getAsList("user_list");
		_model->insertRows(0, users.count());
		for (int i = 0; i != users.count(); ++i) {
			QVariantMap map = users[i].toMap();
			_model->setData(_model->index(i, 0), map["name"]);
			_model->setData(_model->index(i, 1), map["operator_id"]);
		}

		queryUserJobs();
	});
}

void KpiPage::queryUserJobs()
{
	QVariantList userList;
	for (int i = 0; i != _model->rowCount(); ++i) {
		userList.append(_model->data(_model->index(i, 1)));
	}

	QVariantMap data;
	XDateScope ds = getMonthDateScope(_yearCombo->currentText().toInt(), _monthCombo->currentText().toInt());
	data["start_time"] = ds.from;
	data["end_time"] = ds.to;
	data["operator_ids"] = userList;
	_waiter->start();
	_http.post(url(PATH_OP_STATISTICS), data, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取用户绩效数据: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> datas = resp.getAsList("operator_statistics");
		for (int i = 0; i != datas.count(); ++i) {
			QVariantMap map = datas[i].toMap();
			QVariantList &list = map["pkg_nums"].toList();
			_model->setData(_model->index(i, 2), list[0]);
			_model->setData(_model->index(i, 3), list[1]);
			_model->setData(_model->index(i, 4), list[2]);
			_model->setData(_model->index(i, 5), list[3]);
			_model->setData(_model->index(i, 6), list[5]);
		}
	});
}
