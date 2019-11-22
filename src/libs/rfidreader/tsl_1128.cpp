#include "stdafx.h"
#include "tsl_1128.h"
#include <vcclr.h>
#include <msclr\marshal_cppstd.h>

#using "TechnologySolutions.Rfid.AsciiProtocol.FX35.dll"

using namespace System;
using namespace System::Collections::Generic;
using namespace TechnologySolutions::Rfid::AsciiProtocol;
using namespace TechnologySolutions::Rfid::AsciiProtocol::Commands;
using namespace TechnologySolutions::Rfid::AsciiProtocol::Parameters;

ref class ManagedGlobals {
public:
	//static Dictionary<Object^, int> ^dic = nullptr;
	static Dictionary<Object^, int> ^dic = gcnew Dictionary<Object ^, int>;
};

namespace {
	std::string CastToString(System::String^ str)
	{
		IntPtr ip = System::Runtime::InteropServices::Marshal::StringToHGlobalAnsi(str);
		const char* ch = static_cast<const char*>(ip.ToPointer());
		std::string stdStr = ch;
		System::Runtime::InteropServices::Marshal::FreeHGlobal(ip);
		return stdStr;
	}
}

const std::list<std::string> refreshPorts()
{
	std::list<std::string> list;
	array<System::String^> ^ports = System::IO::Ports::SerialPort::GetPortNames();
	for each (System::String ^port in ports)
	{
		list.push_front(CastToString(port));
	}

	return list;
}

static void AsynchronousBarcode_Received(Object ^sender, BarcodeEventArgs ^e)
{
	System::Diagnostics::Debug::WriteLine(e->ToString());
	if (ManagedGlobals::dic->ContainsKey(sender)) {
		int value = ManagedGlobals::dic[sender];

		TSL1128Reader* r = static_cast<TSL1128Reader*>((void*)value);
		r->broadcast(CodeType::Barcode, CastToString(e->Barcode));
	}
}

static void AsynchronousTransponder_Received(Object ^sender, TransponderDataEventArgs ^e)
{
	System::Diagnostics::Debug::WriteLine(e->ToString());
	if (ManagedGlobals::dic->ContainsKey(sender)) {
		int value = ManagedGlobals::dic[sender];

		TSL1128Reader* r = static_cast<TSL1128Reader*>((void*)value);
		r->broadcast(CodeType::Transponder, CastToString(e->Transponder->Epc));
	}
}

TSL1128Reader::TSL1128Reader()
{
	AsciiCommander ^commander = gcnew AsciiCommander();

	InventoryCommand ^ic = gcnew InventoryCommand();
	ic->TransponderReceived += gcnew System::EventHandler<TransponderDataEventArgs^>(AsynchronousTransponder_Received);
	commander->AddResponder(ic->Responder);
	ManagedGlobals::dic->Add(ic, (int)this);

	BarcodeCommand ^bc = gcnew BarcodeCommand();
	bc->BarcodeReceived += gcnew System::EventHandler<BarcodeEventArgs^>(AsynchronousBarcode_Received);
	commander->AddResponder(bc->Responder);
	ManagedGlobals::dic->Add(bc, (int)this);

	gcroot<AsciiCommander^> *a = new gcroot<AsciiCommander^>(commander);
	_commander = static_cast<void*>(a);

	gcroot<InventoryCommand^> *b = new gcroot<InventoryCommand^>(ic);
	_transponderResponder = static_cast<void*>(b);

	gcroot<BarcodeCommand^> *c = new gcroot<BarcodeCommand^>(bc);
	_barcodeResponder = static_cast<void*>(c);
}

TSL1128Reader::~TSL1128Reader()
{
	if (_commander)
	{
		gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(_commander);
		delete c;
		_commander = 0;
	}

	if (_transponderResponder)
	{
		gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(_transponderResponder);
		delete ic;
		_transponderResponder = 0;
	}

	if (_barcodeResponder)
	{
		gcroot<BarcodeCommand^> *bc = static_cast<gcroot<BarcodeCommand^>*>(_barcodeResponder);
		delete bc;
		_barcodeResponder = 0;
	}
}

bool TSL1128Reader::connect()
{
	if (_port.empty()) return false;

	String ^p = gcnew String(_port.c_str());
	bool isConnect = false;
	try
	{
		IAsciiSerialPort ^port = gcnew SerialPortWrapper(p);
		gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(_commander);
		isConnect = ((AsciiCommander^)*c)->Connect(port);
	}
	catch (Exception ^e)
	{
		System::Diagnostics::Debug::WriteLine(e->Message);
	}
	return isConnect;
}

void TSL1128Reader::disconnect()
{
	gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(_commander);
	((AsciiCommander^)*c)->Disconnect();
}

bool TSL1128Reader::isConnected()
{
	gcroot<AsciiCommander^> *c = static_cast<gcroot<AsciiCommander^>*>(_commander);

	return ((AsciiCommander^)*c)->IsConnected;
}

int TSL1128Reader::getOutPutPower()
{
	gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(_transponderResponder);
	System::Nullable<int> nullableInt = ((InventoryCommand ^)*ic)->OutputPower;
	if (nullableInt.HasValue)
		return nullableInt.Value;
	else
		return 0;
}

void TSL1128Reader::setOutPutPower(const int power)
{
	System::Nullable<int> nullableInt = power;
	gcroot<InventoryCommand^> *ic = static_cast<gcroot<InventoryCommand^>*>(_transponderResponder);
	((InventoryCommand ^)*ic)->OutputPower = nullableInt;
}