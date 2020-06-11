#pragma once 

#include "enums.h"
#include <QString>

struct OperatorHistory {
	int id;
	QString name;
	int recycleCount;
	int washCount;
	int packCount;
	int sterCount;
	int dispatchCount;
};

struct RecycleHistory {
	QString pkg_udi;
	QString pkg_name;
	int pkg_cycle;
	int op_id;
	QString op_name;
	QString op_time;
	QString from_dept_name;
};

struct WashHistory {
	QString device_name;
	int cycle_count;
	int cycle_total;
	QString program_name;
	QString op_name;
	QString op_time;
	QString check_op_name;
	QString check_result;
};

struct PackHistory {
	QString pkg_udi;
	QString pkg_name;
	QString op_name;
	QString op_time;
	QString check_op_name;
	QString packType;
};

struct SterHistory {
	QString device_name;
	int cycle_count;
	int cycle_total;
	QString program_name;
	QString op_name;
	QString op_time;
	
	QString phy_check_op_name;
	QString phy_check_time;
	int phy_check_result;

	QString che_check_op_name;
	QString che_check_time;
	int che_check_result;

	QString bio_check_op_name;
	QString bio_check_time;
	int bio_check_result;
};

struct DispatchHistory {
	QString pkg_udi;
	QString pkg_name;
	QString op_name;
	QString op_time;
	QString dept_name;
};