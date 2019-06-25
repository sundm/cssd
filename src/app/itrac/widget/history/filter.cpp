#include "filter.h"

Filter::Filter()
{
}

QVariant Filter::condition(FilterFlag flag)
{
	return _conditions.value(flag, QVariant());
}

void Filter::setCondition(FilterFlag flag, QVariant value)
{
	_conditions[flag] = value;
}
