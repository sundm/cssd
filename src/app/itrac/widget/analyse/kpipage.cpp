#include "kpipage.h"
#include "ui/views.h"
#include "core/net/url.h"
#include "ui/composite/waitingspinner.h"
#include "xnotifier.h"
#include "rdao/dao/operatordao.h"
#include "rdao/dao/historydao.h"
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
	_model->removeRows(0, _model->rowCount());

	int year = _yearCombo->currentText().toInt();
	int month = _monthCombo->currentText().toInt();
	HistoryDao dao;
	QList<OperatorHistory> opHisList;
	result_t res = dao.getOperatorHistoryList(year, month, &opHisList);
	if (res.isOk())
	{
		_model->insertRows(0, opHisList.count());
		for (int i = 0; i != opHisList.count(); ++i) {
			_model->setData(_model->index(i, 0), opHisList.at(i).name);
			_model->setData(_model->index(i, 1), opHisList.at(i).id);
			_model->setData(_model->index(i, 2), opHisList.at(i).recycleCount);
			_model->setData(_model->index(i, 3), opHisList.at(i).washCount);
			_model->setData(_model->index(i, 4), opHisList.at(i).packCount);
			_model->setData(_model->index(i, 5), opHisList.at(i).sterCount);
			_model->setData(_model->index(i, 6), opHisList.at(i).dispatchCount);
		}
	}
}
