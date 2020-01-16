#include "addrecalldialog.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "core/user.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "xnotifier.h"
#include "ui/views.h"
#include "barcode.h"
#include "rdao/dao/devicedao.h"
#include "rdao/dao/recalldao.h"
#include "rdao/entity/operator.h"
#include <QtWidgets/QtWidgets>

AddRecallDialog::AddRecallDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgUDIEdit(new Ui::FlatEdit)
	, _deviceBox(new QComboBox)
	, _cycleBox(new QSpinBox)
	, _reasonEdit(new QTextEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, PackageNum + 1, _view))
{
	QRadioButton *byPackage = new QRadioButton();
	byPackage->setChecked(true);
	_byPkg = true;
	QRadioButton *byDevice = new QRadioButton();
	byDevice->setChecked(false);
	QButtonGroup *bgGroup = new QButtonGroup(this);
	bgGroup->addButton(byPackage, 1);
	bgGroup->addButton(byDevice, 2);

	_pkgUDIEdit->setPlaceholderText("请扫描或输入包UDI");
	connect(_pkgUDIEdit, &QLineEdit::returnPressed, this, &AddRecallDialog::startTrace);
	QHBoxLayout *ahLayout = new QHBoxLayout();
	ahLayout->addWidget(byPackage);
	ahLayout->addWidget(_pkgUDIEdit);

	_deviceBox->setMinimumWidth(300);
	_deviceBox->setDisabled(true);
	_cycleBox->setDisabled(true);

	QLabel *deviceLabel = new QLabel("灭菌器：");
	QLabel *cycleLabel = new QLabel("锅次：");
	QHBoxLayout *bhLayout = new QHBoxLayout();
	bhLayout->addWidget(byDevice);
	bhLayout->addWidget(deviceLabel);
	bhLayout->addWidget(_deviceBox);
	bhLayout->addWidget(cycleLabel);
	bhLayout->addWidget(_cycleBox);
	bhLayout->addStretch(0);

	_model->setHeaderData(DeviceName, Qt::Horizontal, "灭菌器名称");
	_model->setHeaderData(BatchId, Qt::Horizontal, "灭菌批次号");
	_model->setHeaderData(Date, Qt::Horizontal, "灭菌时间");
	_model->setHeaderData(Cycle, Qt::Horizontal, "当日锅次");
	_model->setHeaderData(PackageNum, Qt::Horizontal, "涉及包数量");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 200);
	header->resizeSection(2, 150);

	Ui::PrimaryButton *recallButton = new Ui::PrimaryButton("添加召回", Ui::BtnSize::Small);
	connect(recallButton, SIGNAL(clicked()), this, SLOT(accept()));

	_reasonEdit->setPlaceholderText("请输入召回原因");

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(ahLayout);
	layout->addLayout(bhLayout);
	layout->addWidget(_reasonEdit);
	layout->addWidget(_view);
	layout->addWidget(recallButton);

	resize(750, 500);

	connect(bgGroup, SIGNAL(buttonToggled(int, bool)), this, SLOT(onToggled(int, bool)));

	connect(_cycleBox, SIGNAL(valueChanged(int)), this, SLOT(onCycleChanged(int)));
	connect(_deviceBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDeviceChanged(int)));

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(onRowDoubleClicked(const QModelIndex &)));

	QTimer::singleShot(200, [this] { initDevice(); });
}

void AddRecallDialog::accept()
{
	int deviceId = _sbi.deviceId;
	if (deviceId == 0) return;
	
	Operator op;
	op.id = Core::currentUser().id;
	op.name = Core::currentUser().name;

	int cycle = _sbi.cycleTotal;
	RecallDao dao;
	result_t resp = dao.addRecall(deviceId, cycle, op);
	if (resp.isOk())
	{
		XNotifier::warn(QString("召回成功!"));
		return QDialog::accept();
	}
	else
	{
		XNotifier::warn(QString("召回%1失败: %2").arg(_sbi.deviceName).arg(resp.msg()));
	}
}

void AddRecallDialog::startTrace()
{
	onTransponderReceviced(_pkgUDIEdit->text());
}

void AddRecallDialog::onDeviceChanged(int value)
{
	_devId = _deviceBox->itemData(value).toInt();
	if (_devId == 0) return;

	DeviceDao dao;
	Device dev;
	result_t resp = dao.getDevice(_devId, &dev);
	if (resp.isOk())
	{
		_cycleBox->setMaximum(dev.cycleTotal);
		_cycleBox->setValue(dev.cycleTotal);
	}
	else
	{
		XNotifier::warn(QString("无法获取设备: ").append(resp.msg()));
		return;
	}
}

void AddRecallDialog::onCycleChanged(int value)
{
	clear();

	RecallDao dao;
	
	result_t resp = dao.getSterilizeBatchInfo(_devId, value, &_sbi);
	if (resp.isOk())
	{
		_model->insertRows(0, 1);

		_model->setData(_model->index(0, DeviceName), _sbi.deviceName);
		_model->setData(_model->index(0, DeviceName), _sbi.deviceId, Qt::UserRole + 1);

		_model->setData(_model->index(0, BatchId), _sbi.batchId);
		_model->setData(_model->index(0, Date), _sbi.date.toString("yyyy-MM-dd HH:mm:ss"));

		_model->setData(_model->index(0, Cycle), _sbi.cycleCount);

		_model->setData(_model->index(0, PackageNum), _sbi.packageCount);

	}
}

void AddRecallDialog::initDevice()
{
	_deviceBox->clear();
	DeviceDao dao;
	QList<Sterilizer> sterilizers;
	result_t resp = dao.getSterilizerList(&sterilizers);
	if (resp.isOk())
	{
		_deviceBox->addItem(QString(""), QString(""));
		for (auto &device : sterilizers) {
			_deviceBox->addItem(device.name, device.id);
		}
		updateGeometry();
	}
	else
	{
		XNotifier::warn(QString("无法获取设备列表: ").append(resp.msg()));
		return;
	}
}

void AddRecallDialog::onRowDoubleClicked(const QModelIndex &)
{

}

void AddRecallDialog::onToggled(int itemId, bool isChecked)
{
	if (itemId == 1 && isChecked)
	{
		_pkgUDIEdit->setEnabled(isChecked);
		_deviceBox->setDisabled(isChecked);
		_cycleBox->setDisabled(isChecked);

		_byPkg = true;
	}

	if (itemId == 2 && isChecked)
	{
		_pkgUDIEdit->clear();
		_pkgUDIEdit->setDisabled(isChecked);
		_deviceBox->setEnabled(isChecked);
		_cycleBox->setEnabled(isChecked);

		_byPkg = false;
	}
}

void AddRecallDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	if (_byPkg)
	{
		TranspondCode tc(code);

		if (tc.type() == TranspondCode::Package)
		{
			clear();

			RecallDao dao;
			result_t resp = dao.getSterilizeBatchInfo(code, &_sbi);
			if (resp.isOk())
			{
				_model->insertRows(0, 1);

				_model->setData(_model->index(0, DeviceName), _sbi.deviceName);
				_model->setData(_model->index(0, DeviceName), _sbi.deviceId, Qt::UserRole + 1);

				_model->setData(_model->index(0, BatchId), _sbi.batchId);
				_model->setData(_model->index(0, Date), _sbi.date.toString("yyyy-MM-dd HH:mm:ss"));

				_model->setData(_model->index(0, Cycle), _sbi.cycleCount);

				_model->setData(_model->index(0, PackageNum), _sbi.packageCount);
			}
			
		}
	}
	
	
}

void AddRecallDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}

void AddRecallDialog::clear()
{
	_model->removeRows(0, _model->rowCount());
}