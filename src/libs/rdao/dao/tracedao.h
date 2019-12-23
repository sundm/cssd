# pragma once

#include "../rdao_global.h"
#include <rcore/result.h>

class PackageFlow;
class Surgery;

class RDAO_EXPORT TraceDao
{
public:
	result_t getPackageFlow(
		const QString & udi,
		int cycle,
		PackageFlow *pf
	);

	result_t getPatientSurgeries(int patientId, QList<Surgery> *surgeries);

};

