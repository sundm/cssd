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
		Trace
	};

	enum Asset {
		DEVICE,
		PROGRAM,
		PACKAGE,
		INSTRUMENT,
		DEPT,
		USER,
		COST,
		RECALL
	};

	enum Verdict {
		Success,
		Failed,
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

}

#endif // ITRACNAMESPACE_H

