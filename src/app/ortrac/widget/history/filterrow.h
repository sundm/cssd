#pragma once

#include <QWidget>

class QHBoxLayout;
class QRadioButton;
class QLineEdit;
class QDateEdit;
class Filter;
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
	UserFilterRow(const QString &head, QWidget *parent);
	void reset() override;
	void setCondition2Filter(Filter *f) override;

private:
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
	QLineEdit * _deptEdit;
};