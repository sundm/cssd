#ifndef FILTER_H
#define FILTER_H

#include <QHash>
#include <QVariant>

enum class FilterFlag : int {
	StartDate,
	EndDate,
	Operator,  // id
	Department, // id
	cycle
};

inline uint qHash(const FilterFlag &key, uint seed = 0)
{
	return qHash(static_cast<int>(key), seed);
}

class Filter
{
public:
	Filter();

	QVariant condition(FilterFlag flag);
	void setCondition(FilterFlag flag, QVariant value);

private:
	QHash<FilterFlag, QVariant> _conditions;
};

#endif //FILTER_H
