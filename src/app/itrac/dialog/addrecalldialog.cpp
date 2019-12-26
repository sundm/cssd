#include "addrecalldialog.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "xnotifier.h"
#include "ui/views.h"
#include "barcode.h"
#include "rdao/dao/devicedao.h"
#include <QtWidgets/QtWidgets>

AddRecallDialog::AddRecallDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgUDIEdit(new Ui::FlatEdit)
	, _deviceBox(new QComboBox)
	, _cycleBox(new QSpinBox)
	, _reasonEdit(new QTextEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, Dept + 1, _view))
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

	_model->setHeaderData(UDI, Qt::Horizontal, "UDI");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(Dept, Qt::Horizontal, "所属科室");
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

	QTimer::singleShot(200, [this] { initDevice(); });
}

void AddRecallDialog::accept()
{
	RecallInfo info;
	info.deviceName = _deviceBox->currentText();
	info.deviceId = _deviceBox->currentData().toInt();
	info.pkgNums = _model->rowCount();
}

void AddRecallDialog::startTrace()
{
	onTransponderReceviced(_pkgUDIEdit->text());
}

void AddRecallDialog::onDeviceChanged(int value)
{
	int devId = _deviceBox->itemData(value).toInt();
	if (devId == 0) return;

	DeviceDao dao;
	Device dev;
	result_t resp = dao.getDevice(devId, &dev);
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

		}
	}
	
	
}

void AddRecallDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
}