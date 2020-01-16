# pragma once

#include "../rdao_global.h"
#include "../entity/package.h"
#include <rcore/result.h>

class PackageTypeFilter;

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
	 * Fetch a package type list, the element doesn't include the details of instrument types.
	 * @param pts[OUT]: package types returned
	 * @param total[OUT]: the total number of package types, if set to nullptr, all is returned without pagination
	 * @param page: page index, starts from 1; values less than 1 are treated as 1
	 * @param count: item count per page, the default also the minimum value is 20
	 * @return a result_t object
	 */
	result_t getPackageTypeList(
		QList<PackageType> *pts,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	//result_t getPackageTypeList(
	//	QList<PackageType> *pts,
	//	const PackageTypeFilter &filter);

	result_t addPackageType(const PackageType &packageType, int *typeId);

	// package ops
	result_t getPackage(
		const QString &udi,
		Package* package,
		bool withInstruments = false);

	/**
	 * Fetch a udi package list, the element doesn't include the details of instruments.
	 * @param pkgs[OUT]: udi packages returned
	 * @param total[OUT]: the total number of udi packages, if set to nullptr, all is returned without pagination
	 * @param page: page index, starts from 1; values less than 1 are treated as 1
	 * @param count: item count per page, the default also the minimum value is 20
	 * @return a result_t object
	 */
	result_t getPackageList(
		QList<Package> *pkgs,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	/**
	 * Add a fresh package, instruments are bound if any of them exists.
	 * NOTE: instruments already bound are ignored.
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

	result_t getPackageQualityControl(const Package &pkg, PackageQualityControl *pqc);

	result_t setPackagePhoto(const QString& udi, const QString& photo);
	result_t getPackagePhoto(const QString& udi, QString *photo);

	result_t setPackagePhoto(int typeId, const QString& photo);
	result_t getPackagePhoto(int typeId, QString *photo);
};

