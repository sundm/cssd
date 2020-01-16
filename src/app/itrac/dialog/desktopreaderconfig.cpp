#include "desktopreaderconfig.h"
#include "ui/inputip.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "core/net/url.h"
#include "../libs/rfidreader/desktopreader.h"
#include "xnotifier.h"
#include <QDomDocument>
#include <QtWidgets/QtWidgets>

DesktopReaderConfigDialog::DesktopReaderConfigDialog(const QString &address
	, const int antenna
	, QWidget *parent) : QDialog(parent)
	, _addressEdit(new TLineEditIP)
	, _combo(new QComboBox)
{
	setWindowTitle("连接设置");
	if(address.isEmpty())
		_addressEdit->setText("192.168.1.100");
	else
		_addressEdit->setText(address);

	_combo->clear();
	QMap<int, QString> zone;
	zone.insert(1, "单天线");
	zone.insert(2, "双天线");
	zone.insert(3, "三天线");
	zone.insert(4, "四天线");
	zone.insert(8, "八天线");

	for (auto it = std::begin(zone); it != std::end(zone); ++it) {
		_combo->addItem(it.value(), it.key());
	}

	for (auto it = std::begin(zone); it != std::end(zone); ++it) {
		if (it.key() == antenna)
		{
			_combo->setCurrentText(it.value());
		}
	}

	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow("IP地址", _addressEdit);
	formLayout->addRow("天线端口数", _combo);

	_connectButton = new QPushButton("连接");
	//connectButton->setIcon(QIcon(":/res/check-24.png"));
	_connectButton->setDefault(true);
	connect(_connectButton, &QPushButton::clicked, this, &DesktopReaderConfigDialog::onConnectClicked);

	QPushButton *disconnectButton = new QPushButton("断开");
	connect(disconnectButton, &QPushButton::clicked, this, &DesktopReaderConfigDialog::onDisConnectClicked);

	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonLayout->addWidget(_connectButton);
	buttonLayout->addWidget(disconnectButton);

	QPushButton *submitButton = new QPushButton("确定");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &DesktopReaderConfigDialog::accept);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setSpacing(15);
	mainLayout->addLayout(formLayout);
	mainLayout->addLayout(buttonLayout, Qt::AlignHCenter);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal));
	mainLayout->addWidget(submitButton, Qt::AlignHCenter);
	setFixedHeight(sizeHint().height());
	resize(360, height());

	_isConnected = DesktopReader::getInstance()->isConnected();

	setItemEnabled();
}

void DesktopReaderConfigDialog::onConnectClicked()
{
	DESKTOP_ADDRESS = _addressEdit->text();
	DESKTOP_ANTENNA = _combo->currentData().toInt();

	DesktopReader::getInstance()->setAddress(DESKTOP_ADDRESS.toStdString());
	DesktopReader::getInstance()->setAntenna(DESKTOP_ANTENNA);

	_isConnected = DesktopReader::getInstance()->connect();
	if (_isConnected)
	{
		XNotifier::warn(QString("连接成功！"), -1, this);
	}
	else
	{
		XNotifier::warn(QString("连接失败！"), -1, this);
	}

	setItemEnabled();
}

void DesktopReaderConfigDialog::onDisConnectClicked()
{
	DesktopReader::getInstance()->disconnect();
	_isConnected = false;
	setItemEnabled();
}

void DesktopReaderConfigDialog::accept() 
{
	DESKTOP_ADDRESS = _addressEdit->text();
	DESKTOP_ANTENNA = _combo->currentData().toInt();

	DesktopReader::getInstance()->setAddress(DESKTOP_ADDRESS.toStdString());
	DesktopReader::getInstance()->setAntenna(DESKTOP_ANTENNA);
	
	return QDialog::accept();
}

void DesktopReaderConfigDialog::setItemEnabled()
{
	_addressEdit->setReadOnly(_isConnected);
	_combo->setDisabled(_isConnected);
	_connectButton->setDisabled(_isConnected);
}