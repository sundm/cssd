#include "daoutil.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

using namespace DaoUtil;

QDateTime currentServerTime()
{
	QSqlQuery q;
	if (!q.exec("SELECT now()"))
		return QDateTime();
	if (q.first())
		return q.value(0).toDateTime();
	return QDateTime();
}

QString DaoUtil::deviceBatchId(int deviceId, int cycleTotal)
{
	// TODO: cycleTotal may be exceeds MAXINT
	return QString("%1%2").arg(deviceId).arg(cycleTotal, 8, 10, QChar('0'));
}

QString DaoUtil::udiName(const QString &typeName, int sn)
{
	return typeName + '#' + QString::number(sn);
}
