#include "devicepage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "core/assets.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "dialog/devicedialog.h"

#include <QtWidgets/QtWidgets>

DevicePage::DevicePage(QWidget *parent)
	: QWidget(parent)
	, _filterComboBox(new QComboBox(this))
	, _deviceView(new TableView)
	, _deviceModel(new QStandardItemModel(0, 7, _deviceView))
{
	initDeviceView();

	_filterComboBox->addItem("全部", "");
	_filterComboBox->addItem("仅清洗设备", WASH_DEVICE);
	_filterComboBox->addItem("仅灭菌设备", STERILE_DEVICE);
	_filterComboBox->setCurrentIndex(0);
	connect(_filterComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DevicePage::onFilterChanged);

	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &DevicePage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &DevicePage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, &QToolButton::clicked, this, &DevicePage::modify);

	Ui::IconButton *forbiddenButton = new Ui::IconButton(":/res/forbidden-24.png", "删除");
	connect(forbiddenButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	QWidget *toolBar = new QWidget;
	//QHBoxLayout *hlayout = new QHBoxLayout(toolBar);
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(_filterComboBox);
	hlayout->addWidget(refreshButton);
	hlayout->addWidget(addButton);
	hlayout->addWidget(modifyButton);
	//hlayout->addWidget(forbiddenButton);
	hlayout->addStretch();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	//mainLayout->addWidget(toolBar);
	mainLayout->addLayout(hlayout);
	mainLayout->addWidget(_deviceView);
	
	updateDeviceView();
}

DevicePage::~DevicePage()
{
}

void DevicePage::onFilterChanged(int index)
{
	QString deviceType = _filterComboBox->itemData(index).toString();
	updateDeviceView(deviceType);
}

void DevicePage::initDeviceView()
{
	_deviceModel->setHeaderData(0, Qt::Horizontal, "编号");
	_deviceModel->setHeaderData(1, Qt::Horizontal, "名称");
	_deviceModel->setHeaderData(2, Qt::Horizontal, "总锅次");
	_deviceModel->setHeaderData(3, Qt::Horizontal, "今日锅次");
	_deviceModel->setHeaderData(4, Qt::Horizontal, "投产日期");
	_deviceModel->setHeaderData(5, Qt::Horizontal, "状态");
	_deviceModel->setHeaderData(6, Qt::Horizontal, "类型");
	_deviceView->setModel(_deviceModel);

	_deviceView->setSelectionMode(QAbstractItemView::SingleSelection);
	_deviceView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_deviceView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_deviceView, &QWidget::customContextMenuRequested,
		this, &DevicePage::showDeviceItemContextMenu);

	QHeaderView *header = _deviceView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 260);
	header->resizeSection(1, 300);
	header->resizeSection(4, 260);
}

void DevicePage::showDeviceItemContextMenu(const QPoint& pos)
{
	QModelIndex index(_deviceView->indexAt(pos));
	if (index.isValid()) {
		QString state = _deviceModel->data(index.siblingAtColumn(5), Qt::UserRole + 1).toString();
		if ("2" == state) // it's running
			return;

		QMenu contextMenu;
		QAction *act = 0;
		if ("1" == state) { // forbidden
			act = contextMenu.addAction("启用该设备", this, SLOT(unlockDevice()));
			act->setData(index.row());
		}
		else {
			act = contextMenu.addAction("禁用该设备", this, SLOT(lockDevice()));
			act->setData(index.row());
			act = contextMenu.addAction("修改...", this, SLOT(modifyDevice()));
			act->setData(index.row());
		}

		contextMenu.exec(QCursor::pos());
	}
}

void DevicePage::lockDevice()
{
	setDeviceEnabled(false);
}

void DevicePage::unlockDevice()
{
	setDeviceEnabled(true);
}

void DevicePage::modifyDevice()
{
	modify();
}

void DevicePage::updateDeviceView(const QString& deviceType/* = QString()*/)
{
	_deviceModel->removeRows(0, _deviceModel->rowCount());

	QByteArray data;
	if (deviceType.isEmpty())
		data.append("{}");
	else
		data.append("{\"device_type\":\"").append(deviceType).append("\"}");

	post(url(PATH_DEVICE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> devices = resp.getAsList("devices");
		for(auto &device: devices) {
			QVariantMap map = device.toMap();

			QStandardItem *idItem = new QStandardItem(map["device_id"].toString());
			idItem->setData(map["device_type"]);

			QString stateValue = map["is_forbidden"].toString();
			QStandardItem *stateItem = new QStandardItem(literal_device_state(stateValue));
			stateItem->setData(stateValue);

			QList<QStandardItem *> rowItems;
			rowItems.append(idItem);
			rowItems.append(new QStandardItem(map["device_name"].toString()));
			rowItems.append(new QStandardItem(map["total_cycles"].toString()));
			rowItems.append(new QStandardItem(map["cycle"].toString()));
			rowItems.append(new QStandardItem(map["production_date"].toString()));
			int st = map["sterilize_type"].toInt();
			QStandardItem *stItem = new QStandardItem(sterile_type(st));
			stItem->setData(st);
			rowItems.append(stItem);
			_deviceModel->appendRow(rowItems);
		}
	});
}

void DevicePage::setDeviceEnabled(bool enabled)
{
	if (QAction *action = qobject_cast<QAction*>(sender())) {
		int row = action->data().toInt();
		QString id(_deviceModel->data(_deviceModel->index(row, 0)).toString());

		QString req("{\"device_id\":%1,\"is_forbidden\":\"%2\"}");
		QByteArray data;
		data.append(req.arg(id, enabled ? "0" : "1"));

		post(url(PATH_DEVICE_MODIFY), data, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("更改设备状态失败: ").append(resp.errorString()));
				return;
			}

			QStandardItem *item = _deviceModel->item(row, 5);
			item->setText(literal_device_state(enabled ? "0" : "1"));
			item->setData(enabled ? "0" : "1");
		});
	}
}

void DevicePage::refresh() {
	QString deviceType = _filterComboBox->currentData().toString();
	updateDeviceView(deviceType);
}

void DevicePage::add() {
	AddDeviceDialog d(this);
	if (d.exec() == QDialog::Accepted) {
		QString deviceType = _filterComboBox->currentData().toString();
		updateDeviceView(deviceType);
	}
}

void DevicePage::modify() {
	QModelIndexList indexes = _deviceView->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	Device device;
	device.id = _deviceModel->data(_deviceModel->index(row, 0)).toInt();
	device.setTypeValue(_deviceModel->data(_deviceModel->index(row, 0), Qt::UserRole + 1).toString());
	device.name = _deviceModel->data(_deviceModel->index(row, 1)).toString();
	device.sterile_type = _deviceModel->data(_deviceModel->index(row, 5), Qt::UserRole + 1).toInt();

	ModifyDeviceDialog d(&device, this);
	if (d.exec() == QDialog::Accepted) {
		QString deviceType = _filterComboBox->currentData().toString();
		updateDeviceView(deviceType);
	}
}

void DevicePage::disable() {

}

