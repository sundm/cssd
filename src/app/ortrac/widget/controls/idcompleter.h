#pragma once

#include <QCompleter>

class QStandardItemModel;

class IdCompleter : public QCompleter
{
	Q_OBJECT
	
public:
	IdCompleter(QObject *parent = nullptr);

	int id(const QModelIndex&) const;
	QString name(const QModelIndex&) const;

signals:
	void error(const QString &);

protected:
	QStandardItemModel * _model;
};

class DeptCompleter : public IdCompleter
{
	Q_OBJECT
public:
	DeptCompleter(QObject *parent = nullptr);
	void load();
};

class PackageCompleter : public IdCompleter
{
	Q_OBJECT
public:
	PackageCompleter(QObject *parent = nullptr);
	void loadForDepartment(int deptId);
	void loadForCategory(const QString &category);

private:
	void loadInternal(const QByteArray &data);
};
