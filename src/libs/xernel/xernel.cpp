#include "xernel.h"

void XERNEL_EXPORT trimLeadingChar(QString &s, const char &c)
{
	int i = 0;
	while (i < s.length() && s[i] == c) {
		i++;
	}
	s.remove(0, i);
}
