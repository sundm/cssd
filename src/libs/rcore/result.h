#pragma once

#include "rcore_global.h"
#include <QString>

class RCORE_EXPORT result_t
{
public:
	result_t() : _msg(nullptr) {}

	result_t(const char* m) : _msg(nullptr) {
		strcpy_p(m);
	}

	result_t(const QString &s) : _msg(nullptr) {
		if (!s.isEmpty()) {
			QByteArray ba = s.toUtf8();
			_msg = new char[ba.length() + 1];
			memcpy(_msg, ba.data(), ba.length());
			_msg[ba.length()] = '\0';
		}
	}

	result_t(const result_t &other) : _msg(nullptr) {
		strcpy_p(other._msg);
	}

	result_t& operator=(const result_t &other) {
		_msg = nullptr;
		if (this != &other) strcpy_p(other._msg);
		return *this;
	}

	result_t(result_t &&other) {
		//std::swap(*this, other);
		_msg = other._msg;
		other._msg = nullptr; // TODO: memleak?
	}

	result_t& operator=(result_t &&other) {
		//std::swap(*this, other);
		_msg = other._msg;
		other._msg = nullptr;
		return *this;
	}

	virtual ~result_t() { delete _msg; }

	bool isOk() const { return nullptr == _msg; }
	QString	msg() const { return _msg ? QString(_msg) : QString(); }

private:
	char *_msg;

	void strcpy_p(const char *src) {
		if (src) {
			int len = strlen(src);
			_msg = new char[len + 1];
			strcpy_s(_msg, len + 1, src);
		}
	}
};
