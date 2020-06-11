#include <QVariant>
#include <QSqlError>
#include <QCryptographicHash>
#include "debugsqlquery.h"
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
	query.prepare("select * from t_operator where id = ? and pwd = ?");
	query.addBindValue(id);
	query.addBindValue(encodePwd(pwd));
	if (!query.exec())
		return kErrorDbUnreachable;

	if (!query.first())
		return "用户名或密码不正确";

	if (op) {
		op->id = id;
		op->name = query.value(2).toString();
		op->phone = query.value(3).toString();
		op->gender = static_cast<Rt::Gender>(query.value(4).toInt());
		op->status = static_cast<Rt::Status>(query.value(5).toInt());
		op->role = static_cast<Rt::Role>(query.value(6).toInt());
		op->dept.id = query.value(7).toInt();
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
		return kErrorDbUnreachable;
	if (1 != query.numRowsAffected())
		return "原密码不正确";

	return 0;
}

result_t OperatorDao::getOperatorList(QMap<int, QString> *opList)
{
	opList->clear();

	QSqlQuery query;
	query.prepare("select id,name FROM t_operator");
	if (!query.exec())
		return kErrorDbUnreachable;

	while (query.next())
	{
		opList->insert(query.value(0).toInt(), query.value(1).toString());
	}

	return 0;
}