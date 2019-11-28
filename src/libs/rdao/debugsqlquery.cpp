#include "debugsqlquery.h"
#include <QSqlField>
#include <QSqlDriver>
#include <QDebug>

#ifdef QSqlQuery
#undef QSqlQuery
#endif

DebugSqlQuery::DebugSqlQuery(const QString& query, QSqlDatabase db)
	:QSqlQuery(query, db)
{
}

DebugSqlQuery::~DebugSqlQuery()
{
}

bool DebugSqlQuery::exec(const QString &query)
{
	bool res = QSqlQuery::exec(query);
	qDebug() << query;
	return res;
}

bool DebugSqlQuery::exec()
{
	bool res = QSqlQuery::exec();
	qDebug() << getLastExecutedQuery();
	return res;
}

bool DebugSqlQuery::execBatch(BatchExecutionMode mode /*= ValuesAsRows*/)
{
	bool res = QSqlQuery::execBatch(mode);
	qDebug() << getLastExecutedQuery();
	return res;
}

QString DebugSqlQuery::getLastExecutedQuery()
{
	QString sql = executedQuery();
	const int nbBindValues = boundValues().size();

	for (int i = 0, j = 0; j < nbBindValues; ++j)
	{
		i = sql.indexOf(QLatin1Char('?'), i);
		if (i <= 0)
		{
			break;
		}
		const QVariant &var = boundValue(j);
		QSqlField field(QLatin1String(""), var.type());
		if (var.isNull())
		{
			field.clear();
		}
		else
		{
			field.setValue(var);
		}
		QString formatV = driver()->formatValue(field);
		sql.replace(i, 1, formatV);
		i += formatV.length();
	}

	return sql;
}
