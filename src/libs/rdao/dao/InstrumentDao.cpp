#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "instrumentdao.h"
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
		insType->name = q.value(0).toString();
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
		it.name = q.value(1).toString();
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
	query.addBindValue(it.name);
	query.addBindValue(it.pinyin);
	query.addBindValue(it.photo);
	query.addBindValue(it.isVip);
	query.addBindValue(it.typeId);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::addInstrumentType(const InstrumentType &it)
{
	QSqlQuery query;
	query.prepare("INSERT INTO t_instrument_type (name, pinyin, photo, is_vip)"
	" VALUES (?, ?, ?, ?)");
	query.addBindValue(it.name);
	query.addBindValue(it.pinyin);
	query.addBindValue(it.photo);
	query.addBindValue(it.isVip);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::getInstrument(const QString &udi, Instrument *ins)
{
	QSqlQuery q;
	q.prepare("SELECT a.type_id, a.name, a.photo, a.pkg_udi, a.price, b.is_vip"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id = b.id"
		" WHERE a.udi = ?");
	q.addBindValue(udi);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械的信息";

	if (ins) {
		ins->udi = udi;
		ins->typeId = q.value(0).toInt();
		ins->name = q.value(1).toString();
		ins->photo = q.value(2).toString();
		ins->packageUdi = q.value(3).toString();
		ins->price = q.value(4).toInt();
		ins->isVip = q.value(5).toBool();
	}

	return 0;
}

result_t InstrumentDao::getInstrumentList(
	QList<Instrument> *instruments, int *total/* = nullptr*/, int page/* = 1*/, int count/* = 20*/)
{
	if (!instruments) return 0;
	bool paginated = nullptr != total;

	QSqlQuery q;
	QString sql = "SELECT a.udi, a.type_id, a.name, a.photo, a.pkg_udi, a.price, b.is_vip"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id = b.id";

	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM t_instrument").arg((page - 1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	Instrument ins;
	while (q.next()) {
		ins.udi = q.value(0).toString();;
		ins.typeId = q.value(1).toInt();
		ins.name = q.value(2).toString();
		ins.photo = q.value(3).toString();
		ins.packageUdi = q.value(4).toString();
		ins.price = q.value(5).toInt();
		ins.isVip = q.value(6).toBool();
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

	// when we add a new instrument, we don't know its package udi  
	q.prepare("INSERT INTO t_instrument (udi, type_id, name, photo, price)"
		" VALUES (?, ?, ?, ?, ?)");
	q.addBindValue(ins.udi);
	q.addBindValue(ins.typeId);
	q.addBindValue(ins.name);
	q.addBindValue(ins.photo);
	q.addBindValue(ins.price);

	if (!q.exec())
		return q.lastError().text();
	return 0;
}

result_t InstrumentDao::updateInstrument(const Instrument &it)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument SET name = ?, photo = ?, price = ?"
		" WHERE id = ?");
	query.addBindValue(it.name);
	query.addBindValue(it.photo);
	query.addBindValue(it.price);
	query.addBindValue(it.udi);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}