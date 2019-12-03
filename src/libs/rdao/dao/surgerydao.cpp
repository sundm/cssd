#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "surgerydao.h"
#include "errors.h"

result_t SurgeryDao::getSurgeryList(Rt::SurgeryStatus status, QList<Surgery> *surgeries)
{
	QSqlQuery q;
	q.prepare("SELECT id, surgery_name, surgery_time, room, patient_id,"
		" patient_name, patient_age, patient_gender"
		" FROM r_surgery"
		" WHERE status = ?");
	q.addBindValue(status);
	if (!q.exec())
		return q.lastError().text();

	if (surgeries) {
		Surgery s;
		while (q.next()) {
			s.id = q.value(0).toInt();
			s.surgeryName = q.value(1).toString();
			s.surgeryTime = q.value(2).toDateTime();
			s.surgeryRoom = q.value(3).toString();
			s.patientId = q.value(4).toInt();
			s.patientName = q.value(5).toString();
			s.patientAge = q.value(6).toInt();
			s.patientGender = static_cast<Rt::Gender>(q.value(7).toInt());
			surgeries->append(s);
		}
	}

	return 0;
}

result_t SurgeryDao::getSurgery(int surgeryId, Surgery *surgery, bool excludeBasicInfo/* = true*/)
{
	if (!surgery) return 0;
	surgery->id = surgeryId;

	QSqlQuery q;
	if (!excludeBasicInfo) {
		q.prepare("SELECT surgery_name, surgery_time, room, patient_id,"
			" patient_name, patient_age, patient_gender"
			" FROM r_surgery"
			" WHERE id = ?");
		q.addBindValue(surgeryId);
		if (!q.exec())
			return q.lastError().text();
		if (!q.first())
			return "未找到对应的手术信息";
		
		surgery->surgeryName = q.value(0).toString();
		surgery->surgeryTime = q.value(1).toDateTime();
		surgery->surgeryRoom = q.value(2).toString();
		surgery->patientId = q.value(3).toInt();
		surgery->patientName = q.value(4).toString();
		surgery->patientAge = q.value(5).toInt();
		surgery->patientGender = static_cast<Rt::Gender>(q.value(6).toInt());
	}

	// get detail
	q.prepare("SELECT a.pkg_type_id, a.num, b.name"
		" FROM r_surgery_detail"
		" LEFT JOIN t_package_type ON a.pkg_type_id=b.id"
		" WHERE a.surgery_id = ?");
	q.addBindValue(surgeryId);
	if (!q.exec())
		return q.lastError().text();

	while (q.next()) {
		surgery->detail.append(Surgery::DetailItem(
			q.value(0).toInt(),
			q.value(1).toInt(),
			q.value(2).toString()
		));
	}

	// get bound packages
	q.prepare("SELECT pkg_udi, pkg_name, pkg_cycle"
		" FROM r_surgery_package"
		" WHERE surgery_id = ?");
	q.addBindValue(surgeryId);
	if (!q.exec())
		return q.lastError().text();

	Package pkg;
	while (q.next()) {
		pkg.udi = q.value(0).toString();
		pkg.name = q.value(1).toString();
		pkg.cycle = q.value(2).toInt();
		surgery->packages.append(pkg);
	}

	return 0;
}

result_t SurgeryDao::addSurgery(const Surgery &surgery)
{
	QSqlDatabase db = QSqlDatabase::database();
	db.transaction();

	QSqlQuery q;
	q.prepare("INSERT INTO r_surgery (surgery_name, surgery_time, room, patient_id,"
		" patient_name, patient_age, patient_gender)"
		" VALUES (?, ?, ?, ?, ?, ?, ?)");
	q.addBindValue(surgery.surgeryName);
	q.addBindValue(surgery.surgeryTime);
	q.addBindValue(surgery.surgeryRoom);
	q.addBindValue(surgery.patientId);
	q.addBindValue(surgery.patientName);
	q.addBindValue(surgery.patientAge);
	q.addBindValue(surgery.patientGender);

	if (!q.exec()) {
		db.rollback();
		return q.lastError().text();
	}
	
	// get the last insert surgery id
	int surgeryId = 0;
	if (!q.exec("SELECT MAX(id) FROM r_surgery")) {
		db.rollback();
		return q.lastError().text();
	}
	if (q.first()) {
		surgeryId = q.value(0).toInt();
	}

	// add details (package types and nums)
	if (surgery.detail.isEmpty()) {
		qWarning("You are trying to add a new surgery without package types!");
		return 0;
	}

	// insert new bindings in r_surgery_detail
	QString sql = "INSERT INTO r_surgery_detail (surgery_id, pkg_type_id, num) VALUES";
	QStringList values;
	for each(auto &pt in surgery.detail) {
		QString value = QString(" (%1, %2, %3)").arg(surgeryId).arg(pt.pkgTypeId).arg(pt.pkgNum);
		values << value;
	}
	sql += values.join(',');
	if (!q.exec(sql)) {
		db.rollback();
		return q.lastError().text();
	}

	db.commit();
	return 0;
}
