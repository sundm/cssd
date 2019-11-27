# pragma once

#include "../rdao_global.h"
#include "../entity/instrument.h"
#include <rcore/result.h>

class RDAO_EXPORT InstrumentDao
{
public:

	/**
	 * Fetch the detail of a single instrument type by type id.
	 * @param typeId: instrument type id
	 * @param insType: out param
	 * @return a result_t object
	 */
	result_t getInstrumentType(
		int typeId,
		InstrumentType* insType);

	 /**
	 * Fetch a InstrumentType list, the element doesn't include the details of instrument types.
	 * @param insTypes: out param
	 * @param page: page index
	 * @param count: item count per page, if -1, return all records
	 * @return a result_t object
	 */
	result_t getInstrumentTypeList(
		QList<InstrumentType> *insTypes,
		int page = 1,
		int count = -1);

	result_t addInstrumentType(const InstrumentType &insType);

	result_t updateInstrumentType(const InstrumentType &it);

	result_t getInstrument(
		int udi,
		Instrument* ins);

	result_t getInstrumentList(
		QList<Instrument> *instruments,
		int page = 1,
		int count = -1);

	result_t addInstrument(const Instrument &ins);
};

