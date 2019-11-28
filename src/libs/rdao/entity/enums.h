#pragma once

// global enums

namespace Rt
{
	enum SterilizeType {
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

	enum Gender {
		Male,
		Female,
		UnknownGender
	};

	enum Role {
		Admin = 1,
		HeadNurse,
		OrUser,
		ClinicUser,
		CssdUser,
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
		UnknownDevice
	};

}
