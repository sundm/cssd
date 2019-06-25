#include "sterilecheckwidget.h"
#include <QStandardItemModel>
#include <qscanner/qscanner.h>
#include <qnetworkreply>
#include <printer/Printer.h>
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "operatorchooser.h"
#include "urls.h"
#include "Barcode.h"
#include "xnotifier.h"

SterileCheckWidget::SterileCheckWidget(QWidget *parent)
	: QWidget(parent), _chemInvolved(false), _bioInvolved(false)
{
	setupUi(this);
	//backButton->setText(QChar(0xf053)); // fa-chevron-left

	initPackageView();

	connect(okButton, &QAbstractButton::clicked, this, &SterileCheckWidget::commitSterileQualified);
	connect(notOkButton, &QAbstractButton::clicked, this, &SterileCheckWidget::commitSterileUnqualified);

	connect(backButton, SIGNAL(clicked()), parent, SLOT(goBack()));
	QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
}

SterileCheckWidget::~SterileCheckWidget()
{
}

void SterileCheckWidget::barcodeScanned(const QString & bc)
{
	Barcode barcode(bc);
	if (Barcode::Test == barcode.type())
		updateSterileInfo(bc);
}

void SterileCheckWidget::commitSterileQualified()
{
	OperatorChooser dialog;
	if (QDialog::Accepted == dialog.exec()) {
		QVariantMap vmap;
		vmap.insert("test_id", bcEdit->text());
		vmap.insert("test_operator_id", dialog.getId());
		vmap.insert("phy_test_result", "1");
		if (_chemInvolved)
			vmap.insert("che_test_result", "1");
		if (_bioInvolved)
			vmap.insert("bio_test_result", "1");

		JsonHttpClient().post(url(PATH_STERILE_CHECK), vmap, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success())
				XNotifier::warn(this, QString("提交灭菌合格登记失败: ").append(resp.errorString()), -1);
			else
				XNotifier::warn(this, "已完成灭菌合格登记", -1);
		});
	}
	else {
		QScannerFactory::connect(this, SLOT(barcodeScanned(const QString &)));
	}
}

void SterileCheckWidget::commitSterileUnqualified()
{

}

void SterileCheckWidget::updateSterileInfo(const QString & testId)
{
	bcEdit->setText(testId);

	QByteArray data("{\"test_id\":");
	data.append(testId).append('}');
	JsonHttpClient().post(url(PATH_STERILE_INFO), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取灭菌监测批次 [%1] 的数据: %2").arg(testId, resp.errorString()), -1);
			return;
		}

		_chemInvolved = "1" == resp.getAsString("chemical_test");
		_bioInvolved = "1" == resp.getAsString("biological_test");
		if (_chemInvolved)
			typeEdit->setText(_bioInvolved ? "化学监测+生物监测" : "仅化学监测");
		else
			typeEdit->setText("无");

		deviceEdit->setText(resp.getAsString("ste_device_name"));
		cycleEdit->setText(resp.getAsString("ste_cycle"));
		startTimeEdit->setText(resp.getAsString("ste_start_time"));

		QString result = resp.getAsString("result");
		if (result == "2")
			resultEdit->setText("待审核");
		else
			resultEdit->setText(QString("%1(已审核)").arg(result == "1" ? "合格" : "不合格"));

		// list packages
		QList<QVariant> packages = resp.getAsList("packages");
		foreach(auto &package, packages) {
			QVariantMap map = package.toMap();
			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["package_id"].toString()));
			rowItems.append(new QStandardItem(map["package_name"].toString()));
			_pkgModel->appendRow(rowItems);
		}
	});
}

void SterileCheckWidget::initPackageView()
{
	_pkgModel = new QStandardItemModel(0, 2, pkgView);
	_pkgModel->setHeaderData(0, Qt::Horizontal, "包编号");
	_pkgModel->setHeaderData(1, Qt::Horizontal, "包类型");
	pkgView->setModel(_pkgModel);

	pkgView->setSelectionBehavior(QAbstractItemView::SelectRows);
	pkgView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = pkgView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 300);
}
