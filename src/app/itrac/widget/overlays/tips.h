#pragma once

#include <QGroupBox>

class QVBoxLayout;
class QAbstractButton;

class Tip : public QGroupBox
{
	Q_OBJECT

public:
	Tip(QWidget *parent = nullptr);
	Tip(const QString &text, QWidget *parent = nullptr);
	~Tip();

	void addButton(QAbstractButton *);
	void addQr();
private:
	void setupUi(const QString &text = QString());
	QVBoxLayout * _layout;
};
