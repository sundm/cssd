#include "washabnormal.h"
#include "barcode.h"
#include "core/user.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/composite/waitingspinner.h"
#include "dialog/operatorchooser.h"
#include "ui/buttons.h"
#include <qscanner/qscanner.h>
#include <QtWidgets/QtWidgets>
#include <xui/searchedit.h>

WashAbnormal::WashAbnormal(Scanable* candidate, QWidget *parent)
	:QDialog(parent)
	, _searchBox(new SearchEdit)
	, _deviceName (new QLabel)
	, _deviceProgram(new QLabel)
	, _deviceCycle(new QLabel)
	, _reasonEdit(new QTextEdit)
	, _waiter(new WaitingSpinner(this))
{
	if (candidate) {
		setCandidate(candidate);
	}

	_searchBox->setPlaceholderText("请扫描或输入篮筐ID");
	_searchBox->setMinimumHeight(36);
	_searchBox->setMinimumWidth(450);
	connect(_searchBox, &QLineEdit::returnPressed, this, &WashAbnormal::startTrace);

	setWindowTitle("清洗异常登记");
	QHBoxLayout *hLayout = new QHBoxLayout();

	_deviceName->setText(QString("清洗机:"));
	_deviceProgram->setText(QString("程序:"));
	_deviceCycle->setText(QString("锅次:"));
	
	
	hLayout->addWidget(_deviceName);
	hLayout->addWidget(_deviceProgram);
	hLayout->addWidget(_deviceCycle);
	
	Ui::PrimaryButton *button = new Ui::PrimaryButton("登记异常");
	connect(button, &QPushButton::clicked, this, &WashAbnormal::accept);
	_reasonEdit->setPlaceholderText(QString("异常描述"));

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(_searchBox);
	vLayout->addLayout(hLayout);
	vLayout->addWidget(_reasonEdit);
	vLayout->addWidget(button);

	_hasPlateId = false;
}

void WashAbnormal::startTrace() {
	handleBarcode(_searchBox->text());
}

void WashAbnormal::handleBarcode(const QString &code) {
	_hasPlateId = false;
	Barcode bc(code);
	if (bc.type() == Barcode::Plate) {
		_searchBox->setText(code);
		search(code);
	}
}

void WashAbnormal::accept() {
	if (_hasPlateId)
	{
		QString desc = _reasonEdit->toPlainText();
		if (desc.isEmpty())
		{
			_reasonEdit->setFocus();
			return;
		}

		int opId = OperatorChooser::get(this, this);
		if (0 == opId) {
			XNotifier::warn(QString("请选择操作员"));
			return;
		}

		QVariantMap vmap;
		vmap.insert("check_operator_id", opId);
		vmap.insert("check_result", QString("0"));
		vmap.insert("wash_id", _washId);
		vmap.insert("check_desc", desc);

		_waiter->start();
		post(url(PATH_WASH_CHECK), vmap, [this](QNetworkReply *reply) {
			_waiter->stop();
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("登记失败: ").append(resp.errorString()));
				return;
			}

			return QDialog::accept();
		});

		
	}
	else {
		_searchBox->setFocus();
	}
}

void WashAbnormal::search(const QString &plate_id) {
	QVariantMap vmap;
	vmap.insert("plate_id", plate_id);

	_waiter->start();
	post(url(PATH_WASH_INFO), vmap, [this](QNetworkReply *reply) {
		_waiter->stop();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		_deviceName->setText(QString("清洗机：%1").arg(resp.getAsString("device_name")));
		_deviceProgram->setText(QString("程序：%1").arg(resp.getAsString("mod_name")));
		_deviceCycle->setText(QString("锅次：第%1锅次").arg(resp.getAsInt("device_cycle")));

		_washId = resp.getAsInt("wash_id");

		_hasPlateId = true;
	});

	
}