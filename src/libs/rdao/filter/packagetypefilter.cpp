#include "packagetypefilter.h"
#include <QBitArray>
#include <QHash>
#include <QVariant>

PackageTypeFilter::PackageTypeFilter() :
	_led(new QBitArray(DeptId + 1)),
	_values(new QHash<int, QVariant>())
{
}

PackageTypeFilter::~PackageTypeFilter()
{
	delete _led;
	delete _values;
}

void PackageTypeFilter::addKeyValue(Key key, const QVariant &val)
{
	_values->insert(key, val);
	_led->setBit(key);
}

void PackageTypeFilter::clear()
{
	_values->clear();
	_led->clear();
}

QString PackageTypeFilter::clause() const
{
	QStringList conds;

	if (_led->testBit(Category))
		conds << QString("category=%1").arg(_values->value(Category).toInt());
	if (_led->testBit(Name))
		conds << QString("name LIKE '%%%1%%'").arg(_values->value(Name).toString());
	if (_led->testBit(Pinyin))
		conds << QString("pinyin LIKE '%%%1%%'").arg(_values->value(Pinyin).toString());
	if (_led->testBit(SterilizeMethod))
		conds << QString("sterilize_type=%1").arg(_values->value(SterilizeMethod).toInt());
	if (_led->testBit(ForImplants))
		conds << QString("for_implants=%1").arg(_values->value(ForImplants).toInt());
	if (_led->testBit(PackTypeId))
		conds << QString("pack_type_id=%1").arg(_values->value(PackTypeId).toInt());
	if (_led->testBit(DeptId))
		conds << QString("dept_id=%1").arg(_values->value(DeptId).toInt());

	return conds.join(" AND ");
}
