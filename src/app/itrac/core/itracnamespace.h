#ifndef ITRACNAMESPACE_H
#define ITRACNAMESPACE_H

namespace itrac {

	enum CssdAction {
		OrRecycleAction,
		ClinicRecycleAction,
		BorrowRecycleAction,
		NoBarcodeRecycleAction,
		ExtInstrumentRecycleAction,
		WashAction,
		WashExamAction,
		PackAction,
		SterileAction,
		BDAction,
		SterileExamAction,
		OrDispatchAction,
		ClinicDispatchAction,
		BorrowDispatchAction
	};

	enum Flow {
		Recycle,
		Wash,
		WashCheck,
		Pack,
		Sterile,
		SterileCheck,
		Dispatch,
		Use,
		PackageTrace,
		PatientTrace
	};

	enum Asset {
		DEVICE,
		PROGRAM,
		PACKAGE,
		PACKAGEID,
		PACKTYPE,
		INSTRUMENT,
		INSTRUMENTID,
		DEPT,
		USER,
		RECALL
	};

	enum Verdict {
		Failed,
		Success,
		NotChecked,
		NotInvolved
	};

	enum PackageState {
		Packed,
		Sterilized,
		SteExamined,
		Dispatched,
		Used,
		Recycled,
		Recalled,
		UnknownState
	};

	enum DeptCatogary {
		OprationRoom,
		CSSD,
		Clinic
	};

	enum DeviceType { Washer, Sterilizer };
}

#endif // ITRACNAMESPACE_H

