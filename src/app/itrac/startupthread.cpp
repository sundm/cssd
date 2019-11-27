#include "startupthread.h"
#include <QFile>
//#include <QDomDocument>
#include <constants.h>
#include <rdao/dbconnection.h>
#include "rdao/dao/PackageDao.h"

void doTest()
{
	PackageDao dao;
	//PackageType pt;
	//result_t res = dao.getPackageType(220001, &pt, true);

	QList<PackageType> pts;
	result_t res = dao.getPackageTypeList(&pts);
	pts.clear();
	res = dao.getPackageTypeList(&pts, 7, 10);
	QString s = res.msg();
}

StartupThread::StartupThread(QObject *parent)
	: QThread(parent), _hasError(true)
{
}

StartupThread::~StartupThread()
{

}

void StartupThread::run()
{
	openDatabase();
	// other things here
}

bool StartupThread::openDatabase()
{
	QString provider;
	QString hostName;
	int port = 0;
	QString dbName;
	QString user;
	QString password;
	int ret = 0;

	provider = "MYSQL";
	hostName = "120.76.128.123";
	port = 3306;
	dbName = "rfid_dev";
	user = "cssd";
	password = "Minxin#dev1";

	//do 
	//{
	//	message(tr("正在连接远程服务……"), false);
	//	msleep(200);

	//	QFile dbf(Constants::SQL_CONF_FILE);
	//	if (!dbf.open(QFile::ReadOnly | QFile::Text))
	//	{
	//		ret = -1;
	//		break;
	//	}

	//	QDomDocument domTree;
	//	if (!domTree.setContent(&dbf))
	//	{
	//		dbf.close();
	//		ret = -2;
	//		break;
	//	}
	//	dbf.close();

	//	QDomElement root = domTree.documentElement();
	//	if (root.tagName() != "sql-connections") {
	//		ret = -2;
	//		break;
	//	}

	//	QString attrDef = root.attribute("default");
	//	if (attrDef.isEmpty()){
	//		ret = -2;
	//		break;
	//	}

	//	QDomNodeList childs = root.elementsByTagName("sql-connection");
	//	int i = 0;
	//	for (; i != childs.count(); ++i){
	//		QDomElement childElement = childs.at(i).toElement();
	//		QString attr = childElement.attribute("name");
	//		if (attr == attrDef) {
	//			provider = childElement.attribute("provider");
	//			hostName = childElement.attribute("host");
	//			port = childElement.attribute("port").toInt();
	//			dbName = childElement.attribute("db");
	//			user = childElement.attribute("username");
	//			password = childElement.attribute("password");
	//			break;
	//		}
	//	}

	//	if (i == childs.count()){
	//		ret = -2;
	//		break;
	//	}

		if (!DBConnection::init(provider, hostName, port, dbName, user, password)){
			ret = -3;
			//break;
		}
	//} while (0);

	//test
		doTest();

	QString err;
	switch(ret){
	case 0:
		return true;
	case -1:
		err = tr("配置文件已丢失");
		break;
	case -2:
		err = tr("无法解析的配置文件");
		break;
	case -3:
		QString dberr(DBConnection::instance()->lastError());
		err = dberr.isEmpty() ? tr("未知") : dberr;
		break;
	}
	emit message(tr("连接远程服务失败，原因：%1").arg(err));
	return false;
}

bool StartupThread::hasError() const
{
	return _hasError;
}
