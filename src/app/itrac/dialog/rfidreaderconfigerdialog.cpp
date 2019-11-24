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

	_connectButton = new QPushButton("连接");
	connect(_connectButton, &QPushButton::clicked, this, &ConfigRfidReaderDialog::onClickConnectBtn);

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
	mainLayout->addWidget(_connectButton, 1, 0);
	mainLayout->addWidget(disconnectButton, 1, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 2, 0, 1, 2);
	mainLayout->addWidget(submitButton, 3, 0, 1, 2, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(360, height());

	_reader = nullptr;
}

void ConfigRfidReaderDialog::accept() {
	QString name = _nameEdit->text();
	if (name.isEmpty())
	{
		_nameEdit->setFocus();
		return;
	}
	QString port = _comBox->currentText();
	if (port.isEmpty())
	{
		_comBox->setFocus();
		return;
	}

	if (_reader == nullptr)
	{
		_reader = new TSL1128Reader();
		
		TSL1128Readers.append(_reader);
	}
	_reader->setName(name.toStdString());
	_reader->setPort(port.toStdString());
	

	return QDialog::accept();
}

void ConfigRfidReaderDialog::onClickConnectBtn() {
	QString name = _nameEdit->text();
	QString port = _comBox->currentText();

	if (port.isEmpty())
	{
		XNotifier::warn(QString("串口号为空"));
		return;
	}

	if (_reader == nullptr)
	{
		_reader = new TSL1128Reader();
		TSL1128Readers.append(_reader);
	}
	
	if (_reader->connect()) {
		XNotifier::warn(QString("%1连接成功！").arg(name));
	}
	else
	{
		XNotifier::warn(QString("%1连接失败！").arg(name));
	}

	setItemEnabled();
}

void ConfigRfidReaderDialog::onClickDisconnectBtn() {
	if (_reader != nullptr)
	{
		_reader->disconnect();

		setItemEnabled();
	}
}

void ConfigRfidReaderDialog::setReader(const int index)
{
	_reader = TSL1128Readers.at(index);
	_nameEdit->setText(QString::fromStdString(_reader->getName()));
	_comBox->setCurrentText(QString::fromStdString(_reader->getPort()));

	setItemEnabled();
}

void ConfigRfidReaderDialog::setItemEnabled()
{
	if (_reader == nullptr) return;

	bool isConnected = _reader->isConnected();
	_nameEdit->setReadOnly(isConnected);
	_comBox->setDisabled(isConnected);
	_connectButton->setDisabled(isConnected);
}