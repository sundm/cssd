#include <QVariant>
#include <QSqlError>
#include "debugsqlquery.h"
#include "verdao.h"
#include "errors.h"

result_t VerDao::getVersion(int appId, QString *version, QString *hash)
{
	QSqlQuery q;
	q.prepare("SELECT version, hash"
		" FROM t_version"
		" WHERE id = ?");
	q.addBindValue(appId);
	if (!q.exec())
		return q.lastError().text();
	if (!q.first())
		return "未找到对应的版本信息";

	*version = q.value(0).toString();
	*hash = q.value(1).toString();

	return 0;
}

result_t VerDao::setVersion(int appId, const QString &version, const QString &hash)
{
	QSqlQuery q;
	q.prepare("SELECT version, hash"
		" FROM t_version"
		" WHERE id = ?");
	q.addBindValue(appId);
	if (!q.exec())
		return q.lastError().text();

	if (!q.first())
	{
		q.prepare("INSERT INTO t_version (id, version, hash)"
			" VALUES (?, ?, ?)");
		q.addBindValue(appId);
		q.addBindValue(version);
		q.addBindValue(hash);
	}
	else
	{
		q.prepare("UPDATE t_version SET version = ?, hash = ?"
			" WHERE id = ?");
		q.addBindValue(version);
		q.addBindValue(hash);
		q.addBindValue(appId);
	}

	if (!q.exec())
		return q.lastError().text();

	return 0;
}

