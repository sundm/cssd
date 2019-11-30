#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "packagedao.h"
#include "errors.h"

result_t PackageDao::getPackageType(
	int typeId, PackageType* pt, bool withInstrumentTypes /*= false*/)
{
	QSqlQuery q;
	q.prepare("SELECT a.category, a.name, a.pinyin, a.photo, a.sterilize_type, a.pack_type_id, a.dept_id, b.name, c.name"
		" FROM t_package_type a"
		" LEFT JOIN t_pack_type b ON a.pack_type_id = b.id"
		" LEFT JOIN t_dept c ON a.dept_id = c.id"
		" WHERE a.id = ? ");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应包的信息";

	// reset package type infos
	if (pt) {
		pt->detail.clear();
		pt->typeId = typeId;
		pt->category = static_cast<Rt::PackageCategory>(q.value(0).toInt());
		pt->name = q.value(1).toString();
		pt->pinyin = q.value(2).toString();
		pt->photo = q.value(3).toString();
		pt->sterType = static_cast<Rt::SterilizeType>(q.value(4).toInt());
		pt->packType.id = q.value(5).toInt();
		pt->dept.id = q.value(6).toInt();
		pt->packType.name = q.value(7).toString();
		pt->dept.name = q.value(8).toString();
	}

	if (!withInstrumentTypes)
		return 0;

	q.prepare("SELECT a.ins_type_id, a.num, b.name"
		" FROM t_package_type_detail a"
		" LEFT JOIN t_instrument_type b ON a.ins_type_id = b.id"
		" WHERE pkg_type_id = ?");
	q.addBindValue(typeId);
	if (!q.exec())
		return kErrorDbUnreachable;

	PackageType::DetailItem item;
	while (q.next())
	{
		item.insName = q.value(2).toString();
		item.insNum = q.value(1).toInt();
		item.insTypeId = q.value(0).toInt();
		pt->detail.append(item);
	}

	return 0;
}

result_t PackageDao::getPackageTypeList(
	QList<PackageType> *pts, int page/* = 1*/, int count/* = -1*/)
{
	QSqlQuery q;
	QString sql = "SELECT a.id, a.category, a.name, a.pinyin, a.photo, a.sterilize_type, a.pack_type_id, a.dept_id, b.name, c.name"
		" FROM t_package_type a"
		" LEFT JOIN t_pack_type b ON a.pack_type_id = b.id"
		" LEFT JOIN t_dept c ON a.dept_id = c.id";

	if (-1 != count) { // do pagination
		count = qMax(10, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2").arg((page-1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	if (pts) {
		while (q.next()) {
			PackageType pt;
			pt.typeId = q.value(0).toInt();
			pt.category = static_cast<Rt::PackageCategory>(q.value(1).toInt());
			pt.name = q.value(2).toString();
			pt.pinyin = q.value(3).toString();
			pt.photo = q.value(4).toString();
			pt.sterType = static_cast<Rt::SterilizeType>(q.value(5).toInt());
			pt.packType.id = q.value(6).toInt();
			pt.dept.id = q.value(7).toInt();
			pt.packType.name = q.value(8).toString();
			pt.dept.name = q.value(9).toString();
			pts->append(pt);
		}
	}
	return 0;
}

result_t PackageDao::addPackageType(const PackageType &pt)
{
	QSqlQuery q;
	q.prepare("INSERT INTO t_package_type (category, name, pinyin, sterilize_type, pack_type_id, dept_id)"
	" VALUES (?, ?, ?, ?, ?, ?)");
	q.addBindValue(pt.category);
	q.addBindValue(pt.name);
	q.addBindValue(pt.pinyin);
	q.addBindValue(pt.sterType);
	q.addBindValue(pt.packType.id);
	q.addBindValue(pt.dept.id);

	if (!q.exec())
		return q.lastError().text();

	// get type id of the new package
	if (!q.exec("SELECT id FROM t_package_type ORDER BY id DESC LIMIT 1"))
		return q.lastError().text();
	if (!q.first()) // this should never happen
		return "Internal Error";
	int typeId = q.value(0).toInt();

	// add instrument types
	if (pt.detail.isEmpty()) {
		qWarning("You are trying to add a new package type without instrument types bound!");
		return 0;
	}

	QVariantList pkgTypeIds, insTypeIds, insNums;
	for each(const PackageType::DetailItem &detail in pt.detail) {
		pkgTypeIds << typeId;
		insTypeIds << detail.insTypeId;
		insNums << detail.insNum;
	}

	q.prepare("INSERT INTO t_package_type_detail (`pkg_type_id`, `ins_type_id`, `num`) VALUES (?, ?, ?)");
	q.addBindValue(pkgTypeIds);
	q.addBindValue(insTypeIds);
	q.addBindValue(insNums);

	if (!q.execBatch(QSqlQuery::ValuesAsRows))
		return q.lastError().text();

	return 0;
}

result_t PackageDao::getPackage(
	const QString &udi, Package* pkg, bool withInstruments /*= false*/)
{
	QSqlQuery q;
	q.prepare("SELECT a.name, a.photo, a.type_id, a.cycle, a.status,"
		" b.category, b.sterilize_type, b.pack_type_id, b.dept_id, c.name, d.name"
		" FROM t_package a"
		" LEFT JOIN t_package_type b ON a.type_id = b.id"
		" LEFT JOIN t_pack_type c ON b.pack_type_id = c.id"
		" LEFT JOIN t_dept d ON b.dept_id = d.id"
		" WHERE a.udi = ?");
	q.addBindValue(udi);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应包的信息";

	// reset package infos
	if (pkg) {
		pkg->detail.clear();
		pkg->instruments.clear();
		pkg->udi = udi;
		pkg->name = q.value(0).toString();
		pkg->photo = q.value(1).toString();
		pkg->typeId = q.value(2).toInt();
		pkg->cycle = q.value(3).toInt();
		pkg->status = static_cast<Rt::FlowStatus>(q.value(4).toInt());
		pkg->category = static_cast<Rt::PackageCategory>(q.value(5).toInt());
		pkg->sterType = static_cast<Rt::SterilizeType>(q.value(6).toInt());
		pkg->packType.id = q.value(7).toInt();
		pkg->dept.id = q.value(8).toInt();
		pkg->packType.name = q.value(9).toString();
		pkg->dept.name = q.value(10).toString();
	}

	if (!withInstruments)
		return 0;

	q.prepare("SELECT a.ins_udi, b.type_id, b.name, b.photo, b.price, c.category, c.is_vip"
		" FROM t_package_detail a"
		" LEFT JOIN t_instrument b ON a.ins_udi = b.udi"
		" LEFT JOIN t_instrument_type c ON b.type_id = c.id"
		" WHERE a.pkg_udi = ? AND a.status = 1");
	q.addBindValue(udi);
	if (!q.exec())
		return kErrorDbUnreachable;

	Instrument ins;
	while (q.next())
	{
		ins.packageUdi = udi;
		ins.udi = q.value(0).toString();
		ins.typeId = q.value(1).toInt();
		ins.name = q.value(2).toString();
		ins.category = static_cast<Rt::InstrumentCategory>(q.value(5).toInt());
		ins.isVip = q.value(6).toBool();
		pkg->instruments.append(ins);
	}

	return 0;
}

result_t PackageDao::getPackageList(
	QList<Package> *pkgs, int page/* = 1*/, int count/* = -1*/)
{
	QSqlQuery q;
	QString sql = "SELECT a.udi, a.name, a.photo, a.type_id, a.cycle, a.status,"
		" b.category, b.sterilize_type, b.pack_type_id, b.dept_id, c.name, d.name"
		" FROM t_package a"
		" LEFT JOIN t_package_type b ON a.type_id = b.id"
		" LEFT JOIN t_pack_type c ON b.pack_type_id = c.id"
		" LEFT JOIN t_dept d ON b.dept_id = d.id";

	if (-1 != count) { // do pagination
		count = qMax(10, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2").arg((page - 1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	if (pkgs) {
		Package pkg;
		while (q.next()) {
			pkg.udi = q.value(0).toString();
			pkg.name = q.value(1).toString();
			pkg.photo = q.value(2).toString();
			pkg.typeId = q.value(3).toInt();
			pkg.cycle = q.value(4).toInt();
			pkg.status = static_cast<Rt::FlowStatus>(q.value(5).toInt());
			pkg.category = static_cast<Rt::PackageCategory>(q.value(6).toInt());
			pkg.sterType = static_cast<Rt::SterilizeType>(q.value(7).toInt());
			pkg.packType.id = q.value(8).toInt();
			pkg.dept.id = q.value(9).toInt();
			pkg.packType.name = q.value(10).toString();
			pkg.dept.name = q.value(11).toString();
			pkgs->append(pkg);
		}
	}
	return 0;
}

// TODO: use transactions
result_t PackageDao::addPackage(const Package &pkg)
{
	QSqlQuery q;
	q.prepare("SELECT 1 FROM t_package WHERE udi = ?");
	q.addBindValue(pkg.udi);
	if (!q.exec())
		return kErrorDbUnreachable;
	if (q.first())
		return "该物品包已入库，请勿重复操作";

	q.prepare("INSERT INTO t_package (udi, type_id, name, photo)"
		" VALUES (?, ?, ?, ?)");
	q.addBindValue(pkg.udi);
	q.addBindValue(pkg.typeId);
	q.addBindValue(pkg.name);
	q.addBindValue(pkg.photo);

	if (!q.exec())
		return q.lastError().text();

	// add instruments
	if (pkg.instruments.isEmpty()) {
		qWarning("You are trying to add a new package without instruments!");
		return 0;
	}

	//QString sql = QString("UPDATE t_instrument SET package_udi=%1"
	//	" WHERE udi IN (%2)").arg(pkg.udi, insUdis.join(','));
	QString sql = "INSERT INTO t_package_detail"
		" (ins_udi, ins_cycle_stamp, pkg_udi, pkg_cycle_stamp, bound_tm) VALUES";
	int validCount = 0;
	for each(const Instrument &ins in pkg.instruments) {
		if (ins.packageUdi.isEmpty()) {
			validCount++;
			QString values = QString(" ('%1', %3, '%2', %4, now()),").
				arg(ins.udi, pkg.udi).arg(ins.cycle).arg(pkg.cycle);
			sql.append(values);
		}
	}

	if (0 == validCount) {
		qWarning("You are trying to add a new package with bound instruments!");
		return 0;
	}

	sql.chop(1);		
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

result_t PackageDao::getPackTypeList(QList<PackType> *packTypes)
{
	QSqlQuery q;
	if (!q.exec("SELECT id, name, valid_period, standard_period FROM t_pack_type"))
		return q.lastError().text();

	if (packTypes) {
		PackType pt;
		while (q.next()) {
			pt.id = q.value(0).toInt();
			pt.name = q.value(1).toString();
			pt.validPeriod = q.value(2).toInt();
			pt.standardPeriod = q.value(3).toInt();
			packTypes->append(pt);
		}
	}

	return 0;
}

result_t PackageDao::updatePackType(const PackType &packType)
{
	QSqlQuery q;
	q.prepare("UPDATE t_pack_type SET valid_period = ?"
		" WHERE id = ?");
	q.addBindValue(packType.validPeriod);
	q.addBindValue(packType.id);

	if (!q.exec())
		return q.lastError().text();
	return 0;
}
