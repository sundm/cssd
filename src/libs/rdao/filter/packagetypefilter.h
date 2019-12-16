#pragma once

#include "../rdao_global.h"

class QVariant;
class QBitArray;
template <class Key, class T>
class QHash;

class RDAO_EXPORT PackageTypeFilter
{
public:
	enum Key {
		Category,
		Name,
		Pinyin,
		SterilizeMethod,
		ForImplants,
		PackTypeId,
		DeptId
	};

	PackageTypeFilter();
	~PackageTypeFilter();

	void addKeyValue(PackageTypeFilter::Key k, const QVariant &v);
	void clear();
	QString clause() const;

private:
	QBitArray *_led;
	QHash<int, QVariant> *_values;
};