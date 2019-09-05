#pragma once

#include <qstring>

namespace Core {

	struct User
	{
		enum Gender { Male, Female, Unknown };
		enum Status { Normal, Quit };
		enum Role {Admin = 1, HeadNurse, OrUser, ClinicUser, CssdUser};
		User();

		int id;
		int account;
		int deptId;
		int role;
		Gender gender;
		QString name;
		QString deptName;
		QString phone;
		QString loginTime;
	};

	User &currentUser();
}
