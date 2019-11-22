#include "stdafx.h"
#include "rfidreader.h"

AbstractRfidReader::AbstractRfidReader()
{
	_listeners = new std::list<Abstractlistener *>();
}

AbstractRfidReader::~AbstractRfidReader()
{
	delete _listeners;
}

void AbstractRfidReader::addListener(Abstractlistener *listener)
{
	_listeners->push_back(listener);
}

void AbstractRfidReader::removeListener(Abstractlistener *listener)
{
	for each (Abstractlistener *li in *_listeners)
	{
		if (li == listener)
		{
			_listeners->remove(listener);
			break;
		}
	}
}

void AbstractRfidReader::clearListeners()
{
	_listeners->clear();
}

void AbstractRfidReader::broadcast(CodeType type, std::string code)
{
	for each (Abstractlistener *listener in *_listeners)
	{
		listener->actionperformed(type, code);
	}
}