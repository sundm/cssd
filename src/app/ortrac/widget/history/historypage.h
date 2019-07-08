#pragma once

#include <QWidget>

/**
 * Classes for different history pages.
 * We use polymorphism here, maybe a template class would be better.
 */

class FilterWidget;
class FilterGroup;
class Filter;
class QStandardItemModel;
class QTableView;
class QPushButton;
class QPaginationWidget;
class HistoryPage : public QWidget
{
	Q_OBJECT

public:
	HistoryPage(QWidget *parent = Q_NULLPTR);
	~HistoryPage();

	void setVisibleCount(int count) { _visibleCount = count; }

protected:
	void onFilterChanged(Filter &);
	virtual FilterGroup *createFilterGroup() = 0;
	virtual void doSearch(int page = 1) = 0;

protected slots:
	void showFilterPopup(bool);

protected:
	QPushButton *_filterButton;
	QPaginationWidget *_paginator;
	QTableView *_view;
	FilterWidget *_filterWidget;
	Filter *_filter;
	QStandardItemModel * _historyModel;
	int _visibleCount;
};

class RecycleHistoryPage : public HistoryPage
{
	Q_OBJECT

public:
	RecycleHistoryPage(QWidget *parent = Q_NULLPTR);

protected:
	FilterGroup * createFilterGroup() override;
	void doSearch(int page = 1) override;
};

class WashHistoryPage : public HistoryPage
{
	Q_OBJECT

public:
	WashHistoryPage(QWidget *parent = Q_NULLPTR);

protected:
	FilterGroup * createFilterGroup() override;
	void doSearch(int page = 1) override;
};

class PackHistoryPage : public HistoryPage
{
	Q_OBJECT

public:
	PackHistoryPage(QWidget *parent = Q_NULLPTR);

protected:
	FilterGroup * createFilterGroup() override;
	void doSearch(int page = 1) override;
};

class SterileHistoryPage : public HistoryPage
{
	Q_OBJECT

public:
	SterileHistoryPage(QWidget *parent = Q_NULLPTR);

protected:
	FilterGroup * createFilterGroup() override;
	void doSearch(int page = 1) override;
};

class IssueHistoryPage : public HistoryPage
{
	Q_OBJECT

public:
	IssueHistoryPage(QWidget *parent = Q_NULLPTR);

protected:
	FilterGroup * createFilterGroup() override;
	void doSearch(int page = 1) override;
};

