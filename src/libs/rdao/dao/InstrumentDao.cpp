#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "instrumentdao.h"
#include "errors.h"

result_t InstrumentDao::getInstrumentType(int typeId, InstrumentType* insType)
{
	QSqlQuery q;
	q.prepare("SELECT category, name, pinyin, photo, is_vip"
		" FROM t_instrument_type"
		" WHERE id = ?");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械类型的信息";

	if (insType) {
		insType->typeId = typeId;
		insType->category = static_cast<Rt::InstrumentCategory>(q.value(0).toInt());
		insType->name = q.value(1).toString();
		insType->pinyin = q.value(2).toString();
		insType->photo = q.value(3).toString();
		insType->isVip = q.value(4).toBool();
	}

	return 0;
}

result_t InstrumentDao::getInstrumentTypeList(
	QList<InstrumentType> *insTypes, int page/* = 1*/, int count/* = -1*/)
{
	QSqlQuery q;
	QString sql = "SELECT id, category, name, pinyin, photo, is_vip"
		" FROM t_instrument_type";

	if (-1 != count) { // do pagination
		count = qMax(10, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2").arg((page-1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	if (insTypes) {
		InstrumentType it;
		while (q.next()) {
			it.typeId = q.value(0).toInt();
			it.category = static_cast<Rt::InstrumentCategory>(q.value(1).toInt());
			it.name = q.value(2).toString();
			it.pinyin = q.value(3).toString();
			it.photo = q.value(4).toString();
			it.isVip = q.value(5).toBool();
			insTypes->append(it);
		}
	}
	return 0;
}

result_t InstrumentDao::updateInstrumentType(const InstrumentType &it)
{
	QSqlQuery query;
	query.prepare("UPDATE t_instrument_type SET category = ?, name = ?, pinyin = ?, photo = ?, is_vip = ?"
		" WHERE id = ?");
	query.addBindValue(it.category);
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
	query.prepare("INSERT INTO t_instrument_type (category, name, pinyin, photo, is_vip)"
	" VALUES (?, ?, ?, ?, ?)");
	query.addBindValue(it.category);
	query.addBindValue(it.name);
	query.addBindValue(it.pinyin);
	query.addBindValue(it.photo);
	query.addBindValue(it.isVip);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t InstrumentDao::getInstrument(const QString& udi, Instrument* ins)
{
	QSqlQuery q;
	q.prepare("SELECT a.type_id, a.name, a.photo, a.package_udi, a.price, b.category, b.is_vip"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id=b.id"
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
		ins->category = static_cast<Rt::InstrumentCategory>(q.value(6).toInt());
	}

	return 0;
}

result_t InstrumentDao::getInstrumentList(
	QList<Instrument> *instruments, int page/* = 1*/, int count/* = -1*/)
{
	QSqlQuery q;
	QString sql = "SELECT a.udi, a.type_id, a.name, a.photo, a.package_udi, a.price, b.category, b.is_vip"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id=b.id";

	if (-1 != count) { // do pagination
		count = qMax(10, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2").arg((page - 1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	if (instruments) {
		Instrument ins;
		while (q.next()) {
			ins.udi = q.value(0).toString();;
			ins.typeId = q.value(1).toInt();
			ins.name = q.value(2).toString();
			ins.photo = q.value(3).toString();
			ins.packageUdi = q.value(4).toInt();
			ins.price = q.value(5).toInt();
			ins.isVip = q.value(6).toBool();
			ins.category = static_cast<Rt::InstrumentCategory>(q.value(7).toInt());
			instruments->append(ins);
		}
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