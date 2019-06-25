#pragma once

#include <QTreeView>

class QStandardItemModel;
class PlateView : public QTreeView
{
	Q_OBJECT

public:
	PlateView(QWidget *parent = nullptr);
	bool hasPlate(int) const;
	void addPlate(int);
	QVariantList plates() const;
	void clear();

private:
	QStandardItemModel * _model;
};


class PackPlateView : public QTreeView
{
	Q_OBJECT

public:
	PackPlateView(QWidget *parent = nullptr);
	void addPlate(int);
	QVariantList plates() const;
	void doPack(int, int);

//signals:
//	void packed(const std::list<std::list<std::string>> &);

private:
	enum {
		Package,
		PackType,
		Barcode,
		SterileDate,
		ExpireDate,
	};

	QList<int> getSelectedPackages(QVariantList &) const;

	QStandardItemModel * _model;
};
