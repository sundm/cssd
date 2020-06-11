#include "changeUDIdialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "barcode.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "model/itemdelegate.h"
#include "ui/composite/waitingspinner.h"
#include "rdao/dao/instrumentdao.h"
#include "dialog/scanTipsdialog.h"
#include <QtWidgets/QtWidgets>

ChangeUDIDialog::ChangeUDIDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgCodeLabel(new QLabel)
	, _pkgTypeLabel(new QLabel)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 4, _view))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("替换UDI标签");

	FormGroup * pkgGroup = new FormGroup(this);
	
	pkgGroup->addRow("包UDI:", _pkgCodeLabel);
	pkgGroup->addRow("包类型:", _pkgTypeLabel);

	FormGroup * viewGroup = new FormGroup(this);
	viewGroup->addRow("器械列表",_view);


	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	_resetButton = new Ui::PrimaryButton("重置", Ui::BtnSize::Small);
	connect(_resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	_loadPkgUDIButton = new Ui::PrimaryButton("替换包UDI", Ui::BtnSize::Small);
	connect(_loadPkgUDIButton, SIGNAL(clicked()), this, SLOT(onChangePkgUDIBtn()));

	_checkInsButton = new Ui::PrimaryButton("检查器械", Ui::BtnSize::Small);
	connect(_checkInsButton, SIGNAL(clicked()), this, SLOT(onCheckInsBtn()));

	QVBoxLayout *vlayout = new QVBoxLayout();
	vlayout->addWidget(pkgGroup);
	vlayout->addWidget(viewGroup);


	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->addWidget(_loadPkgUDIButton);
	bLayout->addWidget(_checkInsButton);
	bLayout->addWidget(_resetButton);
	bLayout->addWidget(_commitButton);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(vlayout);
	layout->addLayout(bLayout);

	initInstrumentView();

	resize(1000, 900);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	QTimer::singleShot(1000, this, &ChangeUDIDialog::reset);
}

void ChangeUDIDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "器械类型");
	_model->setHeaderData(1, Qt::Horizontal, "当前器械UDI");
	_model->setHeaderData(2, Qt::Horizontal, "状态");
	_model->setHeaderData(3, Qt::Horizontal, "替换器械UDI");

	_view->setModel(_model);
	_view->setMinimumHeight(500);

	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_view, &QWidget::customContextMenuRequested,
		this, &ChangeUDIDialog::showItemContextMenu);

	QHeaderView *header = _view->horizontalHeader();
	header->setSectionResizeMode(QHeaderView::Stretch);
}

void ChangeUDIDialog::onChangePkgUDIBtn()
{
	XNotifier::warn(QString("请扫描新的UDI!"));
	_step = 1;
}

void ChangeUDIDialog::onCheckInsBtn()
{
	XNotifier::warn(QString("请开始扫描包内器械!"));
	_step = 2;
}

void ChangeUDIDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package && 1 == _step)
	{
		PackageDao dao;
		Package pk;
		result_t resp = dao.getPackage(code, &pk);
		if (resp.isOk())
		{
			XNotifier::warn(QString("该器械包UDI:%1已经入库，无法替换!").arg(code));
		}
		else
		{
			_pkgCodeLabel->setText(code);
			_package_id_new = code;
		}

		_step = 0;
	}

	if (tc.type() == TranspondCode::Instrument && 2 == _step)
	{
		if (_insOldMap.contains(code))
		{
			_model->setData(_model->index(_insOldMap.value(code), 2), "已检测");
		}
	}

	if (tc.type() == TranspondCode::Instrument && 3 == _step)
	{
		InstrumentDao dao;
		Instrument it;
		result_t resp = dao.getInstrument(code, &it);
		if (resp.isOk())
		{
			int typeId = _model->data(_model->index(_row, 0), Qt::UserRole + 1).toInt();
			if (it.typeId == typeId)
			{
				if (it.packageUdi == NULL || it.packageUdi.isEmpty())
				{
					QString oldUDI = _model->data(_model->index(_row, 1), 2).toString();
					_model->setData(_model->index(_row, 3), code);
					_insNewMap.insert(oldUDI, code);
					_step = 0;
				}
				else
				{
					XNotifier::warn(QString("该器械包UDI:%1已经绑定，无法替换!").arg(code));
				}
			}
			else
			{
				XNotifier::warn(QString("该器械类型不匹配，无法替换!"));
			}
		}
		else
		{
			XNotifier::warn(QString("该器械UDI:%1未入库，无法替换!").arg(code));
		}
	}
}

void ChangeUDIDialog::showItemContextMenu(const QPoint& pos)
{
	QModelIndex index(_view->indexAt(pos));
	QMenu contextMenu;
	QAction *act = 0;
	if (index.isValid()) {
		act = contextMenu.addAction("替换UDI", this, SLOT(onChangeInsUDI()));
		act->setData(index.row());

		contextMenu.exec(QCursor::pos());
	}
	
}

void ChangeUDIDialog::onChangeInsUDI()
{
	if (QAction *action = qobject_cast<QAction*>(sender())) {
		_row = action->data().toInt();
		_step = 3;
		XNotifier::warn(QString("请开始扫描新的器械UDI!"));
	}
}

void ChangeUDIDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	Barcode bc(code);

	if (bc.type() == Barcode::Commit) {
		accept();
	}

	if (bc.type() == Barcode::Reset) {
		reset();
	}
	
}

void ChangeUDIDialog::onLoadPackage(const QString &code)
{
	PackageDao dao;
	Package pk;
	result_t resp = dao.getPackage(code, &pk, true);
	if (resp.isOk())
	{
		if (pk.is_del)
		{
			XNotifier::warn(QString("该包已被删除: ").append(code));
			return;
		}
		_pkgTypeLabel->setText(pk.typeName);
		_pkgCodeLabel->setText(pk.udi);
		_model->removeRows(0, _model->rowCount());
		_model->insertRows(0, pk.instruments.count());
		int i = 0;
		for each (auto &item in pk.instruments)
		{
			_model->setData(_model->index(i, 0), item.name);
			_model->setData(_model->index(i, 0), item.typeId, Qt::UserRole + 1);
			_model->setData(_model->index(i, 1), item.udi);
			_insOldMap.insert(item.udi, i);
			i++;
		}

		_package_id_old = code;
	}
	else
		XNotifier::warn(QString("添加包类型信息失败: ").append(resp.msg()));
}

void ChangeUDIDialog::onLoadPackageInfo(const QString &code)
{
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package)
	{
		onLoadPackage(code);
	}
	else
	{
		InstrumentDao dao;
		Instrument it;
		result_t resp = dao.getInstrument(code, &it);
		if (resp.isOk())
		{
			QString pkgUdi = it.packageUdi;

			onLoadPackage(pkgUdi);
		}
	}
}

void ChangeUDIDialog::accept() {
	if (_package_id_old.isEmpty())
		return QDialog::accept();

	QString pkgUDI = _package_id_old;

	if (!_package_id_new.isEmpty())
	{
		PackageDao dao;
		result_t resp = dao.changePackageUDI(_package_id_old, _package_id_new);
		if (!resp.isOk())
		{
			XNotifier::warn(QString("更改包UDI失败: ").append(resp.msg()));
			return QDialog::accept();
		}

		pkgUDI = _package_id_new;
	}

	if (!_insNewMap.isEmpty())
	{
		InstrumentDao dao;
		result_t resp = dao.changeInstrumentBound(pkgUDI, _insNewMap);

		if (!resp.isOk())
		{
			XNotifier::warn(QString("更改器械UDI失败: ").append(resp.msg()));
		}
	}

	return QDialog::accept();
}

void ChangeUDIDialog::reset()
{
	_model->removeRows(0, _model->rowCount());
	_pkgCodeLabel->clear();
	_pkgTypeLabel->clear();

	_package_id_old.clear();
	_package_id_new.clear();

	_step = 0;
	_row = -1;
	_insOldMap.clear();
	_insNewMap.clear();

	ScanTipsDialog d("请扫描UDI", this);
	if (d.exec() == QDialog::Accepted)
	{
		onLoadPackageInfo(d.getCode());
	}
}