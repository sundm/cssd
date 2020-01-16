#pragma once

#include "rdao/entity/recall.h"
#include <QWidget>

class TableView;
class QStandardItemModel;
struct RangedSterBatchInfo;

class RecallPage : public QWidget
{
	Q_OBJECT

public:
	enum {DeviceName, BatchId, Date, Cycle, PackageNum};
	RecallPage(QWidget *parent = Q_NULLPTR);
	~RecallPage();

private slots:
	void reflash();
	void addEntry();
	//void delEntry();
	//void onAddRecall(const RecallInfo&);

	void recall();
	void onBarcodeReceviced(const QString& code);
private:

	TableView * _view;
	QStandardItemModel *_model;

	QFont _font;

	QList<RangedSterBatchInfo> _recalls;
};
