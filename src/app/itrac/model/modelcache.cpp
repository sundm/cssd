#include "modelcache.h"
#include "core/net/url.h"

#include <QStandardItemModel>

ModelCache::ModelCache(QObject *parent)
	: QObject(parent) {
	_models.reserve(ModelCache::EOT);
	for (int i=0; i!=ModelCache::EOT; i++)
		_models.emplace_back(new QStandardItemModel); // models have no parent
}

void ModelCache::update() {
	updateDeptModel();
}

QAbstractItemModel * ModelCache::get(ModelCache::Type type) {
	return _models.at(type).get();
}

void ModelCache::updateDeptModel() {
	Url::post(Url::PATH_DEPT_SEARCH, "{}", [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			emit error(resp.errorString());
			return;
		}
		_Model & model = _models.at(ModelCache::Department);
		model->removeRows(0, model->rowCount()); // clear when succeeded

		QList<QVariant> deps = resp.getAsList("department_list");
		model->insertRows(0, deps.size());
		for (int i = 0; i != deps.size(); ++i) {
			QVariantMap map = deps[i].toMap();
			QStandardItem *depItem = new QStandardItem(map["department_name"].toString());

			depItem->setData(map["department_id"], ID_ROLE);
			depItem->setData(map["pinyin_code"], PY_ROLE);
			appendRow(depItem);
		}
	});
}
