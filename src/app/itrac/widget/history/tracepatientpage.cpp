#include "tracepatientpage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "core/inliner.h"
#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

SurgeryView::SurgeryView(QWidget *parent)
	: QTreeView(parent), _model(new QStandardItemModel(0, Package + 1, this))
{
	_model->setHeaderData(Patient, Qt::Horizontal, "患者信息");
	_model->setHeaderData(Surgery, Qt::Horizontal, "手术信息");
	_model->setHeaderData(Package, Qt::Horizontal, "器械包信息");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = this->header();
	header->setSectionResizeMode(QHeaderView::Stretch);

	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
}

void SurgeryView::addPatient(const QString& patientId)
{
	_model->removeRows(0, _model->rowCount());

	QStandardItem *patientItem = new QStandardItem(QString("张三，%1").arg("123456"));
	patientItem->setData(123456);

	QStandardItem *surgeryItem = new QStandardItem("阑尾手术");
	surgeryItem->setData(12345, Qt::UserRole + 1);
	QStandardItem *packageItem = new QStandardItem("阑尾器械包");
	packageItem->setData("E2009A9050048AF000000203", Qt::UserRole + 1);
	packageItem->setData(1, Qt::UserRole + 2);
	
	surgeryItem->setChild(0, packageItem);
	patientItem->setChild(0, surgeryItem);

	_model->appendRow(patientItem);
	setExpanded(patientItem->index(), true);
}

void SurgeryView::clear()
{
	_model->removeRows(0, _model->rowCount());
}

void SurgeryView::onClicked(const QModelIndex &index)
{
	QString udi = index.data(Qt::UserRole + 1).toString();
	int cycle = index.data(Qt::UserRole + 2).toInt();

	emit packageClicked(udi, cycle);
}

/******************************************************************************/

PatientPackageInfoView::PatientPackageInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _pkgNameLabel(new QLabel)
	, _deptLabel(new QLabel)
	, _insNumLabel(new QLabel)
	, _totalCycleLabel(new QLabel)
	, _cyclelLabel(new QLabel)
{
	QFont font;
	font.setPointSize(16);
	_pkgNameLabel->setFont(font);
	_deptLabel->setFont(font);
	_insNumLabel->setFont(font);
	_totalCycleLabel->setFont(font);
	_cyclelLabel->setFont(font);

	QGridLayout *grid = new QGridLayout();
	grid->setSpacing(20);

	QLabel *namelabel = new QLabel(QString("包名称:"));
	namelabel->setFont(font);
	grid->addWidget(namelabel, 0, 0);
	grid->addWidget(_pkgNameLabel, 0, 1);

	QLabel *deptlabel = new QLabel(QString("所属科室:"));
	deptlabel->setFont(font);
	grid->addWidget(deptlabel, 0, 2);
	grid->addWidget(_deptLabel, 0, 3);

	QLabel *insNumlabel = new QLabel(QString("器械数量:"));
	insNumlabel->setFont(font);
	grid->addWidget(insNumlabel, 0, 4);
	grid->addWidget(_insNumLabel, 0, 5);

	QLabel *totalCyclelabel = new QLabel(QString("循环总次数:"));
	totalCyclelabel->setFont(font);
	grid->addWidget(totalCyclelabel, 1, 0);
	grid->addWidget(_totalCycleLabel, 1, 1);

	QLabel *cyclelabel = new QLabel(QString("当前次数:"));
	cyclelabel->setFont(font);
	grid->addWidget(cyclelabel, 1, 2);
	grid->addWidget(_cyclelLabel, 1, 3);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addLayout(grid);
}


void PatientPackageInfoView::loadInfo(const QString &udi, const int cycel)
{
	_pkgNameLabel->setText("测试器械包");
	_deptLabel->setText("手术室");
	_insNumLabel->setText("6");
	_totalCycleLabel->setText("12");
	_cyclelLabel->setText(QString::number(cycel));
}

void PatientPackageInfoView::clear()
{
	_pkgNameLabel->clear();
	_deptLabel->clear();
	_insNumLabel->clear();
	_totalCycleLabel->clear();
	_cyclelLabel->clear();
}

/*******************************************************************/

PatientPackageDetailView::PatientPackageDetailView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	_model->setColumnCount(State + 1);
	_model->setHeaderData(Operate, Qt::Horizontal, "操作流程");
	_model->setHeaderData(OpName, Qt::Horizontal, "操作人员");
	_model->setHeaderData(OpTime, Qt::Horizontal, "操作时间");
	_model->setHeaderData(State, Qt::Horizontal, "详情");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void PatientPackageDetailView::loadDetail(const QString& udi, const int cycle)
{
	_model->removeRows(0, _model->rowCount());

	QList<QStandardItem *> rowItems;
	rowItems << new QStandardItem();
	rowItems << new QStandardItem();
	rowItems << new QStandardItem();
	rowItems << new QStandardItem();

	for each (QStandardItem * item in rowItems)
	{
		item->setTextAlignment(Qt::AlignCenter);
	}
	_model->appendRow(rowItems);
}

void PatientPackageDetailView::clear()
{
	_model->removeRows(0, _model->rowCount());
}

/*******************************************************************/

TracePatientPage::TracePatientPage(QWidget *parent)
	: QWidget(parent), 
	_searchBox(new SearchEdit), 
	_surgeryView(new SurgeryView),
	_pkgInfoView(new PatientPackageInfoView),
	_pkgDetailView(new PatientPackageDetailView)
{
	_searchBox->setPlaceholderText("患者ID");
	_searchBox->setMinimumHeight(36);
	connect(_searchBox, &QLineEdit::returnPressed, this, &TracePatientPage::startTrace);

	QVBoxLayout *vLayout = new QVBoxLayout;
	vLayout->addWidget(_pkgInfoView);
	vLayout->addWidget(_pkgDetailView);
	vLayout->setStretch(1, 1);

	QHBoxLayout *hlayout = new QHBoxLayout();
	hlayout->addWidget(_surgeryView);
	hlayout->addLayout(vLayout);
	hlayout->setStretch(0, 2);
	hlayout->setStretch(1, 5);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(_searchBox);
	layout->addLayout(hlayout);
	
	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
	
	connect(_surgeryView, SIGNAL(packageClicked(const QString&, const int)), _pkgInfoView, SLOT(loadInfo(const QString &, const int)));
	connect(_surgeryView, SIGNAL(packageClicked(const QString&, const int)), _pkgDetailView, SLOT(loadDetail(const QString&, const int)));

}

void TracePatientPage::startTrace()
{
	tracePatient(_searchBox->text());
}

void TracePatientPage::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
}

void TracePatientPage::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	tracePatient(_searchBox->text());
}

void TracePatientPage::tracePatient(const QString &patientId)
{
	clear();
	_searchBox->setText(patientId);
	_surgeryView->addPatient(patientId);
}

void TracePatientPage::clear()
{
	_surgeryView->clear();
	_pkgInfoView->clear();
	_pkgDetailView->clear();
}