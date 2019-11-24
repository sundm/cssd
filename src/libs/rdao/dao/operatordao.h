#ifndef OPERATORDAO_H
#define OPERATORDAO_H

#include "rdao_global.h"
#include <rcore/result.h>

class Operator;
class RDAO_EXPORT OperatorDao
{
public:
	result_t login(int id, const QString &pwd, Operator *op = Q_NULLPTR);
	void logout(int id);
	result_t changePassword(int id, const QString &oldPwd, const QString &newPwd);
};

#endif // OPERATORDAO_H
