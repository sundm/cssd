#include "user.h"
#include "application.h"

namespace Core {

User::User()
	: id(-1)
	, deptId(-1)
	, role(-1)
	, gender(Unknown)
{ }

User & currentUser() {
	return Core::app()->currentUser();
}

}
