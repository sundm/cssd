#include "stdafx.h"
#include "rfidreader.h"

#include<vcclr.h>
#include <msclr\marshal_cppstd.h>

#using "TechnologySolutions.Rfid.AsciiProtocol.FX35.dll"

using namespace System;
using namespace TechnologySolutions::Rfid::AsciiProtocol;
using namespace TechnologySolutions::Rfid::AsciiProtocol::Commands;
using namespace TechnologySolutions::Rfid::AsciiProtocol::Parameters;

//初始化静态成员变量
RfidReader *RfidReader::m_instance = NULL;

std::string CastToString(System::String^ str)
{
	IntPtr ip = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
	const char* ch = static_cast<const char*>(ip.ToPointer());
	std::string stdStr = ch;
	System::Runtime::InteropServices::Marshal::FreeHGlobal(ip);
	return stdStr;
}

static void AsynchronousBarcode_Received(Object ^sender, BarcodeEventArgs ^e)
{
	System::Diagnostics::Debug::WriteLine(e->ToString());
	RfidReader::getInstance()->firestatelistener(CodeType::Barcode, CastToString(e->Barcode));
}

static void AsynchronousTransponder_Received(Object ^sender, TransponderDataEventArgs ^e)
{
	System::Diagnostics::Debug::WriteLine(e->ToString());
	RfidReader::getInstance()->firestatelistener(CodeType::Transponder, CastToString(e->Transponder->Epc));
}

RfidReader* RfidReader::getInstance()
{
	if (m_instance == NULL) {
		m_instance = new RfidReader();
	}
	
	return m_instance;
}

void RfidReader::deleteInstance()
{
	if (m_instance)
	{
		delete m_instance;
	}

	m_instance = NULL;
}

RfidReader::RfidReader()
{
	AsciiCommander ^commander = gcnew AsciiCommander();

	InventoryCommand ^ic = gcnew InventoryCommand();
	ic->TransponderReceived += gcnew System::EventHandler<TransponderDataEventArgs^>(AsynchronousTransponder_Received);
	commander->AddResponder(ic->Responder);

	BarcodeCommand ^bc = gcnew BarcodeCommand();
	bc->BarcodeReceived += gcnew System::EventHandler<BarcodeEventArgs^>(AsynchronousBarcode_Received);
	commander->AddResponder(bc->Responder);

	gcroot<AsciiCommander^> *a = new gcroot<AsciiCommander^>(commander);
	this->_commander = static_cast<void*>(a);

	gcroot<InventoryCommand^> *b = new gcroot<InventoryCommand^>(ic);
	this->_transponderResponder = static_cast<void*>(b);

	gcroot<BarcodeCommand^> *c = new gcroot<BarcodeCommand^>(bc);
	this->_barcodeResponder = static_cast<void*>(c);
}

RfidReader::~RfidReader()
{
	if (this->_commander)
	{
		gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(this->_commander);
		delete c;
		this->_commander = 0;
	}

	if (this->_transponderResponder)
	{
		gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(this->_transponderResponder);
		delete ic;
		this->_transponderResponder = 0;
	}

	if (this->_barcodeResponder)
	{
		gcroot<BarcodeCommand^> *bc = static_cast<gcroot<BarcodeCommand^>*>(this->_barcodeResponder);
		delete bc;
		this->_barcodeResponder = 0;
	}
}

bool RfidReader::connect(const std::string port)
{
	String ^p = gcnew String(port.c_str());
	bool isConnect = false;
	try
	{
		IAsciiSerialPort ^_port = gcnew SerialPortWrapper(p);
		gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(this->_commander);
		isConnect = ((AsciiCommander^)*c)->Connect(_port);
	}
	catch (Exception ^e)
	{
		System::Diagnostics::Debug::WriteLine(e->Message);
	}
	return isConnect;
}

void RfidReader::disconnect()
{
	gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(this->_commander);
	((AsciiCommander^)*c)->Disconnect();
}

bool RfidReader::isConnected()
{
	gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(this->_commander);
	
	return ((AsciiCommander^)*c)->IsConnected;
}

const std::list<std::string> RfidReader::refreshPorts()
{
	std::list<std::string> list;
	array<System::String^> ^ports = System::IO::Ports::SerialPort::GetPortNames();
	for each (System::String ^port in ports)
	{
		list.push_front(CastToString(port));
	}

	return list;
}

int RfidReader::getOutPutPower()
{
	gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(this->_transponderResponder);
	System::Nullable<int> nullableInt = ((InventoryCommand ^)*ic)->OutputPower;
	if (nullableInt.HasValue)
		return nullableInt.Value;
	else
		return 0;
}

void RfidReader::setOutPutPower(const int power)
{
	System::Nullable<int> nullableInt = power;
	gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(this->_transponderResponder);
	((InventoryCommand ^)*ic)->OutputPower = nullableInt;
}

int RfidReader::getcount()
{
	return listenerlist.size();
}

void RfidReader::addlistener(Abstractlistener *listener)
{
	listenerlist.push_back(listener);
}

void RfidReader::removelistener(Abstractlistener *listener)
{
	for each (Abstractlistener *li in listenerlist)
	{
		if (li == listener)
		{
			listenerlist.remove(listener);
			break;
		}
	}
}

void RfidReader::clearlistener()
{
	listenerlist.clear();
}

void RfidReader::firestatelistener(CodeType type, std::string code)
{
	for each (Abstractlistener *listener in listenerlist)
	{
		listener->actionperformed(type, code);
	}
}