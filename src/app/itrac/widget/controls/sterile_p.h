#pragma once

#include <QGroupBox>
#include "core/datawrapper.h"
#include "rdao/entity/device.h"

namespace Ui {
	class FlatEdit;
}

class QCheckBox;

class SterileInfoGroup : public QGroupBox
{
	Q_OBJECT
public:
	SterileInfoGroup(QWidget *parent = nullptr);
	void updateInfo(const DeviceBatchInfo &);
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
	CheckItem(const QString &title, Rt::SterilizeVerdict verdict, bool involved = false, QWidget *parent = nullptr);
	void reset(Rt::SterilizeVerdict, bool);
	Rt::SterilizeVerdict verdict() const;
	bool disabled() const { return _disabled; };
private:
	Rt::SterilizeVerdict _verdict;
	bool _disabled;
};

class SterileCheckGroup : public QGroupBox
{
	Q_OBJECT
public:
	SterileCheckGroup(QWidget *parent = nullptr);
	void updateInfo(const SterilizeResult &);
	Sterile::Result verdicts() const;
	void reset();
	bool isFirst();
private:
	CheckItem * _phyItem;
	CheckItem * _chemItem;
	CheckItem * _bioItem;
	QCheckBox * _lostLabelItem;

	bool _first;
};
