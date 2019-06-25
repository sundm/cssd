#pragma once

#include <QObject>

class QAbstractItemModel;

class ModelCache : public QObject
{
	Q_OBJECT

public:
	enum Type{
		Department,
		Device,
		Plate,
		EOT
	};

	ModelCache(QObject *parent = nullptr);
	
	void update();
	QAbstractItemModel *get(ModelCache::Type type);
	
signals:
	void ready();
	void error(const QString &);

private:
	void updateDeptModel();

	using _Model = std::unique_ptr<QAbstractItemModel>;
	std::vector<_Model> _models;
};
