#include <QDateTime>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QCryptographicHash>
#include "operatordao.h"
#include "errors.h"
#include "entity/operator.h"

namespace {
	QString encodePwd(const QString &pwd)
	{
		QByteArray sha1 = QCryptographicHash::hash(pwd.toLocal8Bit(), QCryptographicHash::Sha1);
		return QString(sha1.toHex());
	}
}

result_t OperatorDao::login(int id, const QString &pwd, Operator *op/* = Q_NULLPTR*/)
{
	QSqlQuery query;
	query.prepare("select * from t_operator where id = ? and psw = ?");
	query.addBindValue(id);
	query.addBindValue(encodePwd(pwd));
	if (!query.exec())
		return kErrorQueryExec;

	if (!query.first())
		return "用户名或密码不正确";

	if (op) {
		op->id = id;
		op->name = query.value(2).toString();
		op->phone = query.value(3).toString();
		op->gender = static_cast<Operator::Gender>(query.value(4).toInt());
		op->status = static_cast<Operator::Status>(query.value(5).toInt());
		op->role = static_cast<Operator::Role>(query.value(6).toInt());
		op->isOnline = query.value(8).toBool();
		op->lastLoginTime = query.value(9).toDateTime(); // TODO
	}

	return 0;
}

void OperatorDao::logout(int id)
{
	// TODO
}

result_t OperatorDao::changePassword(int id, const QString &oldPwd, const QString &newPwd)
{
	QSqlQuery query;
	query.prepare("UPDATE t_operator SET psw = ? WHERE id = ? and psw = ?");
	query.addBindValue(encodePwd(newPwd));
	query.addBindValue(id);
	query.addBindValue(encodePwd(oldPwd));
	if (!query.exec())
		return kErrorQueryExec;
	if (1 != query.numRowsAffected())
		return "原密码不正确";

	return 0;
}
