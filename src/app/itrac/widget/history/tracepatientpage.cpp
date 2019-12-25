#include "tracepatientpage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "core/inliner.h"
#include "rdao/dao/tracedao.h"
#include "rdao/entity/trace.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/entity/surgery.h"
#include "rdao/entity/package.h"
#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

SurgeryView::SurgeryView(QWidget *parent)
	: QTreeView(parent), _model(new QStandardItemModel(0, 1, this))
{
	_model->setHeaderData(Patient, Qt::Horizontal, "患者信息");
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = this->header();
	header->setSectionResizeMode(QHeaderView::ResizeToContents);

	connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
}

void SurgeryView::addPatient(const QString& patientId)
{
	_model->removeRows(0, _model->rowCount());

	TraceDao dao;
	QList<Surgery> surgeries;
	result_t resp = dao.getPatientSurgeries(patientId.toInt(), &surgeries);
	if (resp.isOk())
	{
		if (surgeries.count() < 1)
		{
			//todo
			return;
		}

		QFont font;
		font.setPointSize(12);

		QStandardItem *patientItem = new QStandardItem(QString("%1，%2").arg(surgeries[0].patientName).arg(surgeries[0].patientId));
		//patientItem->setData(surgeries[0].patientId);
		QList<QStandardItem*> surs;
		for each (Surgery sur in surgeries)
		{
			QStandardItem *surgeryItem = new QStandardItem(sur.surgeryName);
			//surgeryItem->setData(sur.id, Qt::UserRole + 1);

			QList<QStandardItem*> packages;
			QList<Package> pkgs = sur.packages;
			for each (Package pkg in pkgs)
			{
				QStandardItem *packageItem = new QStandardItem(pkg.name);
				packageItem->setData(pkg.udi, Qt::UserRole + 1);
				packageItem->setData(pkg.cycle, Qt::UserRole + 2);

				packages.append(packageItem);
			}

			for each (QStandardItem * item in packages)
			{
				item->setFont(font);
			}

			surgeryItem->appendRows(packages);
			
			surs.append(surgeryItem);
		}

		for each (QStandardItem * item in surs)
		{
			item->setFont(font);
		}

		patientItem->appendRows(surs);
		patientItem->setFont(font);

		_model->appendRow(patientItem);
		setExpanded(patientItem->index(), true);
	}
	else
	{
		XNotifier::warn(QString("查询失败: ").append(resp.msg()));
		return;
	}
}

void SurgeryView::clear()
{
	_model->removeRows(0, _model->rowCount());
}

void SurgeryView::onClicked(const QModelIndex &index)
{
	QString udi = index.data(Qt::UserRole + 1).toString();
	int cycle = index.data(Qt::UserRole + 2).toInt();
	if (!udi.isEmpty())
	{
		emit packageClicked(udi, cycle);
	}
	
}

/******************************************************************************/

PatientPackageInfoView::PatientPackageInfoView(QWidget *parent /*= nullptr*/)
	: QWidget(parent)
	, _pkgNameLabel(new QLabel)
	, _pkgUDILabel(new QLabel)
	, _deptLabel(new QLabel)
	, _insNumLabel(new QLabel)
	, _totalCycleLabel(new QLabel)
	, _cyclelLabel(new QLabel)
{
	QFont font;
	font.setPointSize(14);
	_pkgNameLabel->setFont(font);
	_pkgUDILabel->setFont(font);
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

	QLabel *udilabel = new QLabel(QString("包UDI:"));
	udilabel->setFont(font);
	grid->addWidget(udilabel, 0, 2);
	grid->addWidget(_pkgUDILabel, 0, 3);

	QLabel *deptlabel = new QLabel(QString("所属科室:"));
	deptlabel->setFont(font);
	grid->addWidget(deptlabel, 0, 4);
	grid->addWidget(_deptLabel, 0, 5);

	//QLabel *insNumlabel = new QLabel(QString("器械数量:"));
	//insNumlabel->setFont(font);
	//grid->addWidget(insNumlabel, 0, 4);
	//grid->addWidget(_insNumLabel, 0, 5);

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
	PackageDao dao;
	Package pkg;
	result_t resp = dao.getPackage(udi, &pkg);
	if (resp.isOk())
	{
		_pkgNameLabel->setText(pkg.name);
		_pkgUDILabel->setText(pkg.name);
		_deptLabel->setText(pkg.dept.name);
		_totalCycleLabel->setText(QString::number(pkg.cycle));
		_cyclelLabel->setText(QString::number(cycel));
	}
	else
	{
		XNotifier::warn(QString("查询包信息失败: ").append(resp.msg()));
		clear();
	}
	
}

void PatientPackageInfoView::clear()
{
	_pkgNameLabel->clear();
	_pkgUDILabel->clear();
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

	QHeaderView *header = horizontalHeader();
	header->setStretchLastSection(true);

	setColumnWidth(Operate, 230);
	setColumnWidth(OpName, 160);
	setColumnWidth(OpTime, 300);
}

void PatientPackageDetailView::loadDetail(const QString& udi, const int cycle)
{
	_model->removeRows(0, _model->rowCount());

	TraceDao tDao;
	PackageFlow pkgFlow;
	result_t resp = tDao.getPackageFlow(udi, cycle, &pkgFlow);
	if (resp.isOk())
	{

		Rt::FlowStatus status = pkgFlow.status;//todo

		QList<QStandardItem *> totalItems;

		QList<QStandardItem *> rowWashItems;
		rowWashItems << new QStandardItem("清洗");
		rowWashItems << new QStandardItem(pkgFlow.wash.op);
		rowWashItems << new QStandardItem(pkgFlow.wash.startTime.toString("yyyy-MM-dd HH:mm:ss"));
		rowWashItems << new QStandardItem(QString("清洗机:%1 \r 清洗程序:%2 \r 清洗锅次:%3").arg(pkgFlow.wash.device).arg(pkgFlow.wash.program).arg(pkgFlow.wash.cycleCount));
		totalItems.append(rowWashItems);

		QList<QStandardItem *> rowPackItems;
		rowPackItems << new QStandardItem("配包");
		rowPackItems << new QStandardItem(pkgFlow.pack.op);
		rowPackItems << new QStandardItem(pkgFlow.pack.time.toString("yyyy-MM-dd HH:mm:ss"));
		rowPackItems << new QStandardItem(QString("审核人:%1").arg(pkgFlow.pack.checker));
		totalItems.append(rowPackItems);

		QList<QStandardItem *> rowSterItems;
		rowSterItems << new QStandardItem("灭菌");
		rowSterItems << new QStandardItem(pkgFlow.ster.op);
		rowSterItems << new QStandardItem(pkgFlow.ster.startTime.toString("yyyy-MM-dd HH:mm:ss"));
		rowSterItems << new QStandardItem(QString("灭菌器:%1 \r 灭菌程序:%2 \r 灭菌锅次:%3").arg(pkgFlow.ster.device).arg(pkgFlow.ster.program).arg(pkgFlow.ster.cycleCount));
		totalItems.append(rowSterItems);

		QList<QStandardItem *> rowphyChecItems;
		rowphyChecItems << new QStandardItem("物理监测审核");
		rowphyChecItems << new QStandardItem(pkgFlow.sterCheck.phyChecker);
		rowphyChecItems << new QStandardItem(pkgFlow.sterCheck.phyCheckTime.toString("yyyy-MM-dd HH:mm:ss"));
		rowphyChecItems << new QStandardItem(QString("监测结果:%1").arg(SterilizeVerdictToString(pkgFlow.sterCheck.phyResult)));
		totalItems.append(rowphyChecItems);

		QList<QStandardItem *> rowcheChecItems;
		rowcheChecItems << new QStandardItem("化学监测审核");
		rowcheChecItems << new QStandardItem(pkgFlow.sterCheck.cheChecker);
		rowcheChecItems << new QStandardItem(pkgFlow.sterCheck.cheCheckTime.toString("yyyy-MM-dd HH:mm:ss"));
		rowcheChecItems << new QStandardItem(QString("监测结果:%1").arg(SterilizeVerdictToString(pkgFlow.sterCheck.cheResult)));
		totalItems.append(rowcheChecItems);

		QList<QStandardItem *> rowbioChecItems;
		rowbioChecItems << new QStandardItem("生物监测审核");
		rowbioChecItems << new QStandardItem(pkgFlow.sterCheck.bioChecker);
		rowbioChecItems << new QStandardItem(pkgFlow.sterCheck.bioCheckTime.toString("yyyy-MM-dd HH:mm:ss"));
		rowbioChecItems << new QStandardItem(QString("监测结果:%1").arg(SterilizeVerdictToString(pkgFlow.sterCheck.bioResult)));
		totalItems.append(rowbioChecItems);

		QList<QStandardItem *> rowBindItems;
		rowBindItems << new QStandardItem("手术绑定");
		rowBindItems << new QStandardItem(pkgFlow.bind.op);
		rowBindItems << new QStandardItem(pkgFlow.bind.time.toString("yyyy-MM-dd HH:mm:ss"));
		rowBindItems << new QStandardItem(QString("——"));
		totalItems.append(rowBindItems);

		QList<QStandardItem *> rowPreCheckItems;
		rowPreCheckItems << new QStandardItem("术前检查");
		rowPreCheckItems << new QStandardItem(pkgFlow.preCheck.op);
		rowPreCheckItems << new QStandardItem(pkgFlow.preCheck.time.toString("yyyy-MM-dd HH:mm:ss"));
		rowPreCheckItems << new QStandardItem(QString("——"));
		totalItems.append(rowPreCheckItems);

		QList<QStandardItem *> rowPostCheckItems;
		rowPostCheckItems << new QStandardItem("术后清点");
		rowPostCheckItems << new QStandardItem(pkgFlow.postCheck.op);
		rowPostCheckItems << new QStandardItem(pkgFlow.postCheck.time.toString("yyyy-MM-dd HH:mm:ss"));
		rowPostCheckItems << new QStandardItem(QString("——"));
		totalItems.append(rowPostCheckItems);

		QFont font;
		font.setPointSize(12);
		for each (QStandardItem * item in totalItems)
		{
			item->setTextAlignment(Qt::AlignCenter);
			item->setFont(font);
		}

		_model->appendRow(rowWashItems);
		_model->appendRow(rowPackItems);
		_model->appendRow(rowSterItems);
		_model->appendRow(rowphyChecItems);
		_model->appendRow(rowcheChecItems);
		_model->appendRow(rowbioChecItems);
		_model->appendRow(rowBindItems);
		_model->appendRow(rowPreCheckItems);
		_model->appendRow(rowPostCheckItems);
	}
	else
	{
		//todo
		return;
	}
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
	hlayout->setStretch(0, 1);
	hlayout->setStretch(1, 4);

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