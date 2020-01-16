#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "instrumentdao.h"
#include "daoutil.h"
#include "errors.h"

result_t InstrumentDao::getInstrumentType(int typeId, InstrumentType* insType)
{
	QSqlQuery q;
	q.prepare("SELECT name, pinyin, photo, is_vip"
		" FROM t_instrument_type"
		" WHERE id = ?");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械类型的信息";

	if (insType) {
		insType->typeId = typeId;
		insType->typeName = q.value(0).toString();
		insType->pinyin = q.value(1).toString();
		insType->photo = q.value(2).toString();
		insType->isVip = q.value(3).toBool();
	}

	return 0;
}

result_t InstrumentDao::getInstrumentTypeList(
	QList<InstrumentType> *its, int *total/* = nullptr*/, int page/* = 1*/, int count/* = 20*/)
{
	if (!its) return 0;
	its->clear();
	bool paginated = nullptr != total;

	QSqlQuery q;
	QString sql = "SELECT id, name, pinyin, photo, is_vip FROM t_instrument_type";

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM t_instrument_type").arg((page-1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	InstrumentType it;
	while (q.next()) {
		it.typeId = q.value(0).toInt();
		it.typeName = q.value(1).toString();
		it.pinyin = q.value(2).toString();
		it.photo = q.value(3).toString();
		it.isVip = q.value(4).toBool();
		its->append(it);
	}

	if (paginated) {
		if (!q.nextResult() || !q.first())
			return "Could not determine the total number";
		*total = q.value(0).toInt();
	}

	return 0;
}

result_t InstrumentDao::updateInstrumentType(const InstrumentType &it)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument_type SET name = ?, pinyin = ?, photo = ?, is_vip = ?"
		" WHERE id = ?");
	query.addBindValue(it.typeName);
	query.addBindValue(it.pinyin);
	query.addBindValue(it.photo);
	query.addBindValue(it.isVip);
	query.addBindValue(it.typeId);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::addInstrumentType(const InstrumentType &it, int * typeId)
{
	QSqlQuery query;
	query.prepare("INSERT INTO t_instrument_type (name, pinyin, photo, is_vip)"
	" VALUES (?, ?, ?, ?)");
	query.addBindValue(it.typeName);
	query.addBindValue(it.pinyin);
	query.addBindValue(it.photo);
	query.addBindValue(it.isVip);

	if (!query.exec())
		return query.lastError().text();

	// get type id of the new instrument
	if (!query.exec("SELECT id FROM t_instrument_type ORDER BY id DESC LIMIT 1"))
		return query.lastError().text();
	if (!query.first()) // this should never happen
		return "Internal Error";
	*typeId = query.value(0).toInt();

	return 0;
}

result_t InstrumentDao::getInstrument(const QString &udi, Instrument *ins, int cycle/* = -1*/)
{
	if (!ins) return 0;
	bool getLatest = cycle < 0;

	QString sql = "SELECT a.type_id, a.sn, a.alias, a.photo, %1, a.price, a.cycle, b.name, b.is_vip, c.sn, c.cycle, d.name"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id = b.id"
		" LEFT JOIN t_package c ON %1 = c.udi"
		" LEFT JOIN t_package_type d ON c.type_id = d.id"
		" WHERE a.udi = ?";
	
	QString pkgUdi = "a.pkg_udi";
	int pkgCycle = 0;
	if (!getLatest) {
		result_t res = findBoundPackage(udi, cycle, &pkgUdi, &pkgCycle);
		if (!res.isOk()) return res;
		pkgUdi = '\'' + pkgUdi + '\'';
	}
	sql = sql.arg(pkgUdi);

	QSqlQuery q;
	q.prepare(sql);
	q.addBindValue(udi);

	if (!q.exec())
		return q.lastError().text();

	if (!q.first())
		return "没有找到对应器械的信息";

	// see if cycle exceeds
	int latestCycle = q.value(6).toInt();
	if (cycle > latestCycle)
		return QString("器械尚未到达指定周期: %1").arg(cycle);

	ins->udi = udi;
	ins->typeId = q.value(0).toInt();
	ins->alias = q.value(2).toString();
	ins->photo = q.value(3).toString();
	ins->packageUdi = q.value(4).toString();
	ins->price = q.value(5).toInt();
	ins->cycle = getLatest ? latestCycle : cycle;
	ins->typeName = q.value(7).toString();
	ins->isVip = q.value(8).toBool();
	ins->packageName = DaoUtil::udiName(q.value(11).toString(), q.value(9).toInt());
	ins->packageCycle = getLatest ? q.value(10).toInt() : pkgCycle;
	ins->name = DaoUtil::udiName(ins->typeName, q.value(1).toInt());

	return 0;
}

result_t InstrumentDao::getInstrumentList(
	QList<Instrument> *instruments, int *total/* = nullptr*/, int page/* = 1*/, int count/* = 20*/)
{
	if (!instruments) return 0;
	instruments->clear();
	bool paginated = nullptr != total;

	QSqlQuery q;
	QString sql = "SELECT a.udi, a.type_id, a.sn, a.alias, a.photo, a.pkg_udi, a.price, b.name, b.is_vip, c.sn, d.name"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id = b.id"
		" LEFT JOIN t_package c ON a.pkg_udi = c.udi"
		" LEFT JOIN t_package_type d ON c.type_id = d.id";

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM t_instrument").arg((page - 1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	Instrument ins;
	while (q.next()) {
		ins.udi = q.value(0).toString();
		ins.typeId = q.value(1).toInt();
		ins.alias = q.value(3).toString();
		ins.photo = q.value(4).toString();
		ins.packageUdi = q.value(5).toString();
		ins.price = q.value(6).toInt();
		ins.typeName = q.value(7).toString();
		ins.isVip = q.value(8).toBool();
		ins.packageName = DaoUtil::udiName(q.value(10).toString(), q.value(9).toInt());
		ins.name = DaoUtil::udiName(ins.typeName, q.value(2).toInt());
		instruments->append(ins);
	}

	if (paginated) {
		if (!q.nextResult() || !q.first())
			return "Could not determine the total number";
		*total = q.value(0).toInt();
	}

	return 0;
}

result_t InstrumentDao::addInstrument(const Instrument &ins)
{
	QSqlQuery q;
	q.prepare("SELECT 1 FROM t_instrument WHERE udi = ?");
	q.addBindValue(ins.udi);
	if (!q.exec())
		return kErrorDbUnreachable;
	if (q.first())
		return "该器械已入库，请勿重复操作";

	// when we add a new instrument, we don't know its package udi.
	// FIXME: maybe we should lock the table or use a transaction when query the max sn.
	q.prepare("INSERT INTO t_instrument (udi, type_id, sn, alias, photo, price)"
		" SELECT ?, ?, IFNULL(MAX(sn)+1, 1), ?, ?, ? FROM t_instrument WHERE type_id=?");
	q.addBindValue(ins.udi);
	q.addBindValue(ins.typeId);
	q.addBindValue(ins.alias);
	q.addBindValue(ins.photo);
	q.addBindValue(ins.price);
	q.addBindValue(ins.typeId);

	if (!q.exec())
		return q.lastError().text();
	return 0;
}

result_t InstrumentDao::updateInstrument(const Instrument &it)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument SET alias = ?, photo = ?, price = ?"
		" WHERE id = ?");
	query.addBindValue(it.alias);
	query.addBindValue(it.photo);
	query.addBindValue(it.price);
	query.addBindValue(it.udi);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::findBoundPackage(
	const QString &insUdi, int insCycle, QString *pkgUdi, int *pkgCycle/* = 0*/)
{
	QSqlQuery q;
	q.prepare("SELECT pkg_udi, pkg_cycle_stamp, ins_cycle_stamp FROM t_package_detail"
		" WHERE ins_udi=? AND ins_cycle_stamp<? ORDER BY ins_cycle_stamp DESC LIMIT 1");
	q.addBindValue(insUdi);
	q.addBindValue(0 == insCycle ? 1 : insCycle);

	if (!q.exec())
		return q.lastError().text();

	if (!q.first())
		return "No bound package found with specified instrument cycle";

	if (pkgUdi) *pkgUdi = q.value(0).toString();
	if (pkgCycle) *pkgCycle = q.value(1).toInt() + (insCycle - q.value(2).toInt());

	return 0;
}

result_t InstrumentDao::setInstrumentPhoto(const QString& udi, const QString& photo)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument SET photo = ?"
		" WHERE udi = ?");
	query.addBindValue(photo);
	query.addBindValue(udi);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::getInstrumentPhoto(const QString& udi, QString *photo)
{
	QSqlQuery q;
	q.prepare("SELECT photo"
		" FROM t_instrument"
		" WHERE udi = ?");
	q.addBindValue(udi);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械的图片";

	if (photo) {
		*photo = q.value(0).toString();
	}

	return 0;
}

result_t InstrumentDao::setInstrumentPhoto(int typeId, const QString& photo)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument_type SET photo = ?"
		" WHERE id = ?");
	query.addBindValue(photo);
	query.addBindValue(typeId);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::getInstrumentPhoto(int typeId, QString *photo)
{
	QSqlQuery q;
	q.prepare("SELECT photo"
		" FROM t_instrument_type"
		" WHERE id = ?");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械类型的图片";

	if (photo) {
		*photo = q.value(0).toString();
	}

	return 0;
}
