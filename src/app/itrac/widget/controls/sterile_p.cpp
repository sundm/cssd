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

void SterileInfoGroup::updateInfo(const DeviceBatchInfo &result)
{
	_bcEdit->setText(result.batchId);
	_deviceEdit->setText(result.deviceName);
	_cycleEdit->setText(QString::number(result.cycleCount));
	_startTimeEdit->setText(result.startTime.toString("yyyy-MM-dd HH:mm:ss"));
}

QString SterileInfoGroup::testId() const {
	return _bcEdit->text();
}

CheckItem::CheckItem(const QString &title, Rt::SterilizeVerdict verdict, bool involved, QWidget *parent /*= nullptr*/)
	:QGroupBox(parent)
{
	QVBoxLayout  *layout = new QVBoxLayout(this);
	QLabel *titleLabel = new QLabel(title);
	titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	layout->addWidget(titleLabel);
	setMinimumHeight(120);
	reset(verdict, involved);
}

void CheckItem::reset(Rt::SterilizeVerdict verdict, bool involved) {
	_verdict = verdict;

	QLayout *layout = this->layout();
	QLayoutItem *child;
	while ((child = layout->takeAt(1)) != 0) {
		if (child->widget()) {
			delete child->widget();
		}
		delete child;
	}

	_disabled = true;
	switch (verdict) {
	case Rt::Uninvolved:
		layout->addWidget(new QLabel("未涉及"));
		break;
	case Rt::Qualified:
		layout->addWidget(new QLabel("已审(合格)"));
		break;
	case Rt::Unqualified:
		layout->addWidget(new QLabel("已审(不合格)"));
		break;
	case Rt::Unchecked:
		QRadioButton * qualifiedButton = new QRadioButton("合格");
		QRadioButton * unQualifiedButton = new QRadioButton("不合格");
		connect(qualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = Rt::Qualified; });
		connect(unQualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = Rt::Unqualified; });
		layout->addWidget(qualifiedButton);
		layout->addWidget(unQualifiedButton);
		if (involved)
		{
			QRadioButton * involvedButton = new QRadioButton("未涉及");
			connect(involvedButton, &QRadioButton::clicked, this, [this] {_verdict = Rt::Uninvolved; });
			layout->addWidget(involvedButton);
		}
		_disabled = false;
		break;
	}
	setStyleSheet("background-color:#eeeeee;");
}

void SterileInfoGroup::reset() {
	_bcEdit->clear();
	_deviceEdit->clear();
	_cycleEdit->clear();
	_startTimeEdit->clear();
}

Rt::SterilizeVerdict CheckItem::verdict() const {
	return _verdict;
}

SterileCheckGroup::SterileCheckGroup(QWidget *parent /*= nullptr*/)
{
	QGridLayout *layout = new QGridLayout(this);

	_phyItem = new CheckItem("物理监测审核", Rt::Unchecked);
	layout->addWidget(_phyItem, 0, 0);

	_chemItem = new CheckItem("化学监测审核", Rt::Unchecked);
	layout->addWidget(_chemItem, 0, 1);

	_bioItem = new CheckItem("生物监测审核", Rt::Unchecked, true);
	layout->addWidget(_bioItem, 0, 2);

	QPushButton *commonButton = new QPushButton("常见异常");
	layout->addWidget(commonButton, 1, 0);

	//_wetItem = new QCheckBox("湿包", this);
	//layout->addWidget(_wetItem, 2, 0);

	_lostLabelItem = new QCheckBox("飞标", this);
	layout->addWidget(_lostLabelItem, 2, 0);

	QPushButton *reasonButton = new QPushButton("其他异常描述");
	layout->addWidget(reasonButton, 3, 0);

	QTextEdit *reasonEdit = new QTextEdit;
	layout->addWidget(reasonEdit, 4, 0, 1, 3);
}

Sterile::Result SterileCheckGroup::verdicts() const
{
	Sterile::Result result;

	if (_first) {
		result.physics =  _phyItem->verdict();
		result.chemistry = _chemItem->verdict();
		result.lost = _lostLabelItem->isChecked() ? Rt::Unqualified : Rt::Qualified;
	}

	if (!_bioItem->disabled())
	{
		result.bio =  _bioItem->verdict();
	}

	return result;
}

bool SterileCheckGroup::isFirst()
{
	return _first;
}

void SterileCheckGroup::updateInfo(const SterilizeResult &resultInfo)
{
	_phyItem->reset(resultInfo.phyVerdict, false);
	_chemItem->reset(resultInfo.cheVerdict, false);
	_bioItem->reset(resultInfo.bioVerdict, true);
	_lostLabelItem->setChecked(resultInfo.hasLabelOff);

	_first = resultInfo.phyVerdict == Rt::SterilizeVerdict::Unchecked;
	_lostLabelItem->setEnabled(_first);
	
}

void SterileCheckGroup::reset() {
	_phyItem->reset(Rt::Unchecked, false);
	_chemItem->reset(Rt::Unchecked, false);
	_bioItem->reset(Rt::Unchecked, true);
	//_wetItem->setChecked(false);
	_lostLabelItem->setChecked(false);
	//_wetItem->setEnabled(true);
	_lostLabelItem->setEnabled(true);
}
