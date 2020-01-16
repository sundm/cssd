# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

class RDAO_EXPORT VerDao
{
public:
	result_t getVersion(int appId, QString *version, QString *hash);

	result_t setVersion(int appId, const QString &version, const QString &hash);
};

