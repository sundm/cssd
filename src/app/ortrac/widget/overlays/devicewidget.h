#pragma once

#include <QScrollArea>
#include <QGroupBox>

struct Device;
class ProgramComboBox;
class QLabel;
class QPushButton;

namespace Ui {
	class CheckableImageLabel;
	//class PrimaryButton;
}

class DeviceItem : public QGroupBox
{
	Q_OBJECT
public:
	DeviceItem(Device *device = nullptr, QWidget *parent = nullptr);
	QString name() const;
	int id() const;
	int cycle() const;

	virtual int programId() const = 0;
	virtual void setSelected(bool) = 0;
	virtual void setIdle() = 0;

signals:
	void selected(DeviceItem *);

protected:
	bool eventFilter(QObject *object, QEvent *e) override;

	std::unique_ptr<Device> _device;
	Ui::CheckableImageLabel *_icon;
	QLabel *_title;
};


class WasherItem : public DeviceItem
{
	Q_OBJECT
public:
	WasherItem(Device *device = nullptr, QWidget *parent = nullptr);
	int programId() const override;
	void setSelected(bool) override;
	void setIdle() override;

private slots:
	void stop();

private:
	ProgramComboBox *_comboBox;
	QPushButton *_button;
};


class DeviceArea : public QScrollArea
{
	Q_OBJECT
public:
	enum Type { Washer, Sterilizer }; // todo, use global enum

	DeviceArea(QWidget *parent = nullptr);
	void addDeviceItem(DeviceItem *);
	DeviceItem *currentItem() const { return _curItem; };
	void load(DeviceArea::Type type);

public:
	QSize minimumSizeHint() const override;

private:
	void clear();
	void onItemSelected(DeviceItem *);

	QWidget * _content;
	DeviceItem *_curItem;
};

