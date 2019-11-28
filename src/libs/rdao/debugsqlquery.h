#pragma once

#include <QSqlQuery>

class DebugSqlQuery : public QSqlQuery
{
public:
	DebugSqlQuery(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
	~DebugSqlQuery();

	bool exec(const QString &query);
	bool exec(); 
	bool execBatch(BatchExecutionMode mode = ValuesAsRows);

private:
	QString getLastExecutedQuery();
};


#ifdef QT_DEBUG
#define QSqlQuery DebugSqlQuery
#else
#define QSqlQuery QSqlQuery
#endif
