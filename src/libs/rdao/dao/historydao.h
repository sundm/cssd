# pragma once

#include "../rdao_global.h"
#include "../entity/history.h"
#include <rcore/result.h>

class RDAO_EXPORT HistoryDao
{
public:
	result_t getRecycleHistoryList(const QDate &from_date, const QDate &to_date, 
		const int op_id, const int detp_id, QList<RecycleHistory> *recycleHis,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t getWashHistoryList(const QDate &from_date, const QDate &to_date,
		const int op_id, const int device_id, QList<WashHistory> *washHis,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t getPackHistoryList(const QDate &from_date, const QDate &to_date,
		const int op_id, const int pack_type_id, QList<PackHistory> *packHis,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t getSterHistoryList(const QDate &from_date, const QDate &to_date,
		const int op_id, const int device_id, QList<SterHistory> *sterHis,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t getDispatchHistoryList(const QDate &from_date, const QDate &to_date,
		const int op_id, const int detp_id, QList<DispatchHistory> *dispatchHis,
		int *total = nullptr,
		int page = 1,
		int count = 20);

	result_t getOperatorHistoryList(const int year, const int month, QList<OperatorHistory> *opHistories);
};

