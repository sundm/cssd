#pragma once

#include <QString>
#include <functional>
#include "rdao/entity/enums.h"

class JsonHttpClient;

struct Asset {
	int id;
	QString name;
	QString err;
};

struct Plate : public Asset {
	bool idle;
	static void fetchOnce(const QString &id, const std::function<void(Plate*)> &fn);
};
