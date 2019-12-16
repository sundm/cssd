#pragma once

#include <QDateTime>

namespace DaoUtil {

	QDateTime currentServerTime();

	QString deviceBatchId(int deviceId, int cycleTotal);

	QString udiName(const QString &typeName, int sn);
}