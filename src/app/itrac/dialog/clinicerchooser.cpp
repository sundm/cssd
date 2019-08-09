#include "clinicerchooser.h"
#include "barcode.h"
#include "core/user.h"
#include "core/net/url.h"
#include "xnotifier.h"
#include "ui/ui_commons.h"
#include "ui/buttons.h"
#include "ui/inputfields.h"
#include <qscanner/qscanner.h>
#include <QtWidgets/QtWidgets>

ClinicerChooser::ClinicerChooser(Scanable* candidate, QWidget *parent, int deptId)
	:QDialog(parent)
	, _opIdEdit(new Ui::FlatEdit)
{
	if (candidate) {
		setCandidate(candidate);
	}

	_deptId = deptId;
	_opId = 0;

	setWindowTitle("人员确认");
	QVBoxLayout *layout = new QVBoxLayout(this);

	const QString tip =
		"请扫描人员条码，或在下方输入\"人员工牌ID\"\n";
	QLabel *tipLabel = new QLabel(tip);
	tipLabel->setWordWrap(true);
	layout->addWidget(tipLabel);

	_opIdEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_opIdEdit->setMaxLength(8);
	layout->addWidget(_opIdEdit);

	layout->addWidget(Ui::createSeperator(Qt::Horizontal));

	Ui::PrimaryButton *button = new Ui::PrimaryButton("确定");
	connect(button, &QPushButton::clicked, this, &ClinicerChooser::apply);
	layout->addWidget(button);
}

int ClinicerChooser::get(Scanable* candidate, QWidget *parent, int deptId) {
	ClinicerChooser chooser(candidate, parent, deptId);
	return (QDialog::Accepted == chooser.exec()) ? chooser._opId : 0;
}

void ClinicerChooser::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::User) _opId = code.toInt();
	else
		_opId = 0;

	apply();
}

void ClinicerChooser::apply() {
	if (0 == _opId)
	{
		QString opIdStr = _opIdEdit->text();
		if (opIdStr.isEmpty() || opIdStr.length() < 8) return;
		_opId = opIdStr.toInt();
	}
	
	if (_deptId)
	{
		QString req("{\"operator_id\":%1 }");
		QByteArray data;
		data.append(req.arg(_opId));

		post(url(PATH_USER_SEARCH), data, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				return;
			}

			QList<QVariant> users = resp.getAsList("user_list");
			if (users.isEmpty())
			{
				return;
			}

			bool isMatch = false;
			for (int i = 0; i != users.count(); ++i) {
				QVariantMap map = users[i].toMap();
				if (_deptId == map["dept_id"].toInt())
				{
					isMatch = true;
					_opName = map["name"].toString();
				}
					
			}

			if (isMatch) QDialog::accept();
			else {
				XNotifier::warn(QString("接收员和目标科室不匹配").append(resp.errorString()));
				_opIdEdit->setFocus();
			}
		});
	}
	else
		QDialog::accept();
}
