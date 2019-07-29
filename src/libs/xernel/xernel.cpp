#include "xernel.h"

void XERNEL_EXPORT trimLeadingChar(QString &s, const char &c)
{
	// s.remove(QRegExp("^[0]*")) // this is lower
	int i = 0;
	while (i < s.length() && s[i] == c) {
		i++;
	}
	s.remove(0, i);
}
