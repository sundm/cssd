#include "addinstrumentdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include <xui/images.h>
#include "rdao/dao/InstrumentDao.h"
#include <qhttpmultipart.h>
#include <thirdparty/qjson/src/parser.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>

AddInstrumentDialog::AddInstrumentDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _pinyinEdit(new Ui::FlatEdit)
	, _checkVIPBox(new QCheckBox("贵重器械"))
	//, _checkImplantBox(new QCheckBox("植入器械"))
	//, _imgLabel(new XPicture(this))
	, _waiter(new WaitingSpinner(this))
{
	setWindowTitle("添加新器械");

	_isModify = false;

	_pinyinEdit->setInputValidator(Ui::InputValitor::LetterOnly);

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &AddInstrumentDialog::accept);

	/*_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);*/

	//QPushButton *loadButton = new QPushButton("加载图片");
	//loadButton->setIcon(QIcon(":/res/plus-24.png"));
	//connect(loadButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("器械名"), 0, 0);
	mainLayout->addWidget(new QLabel("拼音检索码"), 1, 0);
	
	mainLayout->addWidget(_nameEdit, 0, 1);
	mainLayout->addWidget(_pinyinEdit, 1, 1);
	mainLayout->addWidget(_checkVIPBox, 2, 0, 1, 1);
	//mainLayout->addWidget(_checkImplantBox, 2, 1, 1, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);

	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

void AddInstrumentDialog::setInfo(const QString &id)
{
	setWindowTitle("修改器械");
	_isModify = true;
	_instrumentId = id;

	InstrumentDao dao;
	InstrumentType it;
	result_t res = dao.getInstrumentType(_instrumentId.toInt(), &it);

	_nameEdit->setText(it.name);
	_pinyinEdit->setText(it.pinyin);
	_nameEdit->setReadOnly(_isModify);

	_checkVIPBox->setChecked(it.isVip);
	//_checkImplantBox->setChecked(it.category);

}

void AddInstrumentDialog::accept() {
	QString name = _nameEdit->text();
	QString pinyin = _pinyinEdit->text().toUpper();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}
	if (pinyin.isEmpty()) {
		_pinyinEdit->setFocus();
		return;
	}

	InstrumentDao dao;
	InstrumentType it;
	it.name = name;
	//it.category = _checkImplantBox->checkState() ? 
	//	Rt::InstrumentCategory::ImplantedInstrument : 
	//	Rt::InstrumentCategory::NormalInstrument;
	it.isVip = _checkVIPBox->checkState();
	it.pinyin = pinyin;

	_waiter->start();
	if (_isModify)
	{
		it.typeId = _instrumentId.toInt();
		result_t res = dao.updateInstrumentType(it);
		if (res.isOk())
			return QDialog::accept();
		else
			XNotifier::warn(QString("修改器械失败: ").append(res.msg()));
	}
	else
	{
		result_t res = dao.addInstrumentType(it);
		if (res.isOk())
			return QDialog::accept();
		else
			XNotifier::warn(QString("添加器械失败: ").append(res.msg()));
		
	}
	
}