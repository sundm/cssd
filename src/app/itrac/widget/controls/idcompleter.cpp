#include "idcompleter.h"
#include "core/net/url.h"
#include "core/constants.h"
#include <QStandardItemModel>
#include "rdao/dao/InstrumentDao.h"
#include "rdao/dao/PackageDao.h"
#include "rdao/dao/deptdao.h"

IdCompleter::IdCompleter(QObject *parent)
	: QCompleter(parent)
	, _model(new QStandardItemModel(this))
{
	setCaseSensitivity(Qt::CaseInsensitive);
	setCompletionRole(Constant::PinyinRole);
	setMaxVisibleItems(50); // the default is 7
	setModel(_model);
}

int IdCompleter::id(const QModelIndex& index) const {
	return _model->data(index, Constant::IdRole).toInt();
}

QString IdCompleter::name(const QModelIndex& index) const {
	return _model->data(index).toString();
}

DeptCompleter::DeptCompleter(QObject *parent /*= nullptr*/)
	: IdCompleter(parent) {
}

void DeptCompleter::load(const int deptTypeId) {
	DeptDao dao;
	QList<Department> depts;
	result_t resp = dao.getDeptList(&depts);
	if (resp.isOk())
	{
		_model->clear(); // when succeeded
		for (auto &dep : depts) {
			if (deptTypeId == 0)
			{
				QStandardItem *depItem = new QStandardItem(dep.name);
				depItem->setData(dep.id, Constant::IdRole);
				depItem->setData(dep.pinyin, Constant::PinyinRole);
				_model->appendRow(depItem);
			}
			if (deptTypeId == 3 && dep.isSurgical)
			{
				QStandardItem *depItem = new QStandardItem(dep.name);
				depItem->setData(dep.id, Constant::IdRole);
				depItem->setData(dep.pinyin, Constant::PinyinRole);
				_model->appendRow(depItem);
			}
		}
	}
	else
	{
		emit error(resp.msg());
		return;
	}
	//_http.post(url(PATH_DEPT_SEARCH), "{}", [=](QNetworkReply *reply) {
	//	JsonHttpResponse resp(reply);
	//	if (!resp.success()) {
	//		emit error(resp.errorString());
	//		return;
	//	}

	//	_model->clear(); // when succeeded

	//	QList<QVariant> deps = resp.getAsList("department_list");
	//	for (auto &dep : deps) {
	//		QVariantMap map = dep.toMap();
	//		if (deptTypeId == 0 || deptTypeId == map["is_or"].toInt())
	//		{
	//			QStandardItem *depItem = new QStandardItem(map["department_name"].toString());
	//			depItem->setData(map["department_id"], Constant::IdRole);
	//			depItem->setData(map["pinyin_code"], Constant::PinyinRole);
	//			_model->appendRow(depItem);
	//		}
	//		
	//	}
	//});
}

PackageCompleter::PackageCompleter(QObject *parent /*= nullptr*/)
	: IdCompleter(parent){
}

void PackageCompleter::loadForDepartment(int deptId) {
	QByteArray data("{\"department_id\":");
	data.append(QString::number(deptId)).append('}');
	loadInternal(data);
}

void PackageCompleter::loadForCategory(const QString &category) {
	QByteArray data("{\"package_category\":\"");
	data.append(category).append("\"}");
	loadInternal(data);
}

void PackageCompleter::loadInternal(const QByteArray &data) {
	PackageDao dao;
	QList<PackageType> pts;

	result_t resp = dao.getPackageTypeList(&pts);
	if (resp.isOk())
	{
		_model->clear();
		for (auto &pt : pts) {
			QStandardItem *pkgItem = new QStandardItem(pt.name);
			pkgItem->setData(pt.typeId, Constant::IdRole);
			pkgItem->setData(pt.pinyin, Constant::PinyinRole);
			_model->appendRow(pkgItem);
		}
	}
	else
	{
		emit error(resp.msg());
		return;
	}
	/*_http.post(url(PATH_PKGTPYE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			emit error(resp.errorString());
			return;
		}

		_model->clear();

		QList<QVariant> pkgs = resp.getAsList("package_types");
		for (auto &pkg : pkgs) {
			QVariantMap map = pkg.toMap();
			QStandardItem *pkgItem = new QStandardItem(map["package_name"].toString());
			pkgItem->setData(map["package_type_id"], Constant::IdRole);
			pkgItem->setData(map["pinyin_code"], Constant::PinyinRole);
			_model->appendRow(pkgItem);
		}
	});*/
}

InstrumentCompleter::InstrumentCompleter(QObject *parent /*= nullptr*/)
	: IdCompleter(parent) {
}

void InstrumentCompleter::load() {
	QByteArray data("{}");
	loadInternal(data);
}

void InstrumentCompleter::loadInternal(const QByteArray &data) {
	InstrumentDao dao;
	QList<InstrumentType> ins;
	result_t resp = dao.getInstrumentTypeList(&ins);
	if (resp.isOk())
	{
		_model->clear();

		for (auto &it : ins) {
			QStandardItem *pkgItem = new QStandardItem(it.name);
			pkgItem->setData(it.typeId, Constant::IdRole);
			pkgItem->setData(it.pinyin, Constant::PinyinRole);
			_model->appendRow(pkgItem);
		}
	}
	else
	{
		emit error(resp.msg());
		return;
	}
	/*_http.post(url(PATH_INSTRUMENT_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			emit error(resp.errorString());
			return;
		}

		_model->clear();

		QList<QVariant> pkgs = resp.getAsList("instrument_list");
		for (auto &pkg : pkgs) {
			QVariantMap map = pkg.toMap();
			QStandardItem *pkgItem = new QStandardItem(map["instrument_name"].toString());
			pkgItem->setData(map["instrument_id"], Constant::IdRole);
			pkgItem->setData(map["pinyin_code"], Constant::PinyinRole);
			_model->appendRow(pkgItem);
		}
	});*/
}