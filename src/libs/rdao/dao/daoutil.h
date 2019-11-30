#pragma once

#include <QDateTime>

namespace DaoUtil {

	QDateTime currentServerTime();

	QString deviceBatchId(int deviceId, int cycleTotal);

}