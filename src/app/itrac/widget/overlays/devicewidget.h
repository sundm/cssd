#pragma once

#include <QScrollArea>
#include <QGroupBox>
#include "itracnamespace.h"
#include "core/net/jsonhttpclient.h"

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
	int sterilize_type() const;
	virtual int programId() const = 0;
	virtual bool isRunning() const = 0;
	virtual void setSelected(bool) = 0;
	virtual void setIdle() = 0;
	virtual void setRunning() = 0;

signals:
	void selected(DeviceItem *);

protected:
	bool eventFilter(QObject *object, QEvent *e) override;
	std::unique_ptr<Device> _device;
	Ui::CheckableImageLabel *_icon;
	QLabel *_title;
	JsonHttpClient _http;
};


class WasherItem : public DeviceItem
{
	Q_OBJECT
public:
	WasherItem(Device *device = nullptr, QWidget *parent = nullptr);
	int programId() const override;
	void setSelected(bool) override;
	void setIdle() override;
	void setRunning() override;
	bool isRunning() const override;

private slots:
	void stop();

private:
	ProgramComboBox *_comboBox;
	QPushButton *_button;
};


class DeviceArea : public QScrollArea, public JsonHttpClient
{
	Q_OBJECT
public:

	DeviceArea(QWidget *parent = nullptr);
	void addDeviceItem(DeviceItem *);
	DeviceItem *currentItem() const { return _curItem; };
	void load(itrac::DeviceType type, bool isHigh = false);
	void scanDevice(const int &code);

public:
	QSize minimumSizeHint() const override;

private:
	void clear();
	void onItemSelected(DeviceItem *);

	QWidget * _content;
	DeviceItem *_curItem;
	QList<DeviceItem *> _items;

	bool _isHigh;
};