#ifndef DBCORE_H
#define DBCORE_H

#include "rdao_global.h"
#include <QtSql/QSqlDatabase>

class RDAO_EXPORT DBConnection
{
public:
    static bool init(const QString &provider,
              const QString &hostName,
              int port,
              const QString &dbName,
              const QString &user,
              const QString &password);

    static void cleanup();

    static DBConnection* instance();

    QString lastError() const;

private:
    DBConnection(const QString &provider,
                 const QString &hostName,
                 int port,
                 const QString &dbName,
                 const QString &user,
                 const QString &password);
    ~DBConnection();

    static DBConnection *_conn;
    QSqlDatabase _db;
};

#endif // DBCORE_H
