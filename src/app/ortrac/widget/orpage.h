#pragma once

#include <QWidget>

class OrPage : public QWidget
{
	Q_OBJECT

public:
	enum {Use, Query, Track};
	OrPage(QWidget *parent = Q_NULLPTR);
	~OrPage();

private slots:
	void showPage(int id);
};
