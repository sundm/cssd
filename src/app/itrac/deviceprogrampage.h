#pragma once

#include <QWidget>

class QStandardItemModel;
class QComboBox;
class TableView;
class DeviceProgramPage : public QWidget
{
	Q_OBJECT

public:
	DeviceProgramPage(QWidget *parent = Q_NULLPTR);

private slots:
	void onFilterChanged(int index);
	void showProgramItemContextMenu(const QPoint&);
	void refresh();
	void add();
	void modify();

private:
	void initProgramView();
	void updateProgramView(const QString& deviceType = QString());

private:
	QComboBox *_filterComboBox;
	TableView *_view;
	QStandardItemModel * _programModel;
};
