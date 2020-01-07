#pragma once
#include "stdafx.h"
#include "rfidreader.h"
#include "ModuleReader.h"
#include <list>
#include <string>
#include <map>

class DLL_API DesktopReader : public AbstractRfidReader
{
public:
	static DesktopReader* getInstance();

	bool connect();
	bool isConnected();
	void disconnect();

	void setAddress(const std::string address) { _address = address;};
	void setAntenna(const int count) { _antenna = count; };
	const std::string getAddress() { return _address; };
	const int getAntenna() { return _antenna; };
	READER_ERR setStart();
	READER_ERR setStop();
	READER_ERR getLastErr() { return _err; };
private:
	DesktopReader();
	~DesktopReader();

	static DesktopReader* _instance;

private:
	bool _connected;
	int _hreader;
	BackReadOption m_BROption;
	READER_ERR _err;
	std::string _address;
	int _antenna;
};