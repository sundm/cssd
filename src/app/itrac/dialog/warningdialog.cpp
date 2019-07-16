#include "warningdialog.h"
#include "ui/views.h"
#include "core/net/url.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QDate>

namespace {
	QColor color4Days(int days) {
		if (days <= 93) return QColor(255, 0, 0);
		if (days <= 95) return QColor(0, 255, 0);
		else return QColor(0, 0, 255);
	}
}

WarningDialog::WarningDialog(QWidget *parent)
	: QDialog(parent)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, DeptPhone+1, _view))
{
	setWindowTitle("物品包预警");
	resize(parent->width()/3*2, 600);

	_model->setHeaderData(PackageId, Qt::Horizontal, "物品包条码");
	_model->setHeaderData(PackageName, Qt::Horizontal, "物品包名称");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
	_model->setHeaderData(LeftDays, Qt::Horizontal, "剩余天数");
	_model->setHeaderData(DeptName, Qt::Horizontal, "所在科室");
	_model->setHeaderData(DeptPhone, Qt::Horizontal, "科室电话");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setSortingEnabled(true);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(_view);

	load();
}

WarningDialog::~WarningDialog() {
}

void WarningDialog::load() {
	_model->removeRows(0, _model->rowCount());

	QVariantMap vmap;
	vmap.insert("date", 100);

	post(url(PATH_WARN_PKGS), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> packages = resp.getAsList("packages");
		QDate today = QDate::currentDate();
		_model->insertRows(0, packages.count());
		for (int i = 0; i != packages.count(); ++i) {
			QVariantMap map = packages[i].toMap();
			QDate date = map["expired_date"].toDate();
			int days = today.daysTo(date);

			_model->setData(_model->index(i, PackageId), map["package_id"]);
			_model->setData(_model->index(i, PackageName), map["package_name"]);
			_model->setData(_model->index(i, PackType), map["pack_type"]);
			_model->setData(_model->index(i, ExpireDate), map["expired_date"]);
			_model->setData(_model->index(i, LeftDays), QString("%1天").arg(days));
			_model->setData(_model->index(i, LeftDays), QBrush(color4Days(days)), Qt::BackgroundRole);
			_model->setData(_model->index(i, DeptName), map["issue_department_name"]);
		}
	});
}
