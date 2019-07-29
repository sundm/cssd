#include "recallpage.h"
#include "ui/views.h"
#include "ui/buttons.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <QComboBox>
#include <QStandardItemModel>
#include <QHBoxLayout>
#include <QVBoxLayout>

RecallPage::RecallPage(QWidget *parent)
	: QWidget(parent)
	, _comboBox(new QComboBox(this))
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, ExpireDate + 1, _view))
{
	loadSterilizer();

	_model->setHeaderData(PackageId, Qt::Horizontal, "物品包条码");
	_model->setHeaderData(PackageName, Qt::Horizontal, "物品包名称");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	//Ui::PrimaryButton *recallButton = new Ui::PrimaryButton("召回", Ui::BtnSize::Small);
	QPushButton *recallButton = new QPushButton("召回", this);
	connect(recallButton, SIGNAL(clicked()), this, SLOT(recall()));
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(_comboBox);
	hlayout->addWidget(recallButton);
	hlayout->addStretch();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(hlayout);
	mainLayout->addWidget(_view);

	connect(_comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onDeviceChanged(int)));
}

RecallPage::~RecallPage()
{
}

void RecallPage::onDeviceChanged(int index)
{
	if (index == -1) return;

	QVariantMap v;
	v.insert("device_id", _comboBox->itemData(index));
	post(url(PATH_RECALL_SEARCH), v, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取待召回物品包列表: ").append(resp.errorString()));
			return;
		}
		QList<QVariant> packages = resp.getAsList("packages");
		_model->insertRows(0, packages.count());
		for (int i = 0; i != packages.count(); ++i) {
			QVariantMap map = packages[i].toMap();
			_model->setData(_model->index(i, PackageId), map["package_id"]);
			_model->setData(_model->index(i, PackageName), map["package_name"]);
			_model->setData(_model->index(i, PackType), map["pack_type_name"]);
			_model->setData(_model->index(i, ExpireDate), map["expired_date"]);
		}
	});
}

void RecallPage::recall() {
	QVariantMap v;
	v.insert("device_id", _comboBox->currentData());
	post(url(PATH_RECALL), v, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法召回: ").append(resp.errorString()));
			return;
		}
		else {
			_view->clear();
			XNotifier::warn("已召回上次生物灭菌成功以来所有尚未使用的包 ");
		}
	});
}

void RecallPage::loadSterilizer() {
	QByteArray data;
	data.append("{\"device_type\":\"0002\"}");
	post(url(PATH_DEVICE_SEARCH), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备列表: ").append(resp.errorString()));
			return;
		}
		QList<QVariant> devices = resp.getAsList("devices");
		for (auto &device : devices) {
			QVariantMap map = device.toMap();
			_comboBox->addItem(map["device_name"].toString(), map["device_id"]);
		}
		_comboBox->setCurrentIndex(-1);
	});
}
