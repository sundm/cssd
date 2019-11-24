#ifndef OPERATOR_H
#define OPERATOR_H

#include "rdao_global.h"
#include <QString>
#include <QDateTime>

struct Operator
{
	enum Gender { Male, Female, Unknown };
	enum Status { Normal, Frozen, Deleted };
	enum Role { Admin = 1, HeadNurse, OrUser, ClinicUser, CssdUser, UnknownRole };

	int id;
	QString name;
	QString phone;
	Operator::Gender gender;
	Operator::Status status;
	Operator::Role role;
	//Department dept;
	bool isOnline;
	QDateTime lastLoginTime;

	Operator() :
		id(-1),
		gender(Gender::Unknown),
		status(Status::Normal),
		role(Role::UnknownRole),
		isOnline(false)
	{ }
};

#endif // OPERATOR_H
