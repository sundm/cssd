#pragma once

#include <QWidget>

class TableView;
class QStandardItemModel;
class RecallInfo;

class RecallPage : public QWidget
{
	Q_OBJECT

public:
	enum {Device, Cycle, PackageNum, Reason};
	RecallPage(QWidget *parent = Q_NULLPTR);
	~RecallPage();

private slots:
	void reflash();
	void addEntry();
	void delEntry();

	void recall();

	void onAddRecall(const RecallInfo&);
	void onBarcodeReceviced(const QString& code);
private:

	TableView * _view;
	QStandardItemModel *_model;

	QFont _font;
};
