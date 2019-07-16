#include "combos.h"
#include "core/net/url.h"

GenderComboBox::GenderComboBox(QWidget *parent /*= nullptr*/)
	: QComboBox(parent)
{
	addItem("男", "M");
	addItem("女", "F");
	setCurrentIndex(-1);
}

ProgramComboBox::ProgramComboBox(int deviceId, QWidget *parent)
	: QComboBox(parent), _deviceId(deviceId)
{
}

void ProgramComboBox::updateEntry(bool force)
{
	if (count() && !force) return;

	clear();

	QByteArray data("{\"device_id\":");
	data.append(QString::number(_deviceId)).append('}');
	_http.post(url(PATH_PROGRAM_SUPPORT), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(this, QString("无法获取清洗程序列表: ").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for(auto &program: programs) {
			QVariantMap map = program.toMap();
			addItem(map["program_name"].toString(), map["device_program_id"]);
		}
	});
}

int ProgramComboBox::currentProgramId() const {
	return currentData().toInt();
}

VendorComboBox::VendorComboBox(QWidget *parent /*= nullptr*/)
	: QComboBox(parent)
{
}

void VendorComboBox::updateEntry() {
	clear();

	QByteArray data("{}");
	_http.post(url(PATH_VENDOR_SEARCH), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			return;
		}

		QList<QVariant> vendors = resp.getAsList("suppliers");
		for (auto &vendor : vendors) {
			QVariantMap map = vendor.toMap();
			addItem(map["name"].toString(), map["id"]);
		}
		setCurrentIndex(-1);
	});
}
