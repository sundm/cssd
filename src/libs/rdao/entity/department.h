#pragma once

#include <QString>

struct Department {
	int id;
	QString name;
	QString pinyin;
	bool isSurgical;
	QString phone;
	QString location;

	Department() : id(-1), isSurgical(false) {}
};
