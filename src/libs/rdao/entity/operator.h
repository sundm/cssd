#pragma once

#include "enums.h"
#include "department.h"
#include <QDateTime>

struct Operator
{
	int id;
	QString name;
	QString phone;
	Rt::Gender gender;
	Rt::Status status;
	Rt::Role role;
	Department dept;
	bool isOnline;
	QDateTime lastLoginTime;

	Operator() :
		id(-1),
		gender(Rt::UnknownGender),
		status(Rt::Normal),
		role(Rt::UnknownRole),
		isOnline(false)
	{ }
};
