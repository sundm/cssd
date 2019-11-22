#include "launcher.h"
#include "platform/platfom_specific.h"
#include "application.h"
#include "core/net/url.h"
#include "../libs/rfidreader/rfidreader.h"
#include <QFile>
#include <QDomDocument>

namespace Core {

	Launcher::Launcher(int argc, char* argv[], const QString &sysVersion)
		:_argc(argc), _argv(argv), _sysVersion(sysVersion) {
	}

	std::unique_ptr<Launcher> Launcher::create(int argc, char *argv[])
	{
		return std::make_unique<Platform::Launcher>(argc, argv);
	}

	int Launcher::start()
	{
		initialize();

		Application app(this, _argc, _argv);
		//QFont font = app.font();
		//font.setPointSize(12);
		//app.setFont(font);
		return app.exec();
	}

	QString Launcher::sysVersion() const {
		return _sysVersion;
	}

	void Launcher::prepareSettings() {
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

		if (document.isNull())
		{
			return;
		}

		QDomElement root = document.documentElement();

		//root_tag_nameΪpersons
		QString root_tag_name = root.tagName();
		if (root_tag_name.compare("cssd") == 0)
		{
			QDomElement address = root.firstChildElement();
			if (address.isNull() || address.tagName().compare("address") != 0)
				return;
			PATH_BASE = address.attribute("url", "");
			QDomElement labelPrinter = address.nextSiblingElement("labelPrinter");
			LABEL_PRINTER = labelPrinter.attribute("name", "");
			QDomElement commonPrinter = labelPrinter.nextSiblingElement("commonPrinter");
			COMMON_PRINTER = commonPrinter.attribute("name", "");

			QDomNodeList readerNodes = root.elementsByTagName("reader");

			for (int i = 0; i < readerNodes.count(); i++) {
				QDomNode readerNode = readerNodes.at(i);
				if (readerNode.toElement().attribute("type", "0").toInt() == 1)
				{
					TSL1128Reader * reader = new TSL1128Reader();
					reader->setName(readerNode.toElement().attribute("name", "").toStdString());
					reader->setPort(readerNode.toElement().attribute("port", "").toStdString());

					TSL1128Readers.append(reader);
				}
			}
			
		}

		file.close();
		/*
		QSettings *configIni = new QSettings("prepareSettings.ini", QSettings::IniFormat);

		PATH_BASE = configIni->value("address/t").toString();

		LABEL_PRINTER = configIni->value("printer/label").toString();

		COMMON_PRINTER = configIni->value("printer/common").toString();

		REMEMBER_READER = false;

		REMEMBER_READER = configIni->value("port/remember").toBool();

		if (REMEMBER_READER)
		{
			LAST_COM = configIni->value("port/name").toString();
		}
		
		std::list<std::string> ports = RfidReader::getInstance()->refreshPorts();
		for each (std::string port in ports)
		{
			COMPORT_LIST.append(QString::fromStdString(port));
		}

		RfidReader::getInstance()->clearlistener();

		if (_listener == nullptr)
			_listener = new RfidCodelistener();

		RfidReader::getInstance()->addlistener(_listener);

		delete configIni;
		*/
	}

	void Launcher::initialize() {
		QCoreApplication::setApplicationName("Winstrac Client Powered By Winstech");
		prepareSettings();
	}

}
