#include "assets.h"
#include "itracnamespace.h"

#include "core/net/url.h"
#include "core/net/jsonhttpclient.h"

namespace {
	inline int getPackageState(const QString &s) {
		if (s == "P") return itrac::Packed;
		if (s == "S") return itrac::Sterilized;
		if (s == "ST") return itrac::SteExamined;
		if (s == "I") return itrac::Dispatched;
		if (s == "U") return itrac::Used;
		if (s == "R") return itrac::Recycled;
		return itrac::UnknownState;
	}
}

void Plate::fetchOnce(const QString &id, const std::function<void(Plate*)> &fn) {
	Plate *plate = new Plate;

	QByteArray data("{\"plate_id\":");
	data.append(id).append('}');

	JsonHttpClient *http = new JsonHttpClient;
	http->post(url(PATH_PLATE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		do {
			if (!resp.success()) {
				plate->err = QString("无法获取编号[%1]的网篮信息").arg(id);
				break;
			}

			QList<QVariant> plates = resp.getAsList("plates");
			if (plates.isEmpty()) {
				plate->err = QString("编号[%1]的网篮不在系统资产目录中").arg(id);
				break;
			}

			QVariantMap map = plates[0].toMap();
			plate->idle = "1" == map["is_finished"].toString();
			plate->name = map["plate_name"].toString();
			plate->id = id.toInt();
			break;
		} while (1);

		fn(plate);
		delete plate;
		//delete http; //todo
	});
}

Device::Device()
	: cycleToday(-1)
	, cycleSum(-1)
	, type(-1)
	, state(Device::Disabled)
	, bdResult(itrac::NotInvolved)
{};

int Device::tranlateState(const QString &s) {
	if (s == "0") return Device::Idle;
	if (s == "2") return Device::Running;
	return Device::Disabled;
}

DumpPackage::DumpPackage()
	:deptId(-1)
	, state(itrac::Packed)
	, expired(false)
	, steQualified(false)
{}

void DumpPackage::fetchOnce(const QString &id, const std::function<void(DumpPackage*)> &fn) {
	DumpPackage *package = new DumpPackage;

	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	JsonHttpClient *http = new JsonHttpClient;
	http->post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			package->err = QString("无法获取包信息: ").append(resp.errorString());
			return;
		}

		package->name = resp.getAsString("package_type_name"); // moved
		package->id = resp.getAsInt("package_type_id");
		package->expireDate = resp.getAsString("valid_date");
		package->packName = resp.getAsString("pack_type_name");
		package->usedBy = resp.getAsString("from_department_name");
		package->deparment = resp.getAsString("department_name");
		package->deptId = resp.getAsInt("deparment_id");
		package->state = getPackageState(resp.getAsString("state"));
		package->expired = resp.getAsBool("expired");
		package->steQualified = resp.getAsBool("sterilize_qualified");

		fn(package);
		delete package;
		delete http; // todo
	});
}
