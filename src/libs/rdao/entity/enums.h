#pragma once

// global enums

namespace Rt
{
	enum SterilizeMethod {
		BothTemperature,
		LowTemperature,
		HighTemperature,
		UnknownSterilizeType = -1
	};

	enum Status {
		Normal,
		Frozen,
		Deleted
	};

	enum DeviceStatus {
		Idle,
		Running,
		Forbidden
	};

	enum Gender {
		Male,
		Female,
		UnknownGender
	};

	enum Role {
		Admin = 1,
		HeadNurse,
		CssdUser,
		OrUser,
		ClinicUser,
		UnknownRole
	};

	enum PackageCategory {
		SurgicalPackage,
		ClinicalPackage,
		ExternalPackage,
		DressingPackage,
		UniversalPackage,
		UnknownPackage
	};

	enum InstrumentCategory {
		NormalInstrument,
		ImplantedInstrument
	};

	enum DeviceCategory {
		Washer,
		Sterilizer,
		UnknownDeviceCategory
	};

	enum FlowStatus {
		UnknownFlowStatus,
		Washed,
		WashChecked,
		Packed,
		Sterilized,
		SterilizeResultChecked, // set even biological result is not committed
		Dispatched,
		Received,
		SurgeryPreChecked,
		SurgeryPostChecked,
		Recycled,
		Recalled = 99
	};

	enum SterilizeVerdict{
		Unchecked,
		Qualified,
		Unqualified,
		Uninvolved
	};

	enum SurgeryStatus {
		WaitingForCheck,
		PreChecked,
		PostChecked
	};
}
