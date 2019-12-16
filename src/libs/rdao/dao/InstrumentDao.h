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
	 * Fetch a instrument type list.
	 * @param its[OUT]: instrument types returned
	 * @param total[OUT]: the total number of instrument types, if set to nullptr, all is returned without pagination
	 * @param page: page index, starts from 1; values less than 1 are treated as 1
	 * @param count: item count per page, the default also the minimum value is 20
	 * @return a result_t object
	 */
	result_t getInstrumentTypeList(
		QList<InstrumentType> *its,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t addInstrumentType(const InstrumentType &insType);

	result_t updateInstrumentType(const InstrumentType &it);

	result_t getInstrument(
		const QString& udi,
		Instrument* ins);

	/**
	 * Fetch a udi instrument list.
	 * @param its[OUT]: udi instruments returned
	 * @param total[OUT]: the total number of udi instruments, if set to nullptr, all is returned without pagination
	 * @param page: page index, starts from 1; values less than 1 are treated as 1
	 * @param count: item count per page, the default also the minimum value is 20
	 * @return a result_t object
	 */
	result_t getInstrumentList(
		QList<Instrument> *instruments,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t addInstrument(const Instrument &ins);
	result_t updateInstrument(const Instrument &it);
};

