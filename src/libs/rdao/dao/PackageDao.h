# pragma once

#include "../rdao_global.h"
#include "../entity/package.h"
#include <rcore/result.h>

class RDAO_EXPORT PackageDao
{
public:

	/**
	 * Fetch the detail of a single package type by type id.
	 * @param typeId: package type id
	 * @param packageType: out param
	 * @param withInstrumentTypes: if true, return instrument type details
	 * @return a result_t object
	 */
	result_t getPackageType(
		int typeId,
		PackageType* packageType,
		bool withInstrumentTypes = false);

	 /**
	 * Fetch a PackageType list, the element doesn't include the details of instrument types.
	 * @param packageTypes: out param
	 * @param page: page index
	 * @param count: item count per page, if -1, return all records
	 * @return a result_t object
	 */
	result_t getPackageTypeList(
		QList<PackageType> *packageTypes,
		int page = 1,
		int count = -1);

	result_t addPackageType(const PackageType &packageType);

	// package ops
	result_t getPackage(
		const QString &udi,
		Package* package,
		bool withInstruments = false);

	result_t getPackageList(
		QList<Package> *packages,
		int page = 1,
		int count = -1);

	/**
	 * Add a fresh package, instruments are binded if any of them exists.
	 * NOTE: instruments already binded are ignored.
	 * @param package: the package to be added
	 * @return a result_t object
	 */
	result_t addPackage(const Package &package);

	// TODO
	//result_t releaseInstruments(const QString &udi);

	result_t getPackTypeList(QList<PackType> *packTypes);

	/**
	 * NOTE: only valid period is allowed to be updated, other fields are ignored.
	 */
	result_t updatePackType(const PackType &packType);
};

