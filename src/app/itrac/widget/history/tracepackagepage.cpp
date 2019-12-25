#include "tracepackagepage.h"
#include "barcode.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "core/inliner.h"
#include <ui/ui_commons.h>
#include <xui/searchedit.h>
#include "rdao/dao/PackageDao.h"
#include "rdao/dao/InstrumentDao.h"
#include "rdao/dao/tracedao.h"
#include "rdao/entity/trace.h"
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
	_pkgUDILabel = new QLabel();
	_deptLabel = new QLabel();
	_totalCycleLabel = new QLabel();
	_patientLabel = new QLabel();
	_surgeryLabel = new QLabel();
	_cyclelBox = new QSpinBox();
	_cyclelBox->setMinimum(1);

	QFont font;
	font.setPointSize(16);
	_insNameLabel->setFont(font);
	_pkgNameLabel->setFont(font);
	_pkgUDILabel->setFont(font);
	_deptLabel->setFont(font);
	_totalCycleLabel->setFont(font);
	_patientLabel->setFont(font);
	_surgeryLabel->setFont(font);
}

void TracePackageInfoView::loadInfo(const QString &udi, bool isPackage)
{
	_udi = udi;
	
	_isPackage = isPackage;

	clear();
	init();

	QFont font;
	font.setPointSize(14);
	
	if (_isPackage)
	{
		PackageDao dao;
		Package pkg;
		result_t resp = dao.getPackage(_udi, &pkg);
		if (resp.isOk())
		{
			_cycle = pkg.cycle;

			_pkgNameLabel->setText(QString("包名称：%1").arg(pkg.name));
			_pkgUDILabel->setText(QString("包UDI:%1").arg(pkg.udi));
			_deptLabel->setText(QString("所属科室：%1").arg(pkg.dept.name));
			_totalCycleLabel->setText(QString("循环总次数：%1").arg(QString::number(_cycle)));
			_cyclelBox->setValue(_cycle);
			_cyclelBox->setMaximum(_cycle);

			_grid->addWidget(_pkgNameLabel, 0, 0);
			_grid->addWidget(_pkgUDILabel, 0, 1);
			_grid->addWidget(_deptLabel, 0, 2);

			_grid->addWidget(_totalCycleLabel, 1, 0);

			QLabel *cyclelabel = new QLabel(QString("当前次数:"));
			cyclelabel->setFont(font);

			_grid->addWidget(cyclelabel, 1, 1);
			_grid->addWidget(_cyclelBox, 1, 2);

			TraceDao tDao;
			PackageFlow pkgFlow;
			resp = tDao.getPackageFlow(_udi, _cycle, &pkgFlow);
			if (resp.isOk())
			{
				
				QString patientName = pkgFlow.use.patientName;
				QString patientId = pkgFlow.use.patientId;
				QString surgeryName = pkgFlow.use.surgeryName;
				QString surgeryTime = pkgFlow.use.surgeryTime.toString("yyyy-MM-dd HH:mm:ss");
				if (!surgeryName.isEmpty()) {
					_surgeryLabel->setText(QString("手术名称：%1，  手术时间：%2").arg(surgeryName).arg(surgeryTime));

					_grid->addWidget(_surgeryLabel, 2, 0);
				}

				if (!patientName.isEmpty())
				{
					_patientLabel->setText(QString("患者姓名：%1，  住院号：%2")
						.arg(patientName).arg(patientId));
					_grid->addWidget(_patientLabel, 2, 1);
				}
				
				emit packageFlow(pkgFlow);
			}
			else
			{
				//todo
				return;
			}
			
		}
		else
		{
			//todo
			return;
		}
		
	}
	else
	{
		InstrumentDao dao;
		Instrument ins;
		result_t resp = dao.getInstrument(_udi, &ins);
		if (resp.isOk())
		{
			_cycle = ins.cycle;
			_insNameLabel->setText(QString("器械名称:%1").arg(ins.name));
			_pkgNameLabel->setText(QString("所属包:%1").arg(ins.packageName));
			_pkgUDILabel->setText(QString("所属包UDI:%1").arg(ins.packageUdi));
			_totalCycleLabel->setText(QString("循环总次数::%1").arg(QString::number(_cycle)));
			//_patientLabel->setText("张三，123456");

			_cyclelBox->setValue(_cycle);
			_cyclelBox->setMaximum(_cycle);

			_grid->addWidget(_insNameLabel, 0, 0);
			_grid->addWidget(_pkgNameLabel, 0, 1);
			_grid->addWidget(_pkgUDILabel, 0, 2);

			_grid->addWidget(_totalCycleLabel, 1, 0);

			QLabel *cyclelabel = new QLabel(QString("当前次数:"));
			cyclelabel->setFont(font);
			_grid->addWidget(cyclelabel, 1, 1);
			_grid->addWidget(_cyclelBox, 1, 2);

			TraceDao tDao;
			PackageFlow pkgFlow;
			resp = tDao.getPackageFlow(ins.packageUdi, ins.packageCycle, &pkgFlow);
			if (resp.isOk())
			{
				QString patientName = pkgFlow.use.patientName;
				QString patientId = pkgFlow.use.patientId;
				QString surgeryName = pkgFlow.use.surgeryName;
				QString surgeryTime = pkgFlow.use.surgeryTime.toString("yyyy-MM-dd HH:mm:ss");
				if (!surgeryName.isEmpty()) {
					_surgeryLabel->setText(QString("手术名称：%1，  手术时间：%2").arg(surgeryName).arg(surgeryTime));

					_grid->addWidget(_surgeryLabel, 2, 0);
				}

				if (!patientName.isEmpty())
				{
					_patientLabel->setText(QString("患者姓名：%1，  住院号：%2")
						.arg(patientName).arg(patientId));
					_grid->addWidget(_patientLabel, 2, 1);
				}

				emit packageFlow(pkgFlow);
			}
			else
			{
				//todo
				return;
			}
		}
		else
		{
			//todo
			return;
		}
		
	}

	connect(_cyclelBox, SIGNAL(valueChanged(int)), this, SLOT(cycleChanged(int)));
}

void TracePackageInfoView::cycleChanged(int value)
{
	_cycle = value;
	updateInfo();
}

void TracePackageInfoView::updateInfo()
{
	if (_isPackage)
	{
		TraceDao tDao;
		PackageFlow pkgFlow;
		result_t resp = tDao.getPackageFlow(_udi, _cycle, &pkgFlow);
		if (resp.isOk())
		{

			QString patientName = pkgFlow.use.patientName;
			QString patientId = pkgFlow.use.patientId;
			QString surgeryName = pkgFlow.use.surgeryName;
			QString surgeryTime = pkgFlow.use.surgeryTime.toString("yyyy-MM-dd HH:mm:ss");
			if (!patientName.isEmpty())
			{
				_patientLabel->setText(QString("%1,住院号:%2。手术名称:%3，手术时间:%4")
					.arg(patientName).arg(patientId).arg(surgeryName).arg(surgeryTime));
			}
			else
			{
				_patientLabel->clear();
			}

			emit packageFlow(pkgFlow);
		}
		else
		{
			//todo
			return;
		}
	}
	else
	{
		InstrumentDao dao;
		Instrument ins;
		result_t resp = dao.getInstrument(_udi, &ins);
		if (resp.isOk())
		{
			_insNameLabel->setText(ins.name);
			_pkgNameLabel->setText(ins.packageName);
			_deptLabel->setText(ins.packageUdi);

			TraceDao tDao;
			PackageFlow pkgFlow;
			resp = tDao.getPackageFlow(ins.packageUdi, ins.packageCycle, &pkgFlow);
			if (resp.isOk())
			{
				QString patientName = pkgFlow.use.patientName;
				QString patientId = pkgFlow.use.patientId;
				QString surgeryName = pkgFlow.use.surgeryName;
				QString surgeryTime = pkgFlow.use.surgeryTime.toString("yyyy-MM-dd HH:mm:ss");
				if (!patientName.isEmpty())
				{
					_patientLabel->setText(QString("%1,住院号:%2。手术名称:%3，手术时间:%4")
						.arg(patientName).arg(patientId).arg(surgeryName).arg(surgeryTime));
				}
				else
				{
					_patientLabel->clear();
				}

				emit packageFlow(pkgFlow);
			}
			else
			{
				//todo
				return;
			}
		}
		else
		{
			//todo
			return;
		}
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
	_model->setHeaderData(State, Qt::Horizontal, "详情");

	setModel(_model);
	setSelectionMode(QAbstractItemView::SingleSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = horizontalHeader();
	header->setStretchLastSection(true);

	setColumnWidth(Operate, 250);
	setColumnWidth(OpName, 200);
	setColumnWidth(OpTime, 350);
}

void TraceDetailView::loadDetail(const PackageFlow &pkgFlow)
{
	_model->removeRows(0, _model->rowCount());

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
	connect(_infoView, SIGNAL(packageFlow(const PackageFlow &)), _detailView, SLOT(loadDetail(const PackageFlow &)));
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
		_infoView->loadInfo(_udi);
	}

	if (tc.type() == TranspondCode::Instrument)
	{
		_udi = code;
		_infoView->setHidden(false);
		_detailView->setHidden(false);
		_searchBox->setText(_udi);
		_infoView->loadInfo(_udi, false);
	}

}

void TracePackagePage::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void TracePackagePage::clear()
{

}