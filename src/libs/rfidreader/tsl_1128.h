#pragma once
#include "stdafx.h"
#include "rfidreader.h"
#include <list>
#include <string>
#include <map>

DLL_API const std::list<std::string> refreshPorts();

class DLL_API TSL1128Reader : public AbstractRfidReader
{
public:
	TSL1128Reader();
	~TSL1128Reader();

	bool connect();
	bool isConnected();
	void disconnect();

	int getOutPutPower();
	//the power is in range(10, 29)
	void setOutPutPower(const int power);

	void setName(const std::string name) { _name = name; };
	void setPort(const std::string port) { _port = port; };
	const std::string getName() { return _name; };
	const std::string getPort() { return _port; };
private:
	void* _commander;
	void* _transponderResponder;
	void* _barcodeResponder;

	std::string _name;
	std::string _port;
};