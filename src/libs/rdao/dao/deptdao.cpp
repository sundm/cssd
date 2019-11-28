#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "deptdao.h"
#include "errors.h"

result_t DeptDao::getDept(int id, Department *dept)
{
	QSqlQuery q;
	q.prepare("SELECT name, pinyin, is_surgical, phone, location"
		" FROM t_dept"
		" WHERE id = ?");
	q.addBindValue(id);
	if (!q.exec())
		return q.lastError().text();
	if (!q.first())
		return "未找到对应的科室信息";

	if (dept) {
		dept->id = id;
		dept->name = q.value(0).toString();
		dept->pinyin = q.value(1).toString();
		dept->isSurgical = q.value(2).toBool();
		dept->phone = q.value(3).toString();
		dept->location = q.value(4).toString();
	}

	return 0;
}

result_t DeptDao::getDeptList(QList<Department> *depts)
{
	QSqlQuery q;
	if (!q.exec("SELECT id, name, pinyin, is_surgical, phone, location FROM t_dept"))
		return q.lastError().text();

	if (depts) {
		Department d;
		while (q.next()) {
			d.id = q.value(0).toInt();
			d.name = q.value(1).toString();
			d.pinyin = q.value(2).toString();
			d.isSurgical = q.value(3).toBool();
			d.phone = q.value(4).toString();
			d.location = q.value(5).toString();
			depts->append(d);
		}
	}
	return 0;
}

