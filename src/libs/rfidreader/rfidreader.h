#pragma once
#include <stdio.h>
#include <string>
#include <list>

#ifdef _DLL_EXPORTS
#define DLL_API _declspec(dllexport)
#else
#define DLL_API _declspec(dllimport)
#endif

class TSL_1128_Private;

enum class CodeType {
	Transponder,
	Barcode,
	QRCode
};

class DLL_API Abstractlistener
{
public:
	Abstractlistener() {};
	virtual ~Abstractlistener() {};
	virtual void actionperformed(CodeType type, std::string code) = 0;
};

class DLL_API AbstractRfidReader
{
public:
	AbstractRfidReader();
	virtual ~AbstractRfidReader();

	//reader method
	virtual bool connect() = 0;
	virtual bool isConnected() = 0;
	virtual void disconnect() = 0;

	//listener method
	void addListener(Abstractlistener *);
	void removeListener(Abstractlistener *);
	void clearListeners();
	void broadcast(CodeType type, std::string code);

private:
	std::list<Abstractlistener*> *_listeners;
};