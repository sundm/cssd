#include "sterile_p.h"
#include "xnotifier.h"

#include "itracnamespace.h"
#include "core/datawrapper.h"
#include "core/net/url.h"
#include "ui/inputfields.h"
#include <QtWidgets/QtWidgets>

SterileInfoGroup::SterileInfoGroup(QWidget *parent)
	: QGroupBox(parent)
	, _bcEdit(new Ui::FlatEdit)
	, _deviceEdit(new Ui::FlatEdit)
	, _cycleEdit(new Ui::FlatEdit)
	, _startTimeEdit(new Ui::FlatEdit)
{
	_deviceEdit->setReadOnly(true);
	_cycleEdit->setReadOnly(true);
	_startTimeEdit->setReadOnly(true);
	connect(_bcEdit, &QLineEdit::returnPressed,
		[this] {emit testIdChanged(_bcEdit->text()); }
	);

	QFormLayout *layout = new QFormLayout(this);
	layout->setWidget(0, QFormLayout::LabelRole, new QLabel("监测条码"));
	layout->setWidget(0, QFormLayout::FieldRole, _bcEdit);
	layout->setWidget(1, QFormLayout::LabelRole, new QLabel("灭菌设备"));
	layout->setWidget(1, QFormLayout::FieldRole, _deviceEdit);
	layout->setWidget(2, QFormLayout::LabelRole, new QLabel("灭菌锅次"));
	layout->setWidget(2, QFormLayout::FieldRole, _cycleEdit);
	layout->setWidget(3, QFormLayout::LabelRole, new QLabel("灭菌开始时间"));
	layout->setWidget(3, QFormLayout::FieldRole, _startTimeEdit);
};

void SterileInfoGroup::updateInfo(const Sterile::TestInfo &testInfo)
{
	_bcEdit->setText(testInfo.testId);
	_deviceEdit->setText(testInfo.device);
	_cycleEdit->setText(QString::number(testInfo.cycle));
	_startTimeEdit->setText(testInfo.startTimeStamp);
}

QString SterileInfoGroup::testId() const {
	return _bcEdit->text();
}

CheckItem::CheckItem(const QString &title, int verdict, QWidget *parent /*= nullptr*/)
	:QGroupBox(parent)
{
	QVBoxLayout  *layout = new QVBoxLayout(this);
	QLabel *titleLabel = new QLabel(title);
	titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	layout->addWidget(titleLabel);
	reset(verdict);
}

void CheckItem::reset(int verdict) {
	_verdict = verdict;

	QLayout *layout = this->layout();
	QLayoutItem *child;
	while ((child = layout->takeAt(1)) != 0) {
		delete child;
	}

	switch (verdict) {
	case itrac::NotInvolved:
		layout->addWidget(new QLabel("未涉及"));
		break;
	case itrac::Success:
		layout->addWidget(new QLabel("已审(合格)"));
		break;
	case itrac::Failed:
		layout->addWidget(new QLabel("已审(不合格)"));
		break;
	case itrac::NotChecked:
		QRadioButton * qualifiedButton = new QRadioButton("合格");
		QRadioButton * unQualifiedButton = new QRadioButton("不合格");
		connect(qualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = itrac::Success; });
		connect(unQualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = itrac::Failed; });
		layout->addWidget(qualifiedButton);
		layout->addWidget(unQualifiedButton);
		//return;
	}
	setStyleSheet("background-color:#eeeeee;");
}


void SterileInfoGroup::reset() {
	_bcEdit->clear();
	_deviceEdit->clear();
	_cycleEdit->clear();
	_startTimeEdit->clear();
}

int CheckItem::verdict() const {
	return _verdict;
}

SterileCheckGroup::SterileCheckGroup(QWidget *parent /*= nullptr*/)
{
	QGridLayout *layout = new QGridLayout(this);

	_phyItem = new CheckItem("物理监测审核", itrac::NotChecked);
	layout->addWidget(_phyItem, 0, 0);

	_chemItem = new CheckItem("化学监测审核", itrac::NotChecked);
	layout->addWidget(_chemItem, 0, 1);

	_bioItem = new CheckItem("生物监测审核", itrac::NotChecked);
	layout->addWidget(_bioItem, 0, 2);

	QPushButton *reasonButton = new QPushButton("具体描述");
	layout->addWidget(reasonButton, 2, 0);

	QTextEdit *reasonEdit = new QTextEdit;
	layout->addWidget(reasonEdit, 3, 0, 1, 3);
}

Sterile::Result SterileCheckGroup::verdicts() const
{
	Sterile::Result result;
	result.physics = _phyItem->verdict();
	result.chemistry = _chemItem->verdict();
	result.bio= _bioItem->verdict();
	return result;
}

void SterileCheckGroup::reset() {
	_phyItem->reset(itrac::NotChecked);
	_chemItem->reset(itrac::NotChecked);
	_bioItem->reset(itrac::NotChecked);
}
