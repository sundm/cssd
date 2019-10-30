#include "devicedialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/inliner.h"
#include "core/net/url.h"
#include "core/assets.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/views.h"
#include "ui/composite/waitingspinner.h"
#include <QtWidgets/QtWidgets>

AddDeviceDialog::AddDeviceDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _typeCombo(new QComboBox)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 3, _view))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新设备");

	_typeCombo->addItem("清洗机", -1);
	_typeCombo->addItem("通用灭菌器", 0);
	_typeCombo->addItem("高温灭菌器", 1);
	_typeCombo->addItem("低温灭菌器", 2);
	_typeCombo->setCurrentIndex(-1);
	connect(_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &AddDeviceDialog::loadPrograms);

	initProgramView();

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddDeviceDialog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("设备类型"), 0, 0);
	mainLayout->addWidget(new QLabel("设备名称"), 1, 0);
	mainLayout->addWidget(new QLabel("预设程序"), 2, 0);
	mainLayout->addWidget(_typeCombo, 0, 1);
	mainLayout->addWidget(_nameEdit, 1, 1);
	mainLayout->addWidget(_view, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void AddDeviceDialog::accept() {
	if (_typeCombo->currentIndex() == -1) {
		_typeCombo->showPopup();
		return;
	}

	QString name = _nameEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}

	QVariantList programs;
	for (int i = 0; i != _model->rowCount(); i++) {
		QStandardItem *item = _model->item(i);
		if (item->checkState() == Qt::Checked) programs.append(item->text());
	}
	QVariantMap vmap;
	int type = _typeCombo->currentData().toInt();
	QString device_type = QString("0002");
	switch (type)
	{
	case -1:
		device_type = QString("0001");
	default:
		break;
	}
	vmap.insert("device_type", device_type);
	vmap.insert("device_name", name);
	vmap.insert("production_date", QDate::currentDate());
	vmap.insert("support_program_ids", programs);
	vmap.insert("sterilize_type", type);

	_waiter->start();
	post(url(PATH_DEVICE_ADD), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("添加设备失败: ").append(resp.errorString()));
			return;
		}
		else {
			QDialog::accept();
		}
	});
}

void AddDeviceDialog::toggleCheckSate(const QModelIndex &index) {
	QStandardItem *item = _model->item(index.row());
	item->setCheckState(item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
}

void AddDeviceDialog::initProgramView() {
	_model->setHeaderData(0, Qt::Horizontal, "编号");
	_model->setHeaderData(1, Qt::Horizontal, "名称");
	_model->setHeaderData(2, Qt::Horizontal, "描述");
	_view->setModel(_model);

	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 200);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(toggleCheckSate(const QModelIndex &)));
}

void AddDeviceDialog::loadPrograms(int index) {
	_model->removeRows(0, _model->rowCount());

	QString deviceType = QString("0002");
	int type = _typeCombo->itemData(index).toInt();
	if (-1 == type)
		deviceType = QString("0001");

	QByteArray data;
	data.append("{\"program_type\":\"").append(deviceType).append("\"}");

	post(url(PATH_PROGRAM_SEARCH), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for (auto &program : programs) {
			QVariantMap map = program.toMap();
			QList<QStandardItem *> rowItems;
			QStandardItem *checkItem = new QStandardItem(map["device_program_id"].toString());
			checkItem->setCheckable(true);
			rowItems.append(checkItem);
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["program_desc"].toString()));
			_model->appendRow(rowItems);
		}
	});
}


ModifyDeviceDialog::ModifyDeviceDialog(Device *device, QWidget *parent /*= Q_NULLPTR*/)
	: QDialog(parent)
	, _device(device) 
	, _nameEdit(new Ui::FlatEdit)
	, _typeEdit(new Ui::FlatEdit)
	, _typeCombo(new QComboBox)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 3, _view))
{
	setWindowTitle("修改设备属性");

	_typeEdit->setText(Device::Washer == device->type ? "清洗设备" : "灭菌设备");
	_typeEdit->setReadOnly(true);
	_nameEdit->setText(device->name);
	_nameEdit->setReadOnly(true);

	_typeCombo->addItem("通用灭菌器", 0);
	_typeCombo->addItem("高温灭菌器", 1);
	_typeCombo->addItem("低温灭菌器", 2);
	_typeCombo->setCurrentIndex(device->sterile_type);
	type = device->sterile_type;

	initProgramView();
	loadPrograms();

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("设备类型"), 0, 0);
	mainLayout->addWidget(new QLabel("设备名称"), 1, 0);
	mainLayout->addWidget(_typeEdit, 0, 1);
	mainLayout->addWidget(_nameEdit, 1, 1);

	if (device->type > 0)
	{
		mainLayout->addWidget(new QLabel("灭菌类型"), 2, 0);
		mainLayout->addWidget(new QLabel("预设程序"), 3, 0);
		mainLayout->addWidget(_typeCombo, 2, 1);
		mainLayout->addWidget(_view, 3, 1);
		mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 4, 0, 1, 2);
		mainLayout->addWidget(submitButton, 5, 0, 1, 2, Qt::AlignHCenter);
	}
	else
	{
		mainLayout->addWidget(new QLabel("预设程序"), 2, 0);
		mainLayout->addWidget(_view, 2, 1);
		mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
		mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);
	}
	
	

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void ModifyDeviceDialog::accept() {
	QString name = _nameEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}

	QVariantList programs;
	for (int i = 0; i != _model->rowCount(); i++) {
		QStandardItem *item = _model->item(i);
		if (item->checkState() == Qt::Checked) programs.append(item->text());
	}

	if (type != -1)
		type = _typeCombo->currentData().toInt();

	QVariantMap vmap;
	vmap.insert("device_id", _device->id);
	vmap.insert("device_name", name);
	vmap.insert("support_program_ids", programs);
	vmap.insert("sterilize_type", type);

	//_waiter->start();
	post(url(PATH_DEVICE_MODIFY), vmap, [this](QNetworkReply *reply) {
		//_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("更新设备信息失败: ").append(resp.errorString()));
			return;
		}
		else {
			QDialog::accept();
		}
	});
}

void ModifyDeviceDialog::toggleCheckSate(const QModelIndex& index) {
	QStandardItem *item = _model->item(index.row());
	item->setCheckState(item->checkState() == Qt::Unchecked ? Qt::Checked : Qt::Unchecked);
}

void ModifyDeviceDialog::initProgramView() {
	_model->setHeaderData(0, Qt::Horizontal, "编号");
	_model->setHeaderData(1, Qt::Horizontal, "名称");
	_model->setHeaderData(2, Qt::Horizontal, "描述");
	_view->setModel(_model);

	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 100);
	header->resizeSection(1, 150);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(toggleCheckSate(const QModelIndex &)));
}

void ModifyDeviceDialog::loadPrograms()
{
	QString deviceType = _device->typeValue();
	QByteArray data;
	data.append("{\"program_type\":\"").append(deviceType).append("\"}");

	post(url(PATH_PROGRAM_SEARCH), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for (auto &program : programs) {
			QVariantMap map = program.toMap();
			QList<QStandardItem *> rowItems;
			QStandardItem *checkItem = new QStandardItem(map["device_program_id"].toString());
			checkItem->setCheckable(true);
			rowItems.append(checkItem);
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["program_desc"].toString()));
			_model->appendRow(rowItems);
		}

		selectPrograms();
	});
}

void ModifyDeviceDialog::selectPrograms() {
	QByteArray data("{\"device_id\":");
	data.append(QString::number(_device->id)).append('}');
	post(url(PATH_PROGRAM_SUPPORT), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(this, QString("无法获取清洗程序列表: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for (auto &program : programs) {
			QVariantMap map = program.toMap();
			QString id = map["device_program_id"].toString();
			for (int i = 0; i != _model->rowCount(); ++i) {
				QStandardItem *item = _model->item(i);
				if (item->text() == id) {
					item->setCheckState(Qt::Checked);
					break;
				}
			}
		}
	});
}
