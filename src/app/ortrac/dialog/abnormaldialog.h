#pragma once

#include <QDialog>
#include "core/net/jsonhttpclient.h"
#include <qscanner/qscanner.h>
#include <QGroupBox>

class TableView;
class QStandardItemModel;
class WaitingSpinner;

struct Result {
	Result();

	int wet;
	int inside;
	static QString toString(int);
};

class CheckItem : public QGroupBox
{
	Q_OBJECT
public:
	CheckItem(const QString &title, QWidget *parent = nullptr);
	void reset();
	int verdict() const;
private:
	int _verdict;
};

class SterileCheckGroup : public QGroupBox
{
	Q_OBJECT
public:
	SterileCheckGroup(QWidget *parent = nullptr);
	Result verdicts() const;
	void reset();

private:
	CheckItem * _wetItem;
	CheckItem * _insideItem;
};

class AbnormalDialog : public QDialog, public Scanable, public JsonHttpClient
{
	Q_OBJECT

public:
	AbnormalDialog(QWidget *parent);
	~AbnormalDialog();

	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void removeEntry();

private:
	void accept();
	void loadSource(const QString &id);

	TableView * _view;
	QStandardItemModel * _model;

	WaitingSpinner *_waiter;

	SterileCheckGroup *_checkGroup;
};
