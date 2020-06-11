#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "packagedao.h"
#include "daoutil.h"
#include "errors.h"

result_t PackageDao::getPackageType(
	int typeId, PackageType* pt, bool withInstrumentTypes /*= false*/)
{
	if (!pt) return 0;

	QSqlQuery q;
	q.prepare("SELECT a.category, a.name, a.pinyin, a.photo, a.sterilize_type, a.for_implants, a.pack_type_id, a.dept_id, b.name, c.name"
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
	pt->detail.clear();
	pt->typeId = typeId;
	pt->category = static_cast<Rt::PackageCategory>(q.value(0).toInt());
	pt->typeName = q.value(1).toString();
	pt->pinyin = q.value(2).toString();
	pt->photo = q.value(3).toString();
	pt->sterMethod = static_cast<Rt::SterilizeMethod>(q.value(4).toInt());
	pt->forImplants = q.value(5).toBool();
	pt->packType.id = q.value(6).toInt();
	pt->dept.id = q.value(7).toInt();
	pt->packType.name = q.value(8).toString();
	pt->dept.name = q.value(9).toString();

	if (!withInstrumentTypes) {
		return 0;
	}

	// else we fetch the instrument types
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
		item.insTypeId = q.value(0).toInt();
		item.insNum = q.value(1).toInt();
		item.insName = q.value(2).toString();
		pt->detail.append(item);
	}

	return 0;
}

result_t PackageDao::getPackageTypeList(
	QList<PackageType> *pts, const QString &kw, int *total/* = nullptr*/, int page/* = 1*/, int count/* = 20*/)
{
	if (!pts) return 0;
	pts->clear();
	bool paginated = nullptr != total;

	QSqlQuery q;
	QString sql = "SELECT a.id, a.category, a.name, a.pinyin, a.photo, a.sterilize_type, a.pack_type_id, a.dept_id, b.name, c.name"
		" FROM t_package_type a"
		" LEFT JOIN t_pack_type b ON a.pack_type_id = b.id"
		" LEFT JOIN t_dept c ON a.dept_id = c.id";
	if (!kw.isEmpty())
	{
		sql.append(" WHERE a.pinyin LIKE '%").append(kw).append("%'");
	}


	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM t_package_type").arg((page-1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	while (q.next()) {
		PackageType pt;
		pt.typeId = q.value(0).toInt();
		pt.category = static_cast<Rt::PackageCategory>(q.value(1).toInt());
		pt.typeName = q.value(2).toString();
		pt.pinyin = q.value(3).toString();
		pt.photo = q.value(4).toString();
		pt.sterMethod = static_cast<Rt::SterilizeMethod>(q.value(5).toInt());
		pt.packType.id = q.value(6).toInt();
		pt.dept.id = q.value(7).toInt();
		pt.packType.name = q.value(8).toString();
		pt.dept.name = q.value(9).toString();
		pts->append(pt);
	}

	if (paginated) {
		if (!q.nextResult() || !q.first())
			return "Could not determine the total number";
		*total = q.value(0).toInt();
	}

	return 0;
}

result_t PackageDao::delPackage(const QString& udi)
{
	QSqlQuery q;
	q.prepare("SELECT pkg_udi"
		" FROM r_recycle"
		" WHERE pkg_udi = ?");
	q.addBindValue(udi);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first()) {
		QSqlDatabase db = QSqlDatabase::database();
		db.transaction();

		QSqlQuery query;
		query.prepare("DELETE FROM t_package  WHERE udi = ?");
		query.addBindValue(udi);

		if (!query.exec())
		{
			db.rollback();
			return query.lastError().text();
		}
			

		query.prepare("DELETE FROM t_package_detail WHERE pkg_udi = ?");
		query.addBindValue(udi);

		if (!query.exec())
		{
			db.rollback();
			return query.lastError().text();
		}

		db.commit();
	}
	else
	{
		return "存在该包记录，无法删除这个包";
	}

	return 0;
}

result_t PackageDao::delPackageType(int typeId)
{
	QSqlQuery q;
	q.prepare("SELECT udi"
		" FROM t_package"
		" WHERE type_id = ?");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first()) {
		QSqlDatabase db = QSqlDatabase::database();
		db.transaction();

		QSqlQuery query;
		query.prepare("DELETE FROM t_package_type  WHERE id = ?");
		query.addBindValue(typeId);

		if (!query.exec())
		{
			db.rollback();
			return query.lastError().text();
		}

		query.prepare("DELETE FROM t_package_type_detail WHERE pkg_type_id = ?");
		query.addBindValue(typeId);

		if (!query.exec())
		{
			db.rollback();
			return query.lastError().text();
		}

		db.commit();
	}
	else
	{
		return "存在该类型包实体，无法删除这个包类型";
	}

	return 0;
}

result_t PackageDao::addPackageType(const PackageType &pt, int *typeId)
{
	QSqlQuery q;
	q.prepare("INSERT INTO t_package_type (category, name, pinyin, sterilize_type, for_implants, pack_type_id, dept_id)"
	" VALUES (?, ?, ?, ?, ?, ?, ?)");
	q.addBindValue(pt.category);
	q.addBindValue(pt.typeName);
	q.addBindValue(pt.pinyin);
	q.addBindValue(pt.sterMethod);
	q.addBindValue(pt.forImplants);
	q.addBindValue(pt.packType.id);
	q.addBindValue(pt.dept.id);

	if (!q.exec())
		return q.lastError().text();

	// get type id of the new package
	if (!q.exec("SELECT id FROM t_package_type ORDER BY id DESC LIMIT 1"))
		return q.lastError().text();
	if (!q.first()) // this should never happen
		return "Internal Error";
	*typeId = q.value(0).toInt();

	// add instrument types
	if (pt.detail.isEmpty()) {
		qWarning("You are trying to add a new package type without instrument types bound!");
		return 0;
	}

	QVariantList pkgTypeIds, insTypeIds, insNums;
	for each(const PackageType::DetailItem &detail in pt.detail) {
		pkgTypeIds << *typeId;
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
	if (!pkg) return 0;

	QSqlQuery q;
	q.prepare("SELECT a.sn, a.alias, a.photo, a.type_id, a.cycle, a.status, a.is_del,"
		" b.name, b.category, b.sterilize_type, b.for_implants, b.pack_type_id, b.dept_id, c.name, c.valid_period, d.name"
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
	pkg->detail.clear();
	pkg->instruments.clear();
	pkg->udi = udi;
	pkg->alias = q.value(1).toString();
	pkg->photo = q.value(2).toString();
	pkg->typeId = q.value(3).toInt();
	pkg->cycle = q.value(4).toInt();
	pkg->status = static_cast<Rt::FlowStatus>(q.value(5).toInt());
	pkg->is_del = q.value(6).toBool();
	pkg->typeName = q.value(7).toString();
	pkg->category = static_cast<Rt::PackageCategory>(q.value(8).toInt());
	pkg->sterMethod = static_cast<Rt::SterilizeMethod>(q.value(9).toInt());
	pkg->forImplants = q.value(10).toBool();
	pkg->packType.id = q.value(11).toInt();
	pkg->dept.id = q.value(12).toInt();
	pkg->packType.name = q.value(13).toString();
	pkg->packType.validPeriod = q.value(14).toUInt();
	pkg->dept.name = q.value(15).toString();
	pkg->name = DaoUtil::udiName(pkg->typeName, q.value(0).toInt());

	if (!withInstruments) {
		return 0;
	}

	q.prepare("SELECT a.udi, a.type_id, a.sn, a.alias, a.photo, a.price, b.name, b.is_vip"
		" FROM t_instrument a"
		" LEFT JOIN t_instrument_type b ON a.type_id=b.id"
		" WHERE a.pkg_udi = ? AND a.is_del = 0");
	q.addBindValue(udi);
	if (!q.exec())
		return kErrorDbUnreachable;

	Instrument ins;
	while (q.next())
	{
		ins.packageUdi = udi;
		ins.udi = q.value(0).toString();
		ins.typeId = q.value(1).toInt();
		ins.alias = q.value(3).toString();
		// FIXME: 4,5 unneccesary?
		ins.typeName = q.value(6).toString();
		ins.isVip = q.value(7).toBool();
		ins.name = DaoUtil::udiName(ins.typeName, q.value(2).toInt());
		pkg->instruments.append(ins);
	}

	return 0;
}

result_t PackageDao::getPackageList(
	QList<Package> *pkgs, const QString &kw, int *total/* = nullptr*/, int page/* = 1*/, int count/* = 20*/)
{
	if (!pkgs) return 0;
	pkgs->clear();
	bool paginated = nullptr != total;

	QSqlQuery q;
	QString sql = "SELECT a.udi, a.sn, a.alias, a.photo, a.type_id, a.cycle, a.status,"
		" b.name, b.category, b.sterilize_type, b.for_implants, b.pack_type_id, b.dept_id, c.name, c.valid_period, d.name"
		" FROM t_package a"
		" LEFT JOIN t_package_type b ON a.type_id = b.id"
		" LEFT JOIN t_pack_type c ON b.pack_type_id = c.id"
		" LEFT JOIN t_dept d ON b.dept_id = d.id"
		" WHERE a.is_del = 0";

	if (!kw.isEmpty())
	{
		sql.append("AND b.pinyin LIKE '%").append(kw).append("%'");
	}


	if (paginated) { // do pagination
		count = qMax(20, count);
		page = qMax(1, page);
		sql.append(QString(" LIMIT %1, %2;SELECT COUNT(id) FROM t_package").arg((page - 1)*count).arg(count));
	}

	if (!q.exec(sql))
		return q.lastError().text();

	Package pkg;
	while (q.next()) {
		pkg.udi = q.value(0).toString();
		pkg.alias = q.value(2).toString();
		pkg.photo = q.value(3).toString();
		pkg.typeId = q.value(4).toInt();
		pkg.cycle = q.value(5).toInt();
		pkg.status = static_cast<Rt::FlowStatus>(q.value(6).toInt());
		pkg.typeName = q.value(7).toString();
		pkg.category = static_cast<Rt::PackageCategory>(q.value(8).toInt());
		pkg.sterMethod = static_cast<Rt::SterilizeMethod>(q.value(9).toInt());
		pkg.forImplants = q.value(10).toBool();
		pkg.packType.id = q.value(11).toInt();
		pkg.dept.id = q.value(12).toInt();
		pkg.packType.name = q.value(13).toString();
		pkg.packType.validPeriod = q.value(14).toUInt();
		pkg.dept.name = q.value(15).toString();
		pkg.name = DaoUtil::udiName(pkg.typeName, q.value(1).toInt());
		pkgs->append(pkg);
	}

	if (paginated) {
		if (!q.nextResult() || !q.first())
			return "Could not determine the total number";
		*total = q.value(0).toInt();
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

	q.prepare("INSERT INTO t_package (udi, type_id, sn, alias, photo)"
		" SELECT ?, ?, IFNULL(MAX(sn)+1, 1), ?, ? FROM t_package WHERE type_id=?");
	q.addBindValue(pkg.udi);
	q.addBindValue(pkg.typeId);
	q.addBindValue(pkg.alias);
	q.addBindValue(pkg.photo);
	q.addBindValue(pkg.typeId);

	if (!q.exec())
		return q.lastError().text();

	// add instruments
	if (pkg.instruments.isEmpty()) {
		qWarning("You are trying to add a new package without instruments!");
		return 0;
	}

	// insert new bindings in t_package_detail
	QString sql = "INSERT INTO t_package_detail"
		" (ins_udi, ins_cycle_stamp, pkg_udi, pkg_cycle_stamp, bound_tm) VALUES";
	QStringList insUdis;
	int validCount = 0;
	for each(const Instrument &ins in pkg.instruments) {
		if (ins.packageUdi.isEmpty()) {
			validCount++;
			QString values = QString(" ('%1', %3, '%2', %4, now()),").
				arg(ins.udi, pkg.udi).arg(ins.cycle).arg(pkg.cycle);
			sql.append(values);
			insUdis << QString("'%1'").arg(ins.udi);
		}
	}

	if (0 == validCount) {
		qWarning("You are trying to add a new package with bound instruments!");
		return 0;
	}

	sql.chop(1);		
	if (!q.exec(sql))
		return q.lastError().text();

	// update the newest package udi for each instrument in t_instrument
	sql = QString("UPDATE t_instrument"
		" SET pkg_udi='%1' WHERE udi IN (%2)").arg(pkg.udi, insUdis.join(','));
	if (!q.exec(sql))
		return q.lastError().text();

	return 0;
}

result_t PackageDao::changePackageUDI(const QString &old_pkg_udi, const QString &new_pkg_udi)
{
	QSqlQuery q;
	q.prepare("SELECT 1 FROM t_package WHERE udi = ?");
	q.addBindValue(new_pkg_udi);
	if (!q.exec())
		return kErrorDbUnreachable;
	if (q.first())
		return "该UDI已经入库，无法替换";

	q.prepare("SELECT * FROM t_package WHERE udi = ?");
	q.addBindValue(old_pkg_udi);
	if (!q.exec())
		return kErrorDbUnreachable;
	if (q.first()) {
		QSqlDatabase db = QSqlDatabase::database();
		db.transaction();

		int type_id = q.value(2).toInt();
		int sn = q.value(3).toInt();
		QString alias = q.value(4).toString();
		QString photo = q.value(5).toString();
		int cycle = q.value(6).toInt();

		int pkg_cycle = q.value(9).toInt();

		q.prepare("INSERT INTO t_package (udi, type_id, sn, alias, photo, cycle, pkg_cycle, old_udi, old_pkg_cycle)"
			" VALUES (?, ?, ?, ?, ?, ?, ?,?,?)");
		q.addBindValue(new_pkg_udi);
		q.addBindValue(type_id);
		q.addBindValue(sn);
		q.addBindValue(alias);
		q.addBindValue(photo);
		q.addBindValue(cycle);
		q.addBindValue(pkg_cycle);
		q.addBindValue(old_pkg_udi);
		q.addBindValue(pkg_cycle);

		if (!q.exec())
		{
			db.rollback();
			return q.lastError().text();
		}

		QString sql = QString("UPDATE t_package"
			" SET is_del = 1 WHERE udi = '%1' ").arg(old_pkg_udi);
		if (!q.exec(sql))
		{
			db.rollback();
			return q.lastError().text();
		}

		sql = QString("SELECT * FROM t_package_detail WHERE pkg_udi = '%1'").arg(old_pkg_udi);
		if (!q.exec(sql))
		{
			db.rollback();
			return q.lastError().text();
		}

		QStringList insUdis;
		sql = QString("INSERT INTO t_package_detail ( pkg_udi, pkg_cycle_stamp, ins_udi, ins_cycle_stamp, bound_tm) VALUES");
		while(q.next())
		{
			int pkg_cycle_stamp = q.value(2).toInt();
			QString insUDI = q.value(3).toString();
			int ins_cycle_stamp = q.value(4).toInt();

			QString values = QString(" ('%1', %2, '%3', %4, now()),").arg(new_pkg_udi).arg(pkg_cycle_stamp).arg(insUDI).arg(ins_cycle_stamp);
			sql.append(values);
			insUdis << QString("'%1'").arg(insUDI);
		}

		sql.chop(1);
		if (!q.exec(sql))
		{
			db.rollback();
			return q.lastError().text();
		}

		sql = QString("UPDATE t_package_detail"
			" SET status = 0, unbound_tm = now() WHERE pkg_udi = '%1' AND ins_udi IN (%2)").arg(old_pkg_udi, insUdis.join(','));
		if (!q.exec(sql))
		{
			db.rollback();
			return q.lastError().text();
		}

		// update the newest package udi for each instrument in t_instrument
		sql = QString("UPDATE t_instrument"
			" SET pkg_udi='%1' WHERE udi IN (%2)").arg(new_pkg_udi, insUdis.join(','));
		if (!q.exec(sql))
		{
			db.rollback();
			return q.lastError().text();
		}

		db.commit();
	}
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

result_t PackageDao::getPackageQualityControl(const Package &pkg, PackageQualityControl *pqc)
{
	const char *kNotFoundErr = "未找到该包对应的质控信息";
	if(pqc) *pqc = PackageQualityControl(); // reset
	
	QSqlQuery q;
	if (Rt::WashChecked < pkg.status) {
		// packed, fetch expiration time as well as the recall information
		q.prepare("SELECT a.recalled, b.expire_time, TO_DAYS(NOW())>TO_DAYS(b.expire_time) as expired"
			" FROM r_package a"
			" LEFT JOIN r_pack b ON a.pkg_udi = b.pkg_udi AND a.pkg_cycle = b.pkg_cycle"
			" WHERE a.pkg_udi=? AND a.pkg_cycle=?");
		q.addBindValue(pkg.udi);
		q.addBindValue(pkg.cycle);
		if (!q.exec())
			return q.lastError().text();
		if (!q.first())
			return kNotFoundErr;
		if (pqc) {
			pqc->isRecalled = q.value(0).toBool();
			pqc->expireDate = q.value(1).toDate();
			pqc->isExpired = q.value(2).toBool();
		}
	}

	if (Rt::Sterilized < pkg.status) {
		// sterilization result checked, fetch the results
		// TODO: use JOIN here may take a long time, exec two queries?
		q.prepare("SELECT a.wet_pack, b.phy_check_result, b.che_check_result, b.bio_check_result"
			" FROM r_ster_package a"
			" LEFT JOIN r_ster_batch b ON a.batch_id = b.batch_id"
			" WHERE a.pkg_udi=? AND a.pkg_cycle=?");
		q.addBindValue(pkg.udi);
		q.addBindValue(pkg.cycle);
		if (!q.exec())
			return q.lastError().text();
		if (!q.first())
			return kNotFoundErr;
		if (pqc) {
			pqc->isWetPack = q.value(0).toBool();
			pqc->phyResult = static_cast<Rt::SterilizeVerdict>(q.value(1).toInt());
			pqc->cheResult = static_cast<Rt::SterilizeVerdict>(q.value(2).toInt());
			pqc->bioResult = static_cast<Rt::SterilizeVerdict>(q.value(3).toInt());
		}
	}

	return 0;
}

result_t PackageDao::setPackagePhoto(const QString& udi, const QString& photo)
{
	QSqlQuery query;
	query.prepare("UPDATE t_package SET photo = ?"
		" WHERE udi = ?");
	query.addBindValue(photo);
	query.addBindValue(udi);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t PackageDao::getPackagePhoto(const QString& udi, QString *photo)
{
	QSqlQuery q;
	q.prepare("SELECT photo"
		" FROM t_package"
		" WHERE udi = ?");
	q.addBindValue(udi);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械包的图片";

	if (photo) {
		*photo = q.value(0).toString();
	}

	return 0;
}

result_t PackageDao::setPackagePhoto(int typeId, const QString& photo)
{
	QSqlQuery query;
	query.prepare("UPDATE t_package_type SET photo = ?"
		" WHERE id = ?");
	query.addBindValue(photo);
	query.addBindValue(typeId);

	if (!query.exec())
		return query.lastError().text();
	return 0;
}

result_t PackageDao::getPackagePhoto(int typeId, QString *photo)
{
	QSqlQuery q;
	q.prepare("SELECT photo"
		" FROM t_package_type"
		" WHERE id = ?");
	q.addBindValue(typeId);

	if (!q.exec())
		return kErrorDbUnreachable;

	if (!q.first())
		return "没有找到对应器械包类型的图片";

	if (photo) {
		*photo = q.value(0).toString();
	}

	return 0;
}