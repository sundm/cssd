#pragma once

#include "rcore_global.h"
#include <QString>

class RCORE_EXPORT result_t
{
public:
	result_t() : _msg(nullptr) {}
	result_t(char const* m) : _msg(m) {}
	//explicit result_t(char const* m) : _msg(m) {}

	bool isOk() const { return nullptr == _msg; }
	QString	msg() const { return _msg ? QString(_msg) : QString(); }

private:
	char const* _msg;
};
