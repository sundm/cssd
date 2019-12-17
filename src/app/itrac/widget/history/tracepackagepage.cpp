#include "tracepackagepage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "core/inliner.h"
#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

TracePackageInfoView::TracePackageInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _grid(new QGridLayout)
{
	_grid->setSpacing(20);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addLayout(_grid);
}

void TracePackageInfoView::init()
{
	_insNameLabel = new QLabel();
	_pkgNameLabel = new QLabel();
	_deptLabel = new QLabel();
	_insNumLabel = new QLabel();
	_totalCycleabel = new QLabel();
	_patientLabel = new QLabel();
	_cyclelBox = new QSpinBox();
	_cyclelBox->setMinimum(1);

	QFont font;
	font.setPointSize(16);
	_insNameLabel->setFont(font);
	_pkgNameLabel->setFont(font);
	_deptLabel->setFont(font);
	_insNumLabel->setFont(font);
	_totalCycleabel->setFont(font);
	_patientLabel->setFont(font);
}

void TracePackageInfoView::loadInfo(const QString &udi, const int &cycel, bool isPackage)
{
	_udi = udi;
	_cycle = cycel;
	_isPackage = isPackage;

	clear();
	init();

	QFont font;
	font.setPointSize(16);
	
	if (_isPackage)
	{
		_pkgNameLabel->setText("测试器械包");
		_deptLabel->setText("手术室");
		_insNumLabel->setText("6");
		_totalCycleabel->setText("12");
		_patientLabel->setText("张三，123456");

		_cyclelBox->setValue(_cycle);
		_cyclelBox->setMaximum(12);

		QLabel *namelabel = new QLabel(QString("包名称:"));
		namelabel->setFont(font);
		_grid->addWidget(namelabel, 0, 0);
		_grid->addWidget(_pkgNameLabel, 0, 1);

		QLabel *deptlabel = new QLabel(QString("所属科室:"));
		deptlabel->setFont(font);
		_grid->addWidget(deptlabel, 0, 2);
		_grid->addWidget(_deptLabel, 0, 3);

		QLabel *insNumlabel = new QLabel(QString("器械数量:"));
		insNumlabel->setFont(font);
		_grid->addWidget(insNumlabel, 0, 4);
		_grid->addWidget(_insNumLabel, 0, 5);

		QLabel *totalCyclelabel = new QLabel(QString("循环总次数:"));
		totalCyclelabel->setFont(font);
		_grid->addWidget(totalCyclelabel, 1, 0);
		_grid->addWidget(_totalCycleabel, 1, 1);

		QLabel *cyclelabel = new QLabel(QString("当前次数:"));
		cyclelabel->setFont(font);
		_grid->addWidget(cyclelabel, 1, 2);
		_grid->addWidget(_cyclelBox, 1, 3);

		QLabel *uselabel = new QLabel(QString("当前使用者:"));
		uselabel->setFont(font);
		_grid->addWidget(uselabel, 2, 0);
		_grid->addWidget(_patientLabel, 2, 1);
	}
	else
	{
		_insNameLabel->setText("测试器械");
		_pkgNameLabel->setText("测试器械包");
		_deptLabel->setText("手术室");
		_insNumLabel->setText("6");
		_totalCycleabel->setText("12");
		_patientLabel->setText("张三，123456");

		_cyclelBox->setValue(_cycle);
		_cyclelBox->setMaximum(12);

		QLabel *namelabel = new QLabel(QString("器械名称:"));
		namelabel->setFont(font);
		_grid->addWidget(namelabel, 0, 0);
		_grid->addWidget(_insNameLabel, 0, 1);

		QLabel *pkglabel = new QLabel(QString("所属包:"));
		pkglabel->setFont(font);
		_grid->addWidget(pkglabel, 0, 2);
		_grid->addWidget(_pkgNameLabel, 0, 3);

		QLabel *deptlabel = new QLabel(QString("所属科室:"));
		deptlabel->setFont(font);
		_grid->addWidget(deptlabel, 0, 4);
		_grid->addWidget(_deptLabel, 0, 5);

		QLabel *totalCyclelabel = new QLabel(QString("循环总次数:"));
		totalCyclelabel->setFont(font);
		_grid->addWidget(totalCyclelabel, 1, 0);
		_grid->addWidget(_totalCycleabel, 1, 1);

		QLabel *cyclelabel = new QLabel(QString("当前次数:"));
		cyclelabel->setFont(font);
		_grid->addWidget(cyclelabel, 1, 2);
		_grid->addWidget(_cyclelBox, 1, 3);

		QLabel *uselabel = new QLabel(QString("当前使用者:"));
		uselabel->setFont(font);
		_grid->addWidget(uselabel, 2, 0);
		_grid->addWidget(_patientLabel, 2, 1);
	}

	connect(_cyclelBox, SIGNAL(valueChanged(int)), this, SLOT(cycleChanged(int)));
}

void TracePackageInfoView::cycleChanged(int value)
{
	_cycle = value;
	updateInfo();
	emit cycle(_cycle);
}

void TracePackageInfoView::updateInfo()
{
	if (_isPackage)
	{
		_pkgNameLabel->setText("测试器械包");
		_deptLabel->setText("手术室");
		_insNumLabel->setText("6");
		_totalCycleabel->setText("12");
		_patientLabel->setText("李四，654321");
	}
	else
	{
		_insNameLabel->setText("测试器械");
		_pkgNameLabel->setText("测试器械包");
		_deptLabel->setText("手术室");
		_insNumLabel->setText("6");
		_totalCycleabel->setText("12");
		_patientLabel->setText("赵武，123654");
	}
}

void TracePackageInfoView::clear() {
	QLayoutItem *child;
	while ((child = _grid->takeAt(0)) != 0) {
		if (child->widget()) {
			delete child->widget();
		}
		delete child;
	}
}
/***************************************************************/

TraceDetailView::TraceDetailView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(State + 1);
	_model->setHeaderData(Operate, Qt::Horizontal, "操作流程");
	_model->setHeaderData(OpName, Qt::Horizontal, "操作人员");
	_model->setHeaderData(OpTime, Qt::Horizontal, "操作时间");
	_model->setHeaderData(State, Qt::Horizontal, "更多");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void TraceDetailView::loadDetail(const QString& udi, const int cycle)
{
	_model->removeRows(0, _model->rowCount());

	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem();
	rowItems << new QStandardItem();
	rowItems << new QStandardItem();

	for each (QStandardItem * item in rowItems)
	{
		item->setTextAlignment(Qt::AlignCenter);
	}
	_model->appendRow(rowItems);

	_btn = new QPushButton("详情");
	connect(_btn, SIGNAL(clicked(bool)), this, SLOT(clickButton()));
	_btn->setProperty("row", 0);
	setIndexWidget(_model->index(0, 3), _btn);
}

void TraceDetailView::clickButton()
{
	QPushButton *btn = (QPushButton *)sender();
	int row = btn->property("row").toInt();
}
/***************************************************************/

TracePackagePage::TracePackagePage(QWidget *parent)
	: QWidget(parent)
	, _searchBox(new SearchEdit)
	, _infoView(new TracePackageInfoView)
	, _detailView(new TraceDetailView)

{
	_searchBox->setPlaceholderText("请扫描UDI");
	_searchBox->setMinimumHeight(36);
	connect(_searchBox, &QLineEdit::returnPressed, this, &TracePackagePage::startTrace);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(_searchBox);
	vLayout->addWidget(_infoView);
	vLayout->addWidget(_detailView);
	_infoView->setHidden(true);
	_detailView->setHidden(true);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
	connect(_infoView, SIGNAL(cycle(int)), this, SLOT(onCycleChange(int)));
}

void TracePackagePage::onCycleChange(int cycle)
{
	_detailView->loadDetail(_udi, cycle);
}

void TracePackagePage::startTrace()
{
	onTransponderReceviced(_searchBox->text());
}

void TracePackagePage::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package)
	{
		_udi = code;
		_infoView->setHidden(false);
		_detailView->setHidden(false);
		_searchBox->setText(_udi);
		_infoView->loadInfo(_udi, 1);
		_detailView->loadDetail(_udi, 1);
	}

	if (tc.type() == TranspondCode::Instrument)
	{
		_udi = code;
		_infoView->setHidden(false);
		_detailView->setHidden(false);
		_searchBox->setText(_udi);
		_infoView->loadInfo(_udi, 1, false);
		_detailView->loadDetail(_udi, 1);
	}

}

void TracePackagePage::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}