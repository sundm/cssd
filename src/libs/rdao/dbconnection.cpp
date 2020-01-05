#include "dbconnection.h"
#include <QSqlError>
#include <QtDebug>
//#include <VSMW32API.h>

//QString decodePassword(const QString &password)
//{
//	QByteArray &hex = QByteArray::fromBase64(password.toLocal8Bit());
//
//	unsigned char pk[16] = {0};
//	unsigned resultlen = 0;
//	unsigned char result[260] = {0};
//	VSM_TripleDES(VSM_DECRYPT, VSM_ECB, VSM_PAD_ISO_2, pk, 0,
//		hex.length(), reinterpret_cast<const unsigned char*>(hex.constData()), &resultlen, result);
//
//	if(0 == resultlen || result[result[0]+1] != 0x80)
//		return QString();
//
//	return QString(QByteArray((char*)result+1, result[0]));
//}

DBConnection* DBConnection::_conn = NULL;

bool DBConnection::init(const QString &provider,
                        const QString &hostName,
                        int port,
                        const QString &dbName,
                        const QString &user,
                        const QString &password)
{
    if(!_conn) {
        _conn = new DBConnection(provider, hostName, port ,dbName, user, password);
    }

    return _conn->_db.isOpen();
}

void DBConnection::cleanup()
{
    if (_conn) {
        delete _conn;
        _conn = NULL;
    }
}

DBConnection* DBConnection::instance()
{
    return _conn;
}

QString DBConnection::lastError() const
{
    return _db.lastError().text();
}

DBConnection::DBConnection(const QString &provider,
                           const QString &hostName,
                           int port,
                           const QString &dbName,
                           const QString &user,
                           const QString &password)
{
    _db = QSqlDatabase::addDatabase(QString('Q').append(provider));
    _db.setPort(port);
    _db.setHostName(hostName);
    _db.setDatabaseName(dbName);
    _db.setUserName(user);
    //_db.setPassword(decodePassword(password));
    _db.setPassword(password);
	if (provider == "MYSQL") {
		_db.setConnectOptions("MYSQL_OPT_RECONNECT=1");
	}
	
    if (_db.open()) {
        qDebug()<< "database connected.";
    }else{
        qDebug()<< "failed to connect database!" <<_db.lastError().text();
    }
}

DBConnection::~DBConnection()
{
	QString connectionName = _db.connectionName();
    if (_db.isOpen()){
        _db.close();
    }
	_db = QSqlDatabase(); /* resign the _db to make the reference count-1,
						     otherwise QSqlDatabase::removeData cannot clear all connections,
						     and a warning says "QSqlDatabasePrivate::removeDatabase: 
						     connection 'qt_sql_default_connection' is still in use, all
						     queries will cease to work" will be raised.*/
	QSqlDatabase::removeDatabase(connectionName);
}
