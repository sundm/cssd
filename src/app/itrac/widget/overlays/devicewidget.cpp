#include "devicewidget.h"

#include "core/assets.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/buttons.h"
#include "xnotifier.h"
#include "widget/controls/combos.h"
#include <QtWidgets/QtWidgets>

DeviceItem::DeviceItem(Device *device, QWidget *parent)
	: QGroupBox(parent)
	, _device(device)
	, _icon(new Ui::CheckableImageLabel)
	, _title(new QLabel(_device->name)) //_title = new Ui::Title(_device->name)
{
	_icon->setAlignment(Qt::AlignCenter);
	_icon->setCursor(Qt::PointingHandCursor);
	_icon->installEventFilter(this);
	
	_title->setWordWrap(true);

	// limit the maximum size to sizeHint
	setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
}

int DeviceItem::id() const {
	return _device->id;
}

int DeviceItem::cycle() const {
	return _device->cycleToday;
}

QString DeviceItem::name() const {
	return _device->name;
}

bool DeviceItem::eventFilter(QObject *object, QEvent *e) {
	if (object == _icon && e->type() == QEvent::MouseButtonRelease) {
		qDebug() << "selected";
		if (_device->isIdle() && !_icon->isChecked()) {
			setSelected(true);
			emit selected(this);
		}
	}
	return QGroupBox::eventFilter(object, e);
}

/* class WasherItem */
WasherItem::WasherItem(Device *device, QWidget *parent)
	: DeviceItem(device, parent)
{
	QGridLayout *layout = new QGridLayout(this);

	_icon->setPixmap(
		QPixmap(_device->isIdle() ? ":/res/washer.png" : ":/res/washer-busy.png"));
	layout->addWidget(_icon, 0, 0, 3, 1);
	
	layout->addWidget(_title, 0, 1);

	Ui::Description *cycle =
		new Ui::Description(QString("今日锅次: %1").arg(_device->cycleToday));
	layout->addWidget(cycle, 1, 1);

	_comboBox = new ProgramComboBox(_device->id);
	layout->addWidget(_comboBox, 3, 0, 1, 2);

	_button = new QPushButton("完成");
	connect(_button, SIGNAL(clicked()), this, SLOT(stop()));
	layout->addWidget(_button, 4, 0, 1, 2);

	if (_device->isIdle()) {
		_button->hide();
	}
	else {
		_comboBox->hide();
	}

	setSelected(false);
}

int WasherItem::programId() const {
	return _comboBox->currentProgramId();
}

void WasherItem::setSelected(bool b) {
	_icon->setChecked(b);
	_comboBox->setEnabled(b);
	if (b) {
		_comboBox->updateEntry();
	}
}

void WasherItem::setIdle() {
	_icon->setPixmap(QPixmap(":/res/washer.png"));
	_comboBox->show();
	_button->hide();
	_device->state = Device::Idle;
}

void WasherItem::stop() {
	QByteArray data("{\"device_id\":");
	data.append(QString::number(_device->id)).append('}');
	_http.post(url(PATH_DEVICE_STOP), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("结束程序出错: ").append(resp.errorString()));
			return;
		}
		setIdle();
	});
}


/* class DeviceArea */
DeviceArea::DeviceArea(QWidget *parent)
	:QScrollArea(parent), _content(new QWidget), _curItem(nullptr)
{
	setWidgetResizable(true);

	QHBoxLayout *layout = new QHBoxLayout(_content);
	layout->setSizeConstraint(QLayout::SetMinimumSize);
	//layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
	setWidget(_content);
}

void DeviceArea::addDeviceItem(DeviceItem *item) {
	QHBoxLayout *layout = static_cast<QHBoxLayout*>(_content->layout());
	layout->addWidget(item, Qt::AlignTop);
	connect(item, &DeviceItem::selected, this, &DeviceArea::onItemSelected);
}

void DeviceArea::load(itrac::DeviceType type)
{
	clear();

	QString data = QString("{\"device_type\":\"000%1\"}").arg((itrac::DeviceType::Washer == type) ? 1: 2);

	post(url(PATH_DEVICE_SEARCH), QByteArray().append(data), [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> devices = resp.getAsList("devices");
		for(auto &device: devices) {
			QVariantMap map = device.toMap();
			int state = Device::tranlateState(map["is_forbidden"].toString());
			if (state == Device::Disabled) continue;

			Device *d = new Device;
			d->name = map["device_name"].toString();
			d->id = map["device_id"].toInt();
			d->cycleToday = map["cycle"].toInt();
			d->cycleSum = map["total_cycle"].toInt();
			d->state = state;
			addDeviceItem(new WasherItem(d));
		}
		// call this whenever the sizeHint or sizePolicy have changed,
		// otherwise the layout doesn't know the minimumSizeHint has changed.
		updateGeometry();
	});
}

QSize DeviceArea::minimumSizeHint() const {

	QSize size = QScrollArea::minimumSizeHint();
	return QSize(size.width() + 2,
		_content->minimumSizeHint().height() + 2);
	//return QScrollArea::minimumSizeHint();
}

void DeviceArea::clear() {
	QLayout *layout = _content->layout();
	QLayoutItem *child;
	while ((child = layout->takeAt(0)) != 0) {
		delete child;
	}
}

void DeviceArea::onItemSelected(DeviceItem *item) {
	if (_curItem) {
		_curItem->setSelected(false);
	}
	_curItem = item;
}
