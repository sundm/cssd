#include <QStandardItemModel>
#include <qscanner/qscanner.h>
#include <qnetworkreply>
#include <printer/Printer.h>
#include "washcheckwidget.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "operatorchooser.h"
#include "urls.h"
#include "xnotifier.h"

WashCheckWidget::WashCheckWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);
	//backButton->setText(QChar(0xf053)); // fa-chevron-left

	initPackageView();

	connect(okButton, &QAbstractButton::clicked, this, &WashCheckWidget::commitWashQualified);
	connect(notOkButton, &QAbstractButton::clicked, this, &WashCheckWidget::commitWashUnqualified);
	connect(packButton, &QAbstractButton::clicked, this, &WashCheckWidget::doPack);

	connect(backButton, SIGNAL(clicked()), parent, SLOT(goBack()));
	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

WashCheckWidget::~WashCheckWidget()
{
}

void WashCheckWidget::barcodeScanned(const QString &bc)
{
	if (bc.startsWith("16") && bc != plateIdLabel->text()) { // new plate
		showPlateDetails(bc);
		return;
	}
}

void WashCheckWidget::commitWashQualified()
{
	//"plate_id":123456,
	//	"check_operator_id" : 123456,
	//	"check_result" : 1
	OperatorChooser dialog;
	if (QDialog::Accepted == dialog.exec()) {
		QVariantMap vmap;
		vmap.insert("plate_id", plateIdLabel->text().toInt());
		vmap.insert("check_operator_id", dialog.getId());
		vmap.insert("check_result", 1);

		JsonHttpClient().post(url(PATH_WASH_CHECK), vmap, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(this, QString("清洗合格登记失败: ").append(resp.errorString()), -1);
				return;
			}
			else {
				XNotifier::warn(this, "已完成清洗合格登记", -1);
			}
		});
	}
	else {
		QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
	}
}

void WashCheckWidget::commitWashUnqualified()
{

}

void WashCheckWidget::doPack()
{
	if (0 == _pkgModel->rowCount())
		return;

	OperatorChooser dialog;
	if (QDialog::Accepted != dialog.exec()) {
		QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
		return;
	}

	QVariantList pkgList;
	for (int i = 0; i != _pkgModel->rowCount(); i++) {
		QVariantMap packageType;
		packageType.insert("package_type_id", _pkgModel->item(i, 1)->data(Qt::UserRole + 1));
		packageType.insert("pack_type_id", _pkgModel->item(i, 3)->data(Qt::UserRole + 1));
		pkgList << packageType;
	}

	QVariantMap vmap;
	vmap.insert("plate_id", plateIdLabel->text().toInt());
	vmap.insert("operator_id", dialog.getId());
	vmap.insert("check_operator_id", 11000001);
	vmap.insert("packages", pkgList);

	JsonHttpClient().post(url(PATH_PACK_ADD), vmap, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("配包登记失败: ").append(resp.errorString()), -1);
			return;
		}

		QVariantList pkgList = resp.getAsList("pack_info");

		// print
		LABELIST toPrintLabels;
		std::string operatorName = resp.getAsString("pack_operator_name").toStdString();
		std::string assessorName = resp.getAsString("check_operator_name").toStdString();

		foreach(auto &pkg, pkgList) {
			QVariantMap map = pkg.toMap();

			list<std::string> label;
			label.push_back(map["package_id"].toString().toStdString());
			label.push_back(map["package_name"].toString().toStdString()); // package name
			label.push_back(map["instrument_num"].toString().toStdString()); // number of instrument
			label.push_back(map["department_name"].toString().toStdString());
			label.push_back(map["pack_type_name"].toString().toStdString());
			label.push_back(map["expire_date"].toString().toStdString());
			label.push_back(map["sterilize_date"].toString().toStdString());
			label.push_back(operatorName);
			label.push_back(assessorName);

			toPrintLabels.push_back(label);
		}

		Printer *printer = PrinterFactory().Create(PrinterFactory::ZEBRA_GT8);
		if (0 != printer->open()) {
			XNotifier::warn(this, "启动标签打印机失败，稍后可到历史查询页中重打标签", -1);
			return;
		}
		printer->printLabels("label/packageLabel", toPrintLabels);
		printer->close();
		delete printer;
	});
}

void WashCheckWidget::initPackageView()
{
	_pkgModel = new QStandardItemModel(0, 4, pkgView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "网篮");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	_pkgModel->setHeaderData(2, Qt::Horizontal, "数量");
	_pkgModel->setHeaderData(3, Qt::Horizontal, "打包方式");
	pkgView->setModel(_pkgModel);

	pkgView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
	header->resizeSection(1, 400);
}

void WashCheckWidget::showPlateDetails(const QString &bc)
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
			QStandardItem *packTypeNameItem = new QStandardItem(map["pack_type_name"].toString());
			packTypeNameItem->setData(map["pack_type_id"]);
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(plateIdLabel->text()));
			rowItems.append(nameItem);
			rowItems.append(new QStandardItem(map["package_type_num"].toString()));
			rowItems.append(packTypeNameItem);
			_pkgModel->appendRow(rowItems);
		}
	});
}
