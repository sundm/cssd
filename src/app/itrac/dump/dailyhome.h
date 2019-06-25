#ifndef DAILYHOME_H
#define DAILYHOME_H

#include "ui_dailyhome.h"
#include <qscanner/qscanner.h>

class QButtonGroup;
class DailyHome : public QScrollArea, Ui::DailyHome, public Scanable
{
	Q_OBJECT

public:
	explicit DailyHome(QWidget *parent = 0);
	~DailyHome();

signals:
	void flowBtnClicked(int state);

private slots:
	void updateButtonGroupState(QAbstractButton *);

private:
	void handleBarcode(const QString &) override;

private:
	QButtonGroup * _btns;
};

#endif // !DAILYHOME_H
