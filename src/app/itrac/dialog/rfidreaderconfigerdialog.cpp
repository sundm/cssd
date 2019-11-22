#include "rfidreaderconfigerdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "xnotifier.h"
#include <QtWidgets/QtWidgets>

ConfigRfidReaderDialog::ConfigRfidReaderDialog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _comBox(new QComboBox(this))
{
	setWindowTitle("配置RFID扫描器");

	QPushButton *submitButton = new QPushButton("确定");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &ConfigRfidReaderDialog::accept);

	QPushButton *connectButton = new QPushButton("连接");
	connect(connectButton, &QPushButton::clicked, this, &ConfigRfidReaderDialog::onClickConnectBtn);

	QPushButton *disconnectButton = new QPushButton("断开");
	connect(disconnectButton, &QPushButton::clicked, this, &ConfigRfidReaderDialog::onClickDisconnectBtn);

	QStringList portlist;
	std::list<std::string> ports = refreshPorts();
	for each (std::string port in ports)
	{
		portlist.append(QString::fromStdString(port));
	}
	_comBox->addItems(portlist);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(_nameEdit, 0, 0);
	mainLayout->addWidget(_comBox, 0, 1);
	mainLayout->addWidget(connectButton, 1, 0, 1, 2);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 2, 0, 1, 2);
	mainLayout->addWidget(submitButton, 3, 0, 1, 2, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(360, height());
}

void ConfigRfidReaderDialog::accept() {
	/*REMEMBER_READER = _rememberMeBox->isChecked();
	QSettings *configIni = new QSettings("prepareSettings.ini", QSettings::IniFormat);
	configIni->setValue("port/remember", REMEMBER_READER);
	configIni->setValue("port/name", LAST_COM);*/
	return QDialog::accept();
}

void ConfigRfidReaderDialog::onClickConnectBtn() {
	QString port = _comBox->currentText();

	if (port.isEmpty())
	{
		XNotifier::warn(QString("串口号为空"));
		return;
	}

	/*if (RfidReader::getInstance()->connect(port.toStdString()))
	{
		LAST_COM = port;
		XNotifier::warn(QString("%1连接成功！").arg(port));
	}
	else {
		XNotifier::warn(QString("%1连接失败！").arg(port));
	}*/
}

void ConfigRfidReaderDialog::onClickDisconnectBtn() {
	//RfidReader::getInstance()->disconnect();
}