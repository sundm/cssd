#pragma once

#include <QWidget>

class QTabWidget;
class AssetPage : public QWidget
{
	Q_OBJECT

public:
	AssetPage(QWidget *parent = Q_NULLPTR);

private slots:
	void showPage(int id);
	void closePage(int index);

private:
	QHash<int, QWidget*> _pageMap;
	QTabWidget *_tabWidget;
};
