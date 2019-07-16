#include "packageview.h"
#include "core/itracnamespace.h"
#include "core/assets.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include <QStandardItemModel>

PackageView::PackageView(QWidget *parent /*= nullptr*/)
	: TableView(parent), _model(new QStandardItemModel(this))
{
	setModel(_model);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	_model->setColumnCount(ExpireDate + 1);
	_model->setHeaderData(Barcode, Qt::Horizontal, "包条码");
	_model->setHeaderData(Name, Qt::Horizontal, "包名");
	_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
	_model->setHeaderData(Department, Qt::Horizontal, "目标科室");
	_model->setHeaderData(ExpireDate, Qt::Horizontal, "失效日期");
}

bool PackageView::hasPackage(const QString &id) const
{
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::DisplayRole, id, 1);
	return !matches.isEmpty();
}

QVariantList PackageView::packages() const {
	QVariantList list;
	for (int i = 0; i != _model->rowCount(); i++) {
		//QString packageId = _model->data(_model->index(i, 0)).toString();
		list.push_back(_model->data(_model->index(i, 0)));
	}
	return list;
}

void PackageView::addPackage(const QString &id) {
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	_http.post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		QString state = resp.getAsString("state");
		if (state == "X") {
			XNotifier::warn("该包已被召回，严禁使用");
			return;
		}
		if (state != "I") {
			XNotifier::warn("该包处于不可使用状态，请检查包的来源");
			return;
		}

		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("from_department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));
		_model->appendRow(rowItems);
	});
}
