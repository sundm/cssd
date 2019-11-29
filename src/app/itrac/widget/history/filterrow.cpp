#include <QHBoxLayout>
#include <QDateEdit>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QComboBox>
#include <QSpinBox>
#include <QSpacerItem>
#include <QButtonGroup>
#include <xernel/xtimescope.h>
#include "core/net/url.h"
#include "core/assets.h"
#include "xnotifier.h"
#include "filterrow.h"
#include "filter.h"
#include "idedit.h"
#include "rdao/dao/devicedao.h"

FilterRow::FilterRow(const QString &head, QWidget *parent)
	: QWidget(parent), _layout(new QHBoxLayout)
{
	_layout->addWidget(new QLabel(head));
	_layout->addStretch();
}

FilterRow::~FilterRow()
{
}

DateFilterRow::DateFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_todayButton = new QRadioButton("今天", this);
	_weekButton = new QRadioButton("本周", this);
	_monthButton = new QRadioButton("本月", this);
	_yearButton = new QRadioButton("今年", this);
	_customButton = new QRadioButton("自定义", this);

	// add radio buttons into group
	_dateButtons = new QButtonGroup(this);
	_dateButtons->addButton(_todayButton, XPeriod::Today);
	_dateButtons->addButton(_weekButton, XPeriod::ThisWeek);
	_dateButtons->addButton(_monthButton, XPeriod::ThisMonth);
	_dateButtons->addButton(_yearButton, XPeriod::ThisYear);
	_dateButtons->addButton(_customButton, XPeriod::Custom);
	connect(_dateButtons, static_cast<void(QButtonGroup::*)(int, bool)>(&QButtonGroup::buttonToggled),
		this, &DateFilterRow::onDateButtonToggled);

	_layout->addWidget(_todayButton);
	_layout->addWidget(_weekButton);
	_layout->addWidget(_monthButton);
	_layout->addWidget(_yearButton);
	_layout->addWidget(_customButton);

	QDate today = QDate::currentDate();
	_startDateEdit = new QDateEdit(today, this);
	_startDateEdit->setCalendarPopup(true);
	_startDateEdit->setMaximumDate(today);
	_startDateEdit->setEnabled(false);
	_layout->addWidget(_startDateEdit);

	_layout->addWidget(new QLabel("-", this));

	_endDateEdit = new QDateEdit(today, this);
	_endDateEdit->setCalendarPopup(true);
	_endDateEdit->setMaximumDate(today);
	_endDateEdit->setEnabled(false);
	_layout->addWidget(_endDateEdit);

	_layout->addStretch(1);
	_layout->setSpacing(13);
	setLayout(_layout);

	_todayButton->setChecked(true);
}

void DateFilterRow::reset()
{
	_todayButton->setChecked(true);
}

void DateFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::StartDate, _startDateEdit->date());
	f->setCondition(FilterFlag::EndDate, _endDateEdit->date());
}

void DateFilterRow::onDateButtonToggled(int id, bool checked)
{
	if (XPeriod::Custom == id) {
		_startDateEdit->setEnabled(checked);
		_endDateEdit->setEnabled(checked);
		return;
	}

	// for other buttons, ignore their unchecked signals.
	if (!checked) return;

	XDateScope ds(static_cast<XPeriod>(id));
	_startDateEdit->setDate(ds.from);
	_endDateEdit->setDate(ds.to);
}

UserFilterRow::UserFilterRow(const QString &head, QWidget *parent, USER type)
	:FilterRow(head, parent)
{
	_userBox = new QComboBox(this);
	_userBox->setFixedWidth(150);
	_layout->addWidget(_userBox);

	_layout->addStretch(1);
	setLayout(_layout);

	_type = type;

	reset();
}

void UserFilterRow::reset()
{
	_userBox->clear();

	QString data = QString("{}");

	_http.post(url(PATH_USER_SEARCH), QByteArray().append(data), [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取用户列表: ").append(resp.errorString()));
			return;
		}

		_userBox->addItem(QString("不限"), 0);

		QList<QVariant> devices = resp.getAsList("user_list");
		for (auto &device : devices) {
			QVariantMap map = device.toMap();
			int role = map["role_id"].toInt();
			if (5 == role || 1 == role ||  2 == role)
			{
				_userBox->addItem(map["name"].toString(), map["operator_id"].toInt());
			}
			
		}

		updateGeometry();
	});
}

void UserFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(_type == USER::Operator ? FilterFlag::Operator : FilterFlag::Auditor, _userBox->currentData().toInt());
}

DeptFilterRow::DeptFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_deptEdit = new DeptEdit(this);
	_layout->addWidget(_deptEdit);

	_layout->addStretch(1);
	setLayout(_layout);

	_deptEdit->load(DeptEdit::DEPTYPE::ALL);
}

void DeptFilterRow::reset()
{
	_deptEdit->reset();
}

void DeptFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::Department, _deptEdit->currentId());
}

DeviceFilterRow::DeviceFilterRow(const QString &head, QWidget *parent, itrac::DeviceType type)
	:FilterRow(head, parent)
{
	_deviceBox = new QComboBox(this);
	_deviceBox->setFixedWidth(200);
	_layout->addWidget(_deviceBox);

	_layout->addStretch(1);
	setLayout(_layout);

	_type = type;

	reset();
}

void DeviceFilterRow::reset()
{
	_deviceBox->clear();
	DeviceDao dao;
	if (itrac::DeviceType::Washer == _type)
	{
		QList<Washer> washers;
		result_t resp = dao.getWasherList(&washers);
		if (resp.isOk())
		{
			_deviceBox->addItem(QString("不限"), QString(""));
			for (auto &device : washers) {
				_deviceBox->addItem(device.name, device.id);
			}
			updateGeometry();
		}
		else
		{
			XNotifier::warn(QString("无法获取设备列表: ").append(resp.msg()));
			return;
		}
	}

	if (itrac::DeviceType::Sterilizer == _type)
	{
		QList<Sterilizer> sterilizers;
		result_t resp = dao.getSterilizerList(&sterilizers);
		if (resp.isOk())
		{
			_deviceBox->addItem(QString("不限"), QString(""));
			for (auto &device : sterilizers) {
				_deviceBox->addItem(device.name, device.id);
			}
			updateGeometry();
		}
		else
		{
			XNotifier::warn(QString("无法获取设备列表: ").append(resp.msg()));
			return;
		}
	}
}

void DeviceFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::Device, _deviceBox->currentData().toInt());
}

CycleFilterRow::CycleFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_cycleBox = new QSpinBox(this);
	_cycleBox->setMinimum(0);
	_cycleBox->setMaximum(99);
	_layout->addWidget(_cycleBox);

	_layout->addStretch(1);
	setLayout(_layout);
}

void CycleFilterRow::reset()
{
	_cycleBox->setValue(0);
}

void CycleFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::cycle, _cycleBox->text().toInt());
}

CheckFilterRow::CheckFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_checkBox = new QComboBox(this);
	_checkBox->addItem(QString("不限"), 2);
	_checkBox->addItem(QString("合格"), 1);
	_checkBox->addItem(QString("不合格"), 0);
	_checkBox->setCurrentIndex(0);
	_layout->addWidget(_checkBox);

	_layout->addStretch(1);
	setLayout(_layout);
}

void CheckFilterRow::reset()
{
	_checkBox->setCurrentIndex(0);
}

void CheckFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::Check, _checkBox->currentData().toString());
}

PackTypeFilterRow::PackTypeFilterRow(const QString &head, QWidget *parent)
	:FilterRow(head, parent)
{
	_typeBox = new QComboBox(this);
	_typeBox->setFixedWidth(150);

	_layout->addWidget(_typeBox);

	_layout->addStretch(1);
	setLayout(_layout);

	reset();
}

void PackTypeFilterRow::reset()
{
	_typeBox->clear();

	QString data = QString("{}");

	_http.post(url(PATH_PACKTYPE_SEARCH), QByteArray().append(data), [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取类型列表: ").append(resp.errorString()));
			return;
		}

		_typeBox->addItem(QString("不限"), 0);

		QList<QVariant> devices = resp.getAsList("pack_types");
		for (auto &device : devices) {
			QVariantMap map = device.toMap();
			_typeBox->addItem(map["pack_type_name"].toString(), map["pack_type_id"].toInt());
		}

		updateGeometry();
	});
}

void PackTypeFilterRow::setCondition2Filter(Filter *f)
{
	f->setCondition(FilterFlag::PackType, _typeBox->currentData().toInt());
}