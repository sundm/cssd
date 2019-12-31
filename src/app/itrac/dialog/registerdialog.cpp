#include "registerdialog.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "des/des3.h"
#include <QDomDocument>
#include <QtWidgets/QtWidgets>

RegisterDialog::RegisterDialog(const QString &src, QWidget *parent)
	: QDialog(parent)
	, _srcCodeEdit(new Ui::FlatEdit)
	, _codeEdit(new Ui::FlatEdit)
{
	setWindowTitle("注册");
	_srcCodeEdit->setReadOnly(true);
	_srcCodeEdit->setInputMask("XXXX-XXXX-XXXX-XXXX");
	_codeEdit->setInputMask("XXXX-XXXX-XXXX-XXXX");

	_srcCodeEdit->setText(src);

	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow("机器码", _srcCodeEdit);
	formLayout->addRow("注册码", _codeEdit);

	QPushButton *submitButton = new QPushButton("注册");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterBtnClicked);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(15);
	mainLayout->addLayout(formLayout);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal));
	mainLayout->addWidget(submitButton, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(360, height());
}

void RegisterDialog::writeKeyToFile()
{
	QString code = _codeEdit->text();

	QString xmlFileName = "prepareSetting.xml";
	QFile file(xmlFileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return;
	}

	QDomDocument document;
	QString error;
	int row = 0, column = 0;
	if (!document.setContent(&file, false, &error, &row, &column))
	{
		return;
	}

	file.close();

	if (document.isNull())
	{
		return;
	}

	QDomElement root = document.documentElement();

	QString root_tag_name = root.tagName();
	if (root_tag_name.compare("cssd") == 0)
	{
		QDomNodeList codeNodes = document.elementsByTagName("code");
		int count = codeNodes.count();
		for (int i = 0; i < count; i++) {
			QDomNode readerNode = codeNodes.at(0);
			root.removeChild(readerNode);
		}

		QDomElement element = document.createElement("code");
		element.setAttribute("value", code);
		root.appendChild(element);

		if (!file.open(QFile::WriteOnly | QFile::Text))
		{
			return;
		}

		QTextStream out(&file);
		document.save(out, 4);
		file.close();
	}

}

void RegisterDialog::onRegisterBtnClicked() {
	QString src = _srcCodeEdit->text();
	QString code = _codeEdit->text();
	src = src.remove("-");
	code = code.remove("-");
	if (code.isEmpty() || code.length() < 16)
	{
		_codeEdit->setFocus();
		return;
	}

	DES3 des(0x2BD6459F82C5B300, 0x952C49104881FF48, 0x2BD6459F82C5B300);

	bool isOK = false;
	ui64 input = code.toULongLong(&isOK, 16);
	ui64 uiSrc = src.toULongLong(&isOK, 16);
	if (isOK)
	{
		ui64 result = des.decrypt(input);
		if (result == uiSrc)
		{
			writeKeyToFile();
			return QDialog::accept();
		}
		else
		{
			QMessageBox::critical(this, "注册", "验证码校验失败", QMessageBox::Yes | QMessageBox::Yes);
			return;
		}
	}
	else
	{
		_codeEdit->setFocus();
		return;
	}

}

