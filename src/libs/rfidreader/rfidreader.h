#pragma once
#include <stdio.h>
#include <string>
#include <list>

#ifdef _DLL_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

enum class CodeType {
	Transponder,
	Barcode
};

class Abstractlistener
{
public:
	Abstractlistener() {};
	virtual ~Abstractlistener() {};
	virtual void actionperformed(CodeType type, std::string code) = 0;
};

class DLL_API RfidReader {

private:
	RfidReader();
	~RfidReader();

public:
	static RfidReader* getInstance();
	static void deleteInstance();

	const std::list<std::string> refreshPorts();
	bool connect(const std::string port);
	bool isConnected();
	void disconnect();
	int getOutPutPower();

	//the power is range in(10, 29)
	void setOutPutPower(const int power);

	int getcount();
	void addlistener(Abstractlistener *);
	void removelistener(Abstractlistener *);
	void clearlistener();
	void firestatelistener(CodeType type, std::string code);

private:
	static RfidReader* m_instance;
	void* _commander;
	void* _transponderResponder;
	void* _barcodeResponder;

	std::list<Abstractlistener*> listenerlist;
};