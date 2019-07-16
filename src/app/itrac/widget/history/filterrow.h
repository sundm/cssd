#pragma once

#include <QWidget>
#include "itracnamespace.h"
#include "core/net/jsonhttpclient.h"

class QHBoxLayout;
class QRadioButton;
class QLineEdit;
class QDateEdit;
class QComboBox;
class QSpinBox;
class Filter;
class DeptEdit;

class FilterRow : public QWidget
{
	Q_OBJECT

public:
	FilterRow(const QString &head, QWidget *parent);
	virtual ~FilterRow();

	virtual void reset() = 0;
	virtual void setCondition2Filter(Filter *) = 0;

protected:
	QHBoxLayout * _layout;
};

class QButtonGroup;
class DateFilterRow : public FilterRow
{
	Q_OBJECT

public:
	DateFilterRow(const QString &head, QWidget *parent);

	void reset() override;
	void setCondition2Filter(Filter *f) override;

private slots:
	void onDateButtonToggled(int, bool);

private:
	QButtonGroup * _dateButtons;
	QRadioButton * _todayButton;
	QRadioButton * _weekButton;
	QRadioButton * _monthButton;
	QRadioButton * _yearButton;
	QRadioButton * _customButton;
	QDateEdit * _startDateEdit;
	QDateEdit * _endDateEdit;
};

class UserFilterRow : public FilterRow
{
	Q_OBJECT
	
public:
	enum USER
	{
		Operator,
		Checker
	};

	UserFilterRow(const QString &head, QWidget *parent, USER type);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	USER _type;
	QLineEdit * _userEdit;
};

class DeptFilterRow : public FilterRow
{
	Q_OBJECT

public:
	DeptFilterRow(const QString &head, QWidget *parent);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	DeptEdit * _deptEdit;
};

class DeviceFilterRow : public FilterRow
{
	Q_OBJECT

public:
	DeviceFilterRow(const QString &head, QWidget *parent, itrac::DeviceType type);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	int _type;
	QComboBox * _deviceBox;
	JsonHttpClient _http;
};

class PackTypeFilterRow : public FilterRow
{
	Q_OBJECT

public:
	PackTypeFilterRow(const QString &head, QWidget *parent);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	QComboBox * _typeBox;
};

class CycleFilterRow : public FilterRow
{
	Q_OBJECT

public:
	CycleFilterRow(const QString &head, QWidget *parent);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	QSpinBox * _cycleBox;
};

class CheckFilterRow : public FilterRow
{
	Q_OBJECT

public:
	CheckFilterRow(const QString &head, QWidget *parent);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
	QComboBox * _checkBox;
};