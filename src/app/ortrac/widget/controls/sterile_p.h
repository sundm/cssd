#pragma once

#include <QGroupBox>
#include "core/datawrapper.h"

namespace Ui {
	class FlatEdit;
}

class SterileInfoGroup : public QGroupBox
{
	Q_OBJECT
public:
	SterileInfoGroup(QWidget *parent = nullptr);
	void updateInfo(const Sterile::TestInfo &);
	QString testId() const;
	void reset();

signals:
	void testIdChanged(const QString &);

private:
	Ui::FlatEdit *_bcEdit;
	Ui::FlatEdit *_deviceEdit;
	Ui::FlatEdit *_cycleEdit;
	Ui::FlatEdit *_startTimeEdit;
};

class CheckItem : public QGroupBox
{
	Q_OBJECT
public:
	CheckItem(const QString &title, int verdict, QWidget *parent = nullptr);
	void reset(int);
	int verdict() const;
private:
	int _verdict;
};

class SterileCheckGroup : public QGroupBox
{
	Q_OBJECT
public:
	SterileCheckGroup(QWidget *parent = nullptr);
	Sterile::Result verdicts() const;
	void reset();

private:
	CheckItem * _phyItem;
	CheckItem * _chemItem;
	CheckItem * _bioItem;
};
