#include "kpipage.h"
#include "ui/views.h"
#include "core/net/url.h"
#include <QtWidgets/QtWidgets>

KpiPage::KpiPage(QWidget *parent)
	: QWidget(parent)
	, _yearCombo(new QComboBox)
	, _monthCombo(new QComboBox)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 9, _view))
{
	for (int i = 0; i != 13; ++i) {
		_monthCombo->addItem(QString::number(i));
	}
	for (int i = QDate::currentDate().year(), j=3; j>0; --i, --j) {
		_yearCombo->addItem(QString::number(i));
	}

	QGroupBox *groupBox = new QGroupBox(this);
	QPushButton *searchButton = new QPushButton(groupBox);
	connect(searchButton, SIGNAL(clicked()), this, SLOT(doSearch()));

	QHBoxLayout *hlayout = new QHBoxLayout(groupBox);
	hlayout->addWidget(new QLabel("查询月份:"));
	hlayout->addWidget(_yearCombo);
	hlayout->addWidget(new QLabel("年"));
	hlayout->addWidget(_monthCombo);
	hlayout->addWidget(new QLabel("月"));
	hlayout->addStretch();

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
}

KpiPage::~KpiPage()
{
}

void KpiPage::doSearch()
{

}
