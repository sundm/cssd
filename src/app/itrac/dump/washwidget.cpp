#include <qdebug>
#include <QStandardItemModel>
#include <qscanner/qscanner.h>

#include "washwidget.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include <qnetworkreply>
#include "operatorchooser.h"
#include "controls/RegExpInputDialog.h"
#include "urls.h"
#include "xnotifier.h"

WashWidget::WashWidget(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);

	//backButton->setText(QChar(0xf053)); // fa-chevron-left

	initPackageTableView();
	initDeviceTableView();
	initProgramTableView();

	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(washButton, SIGNAL(clicked()), this, SLOT(startWash()));
	connect(bcInputButton, SIGNAL(clicked()), this, SLOT(inputBarcode()));
	connect(deviceView->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &WashWidget::updateProgramView);

	connect(backButton, SIGNAL(clicked()), parent, SLOT(goBack()));
	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

WashWidget::~WashWidget()
{
}

void WashWidget::barcodeScanned(const QString & bc)
{
	if (bc.startsWith("16") && bc != plateIdLabel->text()) { // new plate
		showPlateDetails(bc);
		return;
	}
}

void WashWidget::initPackageTableView()
{
	_pkgModel = new QStandardItemModel(0, 3, pkgTableView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "网篮");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "数量");
	pkgTableView->setModel(_pkgModel);

	pkgTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgTableView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);
}

void WashWidget::initDeviceTableView()
{
	_deviceModel = new QStandardItemModel(0, 4, deviceView);
	_deviceModel->setHeaderData(0, Qt::Horizontal, "编号");
	_deviceModel->setHeaderData(1, Qt::Horizontal, "名称");
	_deviceModel->setHeaderData(2, Qt::Horizontal, "当前锅次");
	_deviceModel->setHeaderData(3, Qt::Horizontal, "条码");
	deviceView->setModel(_deviceModel);

	deviceView->setSelectionBehavior(QAbstractItemView::SelectRows);
	deviceView->setSelectionMode(QAbstractItemView::SingleSelection);
	deviceView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = deviceView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);

	// update content
	QByteArray data("{\"device_type\":\"0001\"}");
	JsonHttpClient().post(url(PATH_DEVICE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取清洗设备列表: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> devices = resp.getAsList("devices");
		foreach(auto &device, devices) {
			QVariantMap map = device.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_id"].toString()));
			rowItems.append(new QStandardItem(map["device_name"].toString()));
			rowItems.append(new QStandardItem(map["is_forbidden"].toString()));
			_deviceModel->appendRow(rowItems);
		}
	});
}

void WashWidget::initProgramTableView()
{
	_programModel = new QStandardItemModel(0, 3, programView);
	_programModel->setHeaderData(0, Qt::Horizontal, "编号");
	_programModel->setHeaderData(1, Qt::Horizontal, "清洗程序");
	_programModel->setHeaderData(2, Qt::Horizontal, "条码");
	programView->setModel(_programModel);

	programView->setSelectionBehavior(QAbstractItemView::SelectRows);
	programView->setSelectionMode(QAbstractItemView::SingleSelection);
	programView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = programView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);
}

void WashWidget::showPlateDetails(const QString &bc)
{
	plateIdLabel->setText(bc);

	QByteArray data("{\"plate_id\":");
	data.append(bc).append('}');

	JsonHttpClient().post(url(PATH_PKG_IN_PLATE), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取清洗网篮数据: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> pkgTypes = resp.getAsList("package_types");
		foreach(auto &pkgType, pkgTypes) {
			QVariantMap map = pkgType.toMap();
			QStandardItem *nameItem = new QStandardItem(map["package_type_name"].toString());
			nameItem->setData(map["package_type_id"]);
			QStandardItem *numberItem = new QStandardItem(map["package_type_num"].toString());
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(plateIdLabel->text()));
			rowItems.append(nameItem);
			rowItems.append(numberItem);
			_pkgModel->appendRow(rowItems);
		}
	});
}

void WashWidget::clearPrograms()
{
	_programModel->removeRows(0, _programModel->rowCount());
}

void WashWidget::updateProgramView(const QModelIndex &current, const QModelIndex &previous)
{
	clearPrograms();
	QString deviceId = _deviceModel->item(current.row(), 0)->text();

	QByteArray data("{\"device_id\":");
	data.append(deviceId).append('}');

	JsonHttpClient().post(url(PATH_PROGRAM_SUPPORT), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取清洗程序列表: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		foreach(auto &program, programs) {
			QVariantMap map = program.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_program_id"].toString()));
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["program_desc"].toString()));
			_programModel->appendRow(rowItems);
		}
	});
}

void WashWidget::inputBarcode()
{
	bool ok;
	QRegExp regExp("\\d{10,}");
	QString text = RegExpInputDialog::getText(this, "手工输入条码", "请输入网篮的条码", "", regExp, &ok);
	if (ok) {
		barcodeScanned(text);
	}
}

void WashWidget::reset()
{
	_pkgModel->removeRows(0, _pkgModel->rowCount());
	clearPrograms();
}

void WashWidget::startWash()
{
	QModelIndex deviceIndex = deviceView->currentIndex();
	QModelIndex programIndex = programView->currentIndex();
	if (!deviceIndex.isValid() || !programIndex.isValid()) {
		return;
	}
	if (0 == _pkgModel->rowCount()) {
		return;
	}
	OperatorChooser dialog;
	if (QDialog::Accepted == dialog.exec()) {
		QVariantMap vmap;
		vmap.insert("plate_id", plateIdLabel->text().toInt());
		vmap.insert("device_id", _deviceModel->data(deviceIndex.siblingAtColumn(0)).toInt());
		vmap.insert("program_id", _programModel->data(programIndex.siblingAtColumn(0)).toInt());
		vmap.insert("operator_id", dialog.getId());

		JsonHttpClient().post(url(PATH_WASH_ADD), vmap, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success())
				XNotifier::warn(this, QString("无法完成清洗登记: ").append(resp.errorString()), -1);
			else
				XNotifier::warn(this, "已完成清洗登记", -1);
		});
	}
	else {
		QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
	}
}
