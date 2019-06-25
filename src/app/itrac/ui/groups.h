#pragma once

#include <QGroupBox>

class QFormLayout;
class FormGroup : public QGroupBox
{
	Q_OBJECT

public:
	FormGroup(QWidget *parent = Q_NULLPTR);
	void addRow(const QString &lable, QWidget *w);

private:
	QFormLayout * _layout;
};
