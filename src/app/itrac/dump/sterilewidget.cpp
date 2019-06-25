#include <qdebug>
#include <QStandardItemModel>
#include <qscanner/qscanner.h>
#include <printer/Printer.h>

#include "sterilewidget.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include <qnetworkreply>
#include "operatorchooser.h"
#include "urls.h"
#include "inliner.h"
#include "xnotifier.h"

SterileWidget::SterileWidget(QWidget *parent)
	:QWidget(parent)
{
	setupUi(this);

	initPackageTableView();
	initDeviceTableView();
	initProgramTableView();

	chemistryButton->setChecked(true);

	connect(resetButton, SIGNAL(clicked()), this, SLOT(reset()));
	connect(sterileButton, SIGNAL(clicked()), this, SLOT(startSterile()));
	connect(deviceView->selectionModel(), &QItemSelectionModel::currentRowChanged,
		this, &SterileWidget::updateProgramView);

	connect(backButton, SIGNAL(clicked()), parent, SLOT(goBack()));
	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

SterileWidget::~SterileWidget()
{
}

void SterileWidget::barcodeScanned(const QString & bc)
{
	if (bc.startsWith("10")) { // new package
		updatePackageView(bc);
		return;
	}
}

void SterileWidget::initPackageTableView()
{
	_pkgModel = new QStandardItemModel(0, 3, pkgTableView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "包条码");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "打包类型");
	pkgTableView->setModel(_pkgModel);

	pkgTableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgTableView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 260);
}

void SterileWidget::initDeviceTableView()
{
	_deviceModel = new QStandardItemModel(0, 4, deviceView);
	_deviceModel->setHeaderData(0, Qt::Horizontal, "编号");
	_deviceModel->setHeaderData(1, Qt::Horizontal, "名称");
	_deviceModel->setHeaderData(2, Qt::Horizontal, "当前锅次");
	_deviceModel->setHeaderData(3, Qt::Horizontal, "类型");
	deviceView->setModel(_deviceModel);

	deviceView->setSelectionBehavior(QAbstractItemView::SelectRows);
	deviceView->setSelectionMode(QAbstractItemView::SingleSelection);
	deviceView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = deviceView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 200);
	header->resizeSection(1, 260);

	// update the content
	QByteArray data("{\"device_type\":\"0002\"}");
	JsonHttpClient().post(url(PATH_DEVICE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取灭菌设备列表: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> devices = resp.getAsList("devices");
		foreach(auto &device, devices) {
			QVariantMap map = device.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_id"].toString()));
			rowItems.append(new QStandardItem(map["device_name"].toString()));
			rowItems.append(new QStandardItem(map["is_forbidden"].toString()));
			rowItems.append(new QStandardItem(get_sterile_type(map["sterilize_type"].toInt())));
			_deviceModel->appendRow(rowItems);
		}
	});
}

void SterileWidget::initProgramTableView()
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
	header->resizeSection(0, 200);
	header->resizeSection(1, 260);
}

void SterileWidget::updatePackageView(const QString &pkgId)
{
	QList<QStandardItem *> items = _pkgModel->findItems(pkgId, Qt::MatchExactly, 0);
	if (0 != items.size())
		return;

	QString data = QString("{\"package_id\":\"%1\"}").arg(pkgId);
	JsonHttpClient().post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取包信息: ").append(resp.errorString()), -1);
			return;
		}

		QString state = resp.getAsString("state");
		if (state != "P") {
			XNotifier::warn(this, QString("包 [%1] 已进行过灭菌，请勿重复登记").arg(pkgId), -1);
			return;
		}
		QList<QStandardItem *> rowItems;
		rowItems.append(new QStandardItem(pkgId));
		rowItems.append(new QStandardItem(resp.getAsString("package_type_name")));
		rowItems.append(new QStandardItem(resp.getAsString("pack_type_name")));
		_pkgModel->appendRow(rowItems);
	});
}

void SterileWidget::clearPrograms()
{
	_programModel->removeRows(0, _programModel->rowCount());
}

//void SterileWidget::updatePkgView(QNetworkReply* reply)
//{
//	XHttpResponse resp(reply);
//	if (resp.success()) {
//		const QVariantMap *vmap = resp.json();
//		QList<QVariant> pkgTypes = (*vmap)["package_types"].toList();
//		foreach(auto &pkgType, pkgTypes) {
//			QVariantMap map = pkgType.toMap();
//			QStandardItem *nameItem = new QStandardItem(map["package_type_name"].toString());
//			nameItem->setData(map["package_type_id"]);
//			QStandardItem *numberItem = new QStandardItem(map["package_type_num"].toString());
//			QList<QStandardItem *> rowItems;
//			//rowItems.append(new QStandardItem(plateIdLabel->text()));
//			rowItems.append(nameItem);
//			rowItems.append(numberItem);
//			_pkgModel->appendRow(rowItems);
//		}
//	}
//	else {
//		XNotifier::warn(this, "无法获取清洗网篮数据", -1);
//	}
//}

void SterileWidget::updateProgramView(const QModelIndex &current, const QModelIndex &previous)
{
	QString deviceId = _deviceModel->item(current.row(), 0)->text();
	clearPrograms();

	QByteArray data("{\"device_id\":");
	data.append(deviceId).append('}');

	JsonHttpClient().post(url(PATH_PROGRAM_SUPPORT), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取包信息: ").append(resp.errorString()), -1);
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

void SterileWidget::reset()
{
	_pkgModel->removeRows(0, _pkgModel->rowCount());
	clearPrograms();
}

void SterileWidget::startSterile()
{
	QModelIndex deviceIndex = deviceView->currentIndex();
	QModelIndex programIndex = programView->currentIndex();
	if (!deviceIndex.isValid() || !programIndex.isValid()) {
		return;
	}

	int pkgNumber = _pkgModel->rowCount();
	if (0 == pkgNumber) {
		return;
	}
	OperatorChooser dialog;
	if (QDialog::Accepted == dialog.exec()) {
		QVariantMap vmap;
		vmap.insert("device_id", _deviceModel->data(deviceIndex.siblingAtColumn(0)).toInt());
		vmap.insert("program_id", _programModel->data(programIndex.siblingAtColumn(0)).toInt());
		vmap.insert("operator_id", dialog.getId());

		QVariantList vlist;
		for (int i = 0; i != _pkgModel->rowCount(); ++i) {
			vlist << _pkgModel->item(i, 0)->text();
		}
		vmap.insert("package_ids", vlist);

		bool chemicalTest = chemistryButton->isChecked() || compositeButton->isChecked();
		bool biologicalTest = compositeButton->isChecked();
		vmap.insert("chemical_test", chemicalTest);
		vmap.insert("biological_test", biologicalTest);

		JsonHttpClient().post(url(PATH_STERILE_ADD), vmap, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(this, QString("提交灭菌登记失败: ").append(resp.errorString()), -1);
				return;
			}

			LABELIST toPrintLabels;

			std::string deviceName = _deviceModel->data(deviceIndex.siblingAtColumn(1)).toString().toStdString();
			int currentCycle = _deviceModel->data(deviceIndex.siblingAtColumn(2)).toInt();
			std::string nextCycle = std::to_string(currentCycle + 1);

			list<std::string> label;
			label.push_back(deviceName);
			label.push_back(resp.getAsString("sterilize_time").toStdString());
			label.push_back(nextCycle);
			label.push_back(std::to_string(pkgNumber));
			label.push_back(resp.getAsString("test_id").toStdString());

			toPrintLabels.push_back(label);

			Printer *printer = PrinterFactory().Create(PrinterFactory::ZEBRA_GT8);
			printer->open();
			printer->printLabels("label/sterillzation", toPrintLabels);
			printer->close();
			delete printer;
		});
	}
	else {
		QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
	}
}
