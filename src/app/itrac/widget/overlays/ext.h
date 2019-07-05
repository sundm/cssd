#pragma once

#include "cssd_overlay.h"
#include "ui/views.h"

class QStandardItemModel;
class QButtonGroup;
class QRadioButton;
class QDateEdit;

namespace Composite {
	class Title;
}

namespace Internal {
	class ExtView : public PaginationView
	{
		enum Column {
			Name,
			Doctor,
			Patient,
			Identifier,
			Age,
			Gender,
			Barcode
		};
	public:
		ExtView(QWidget *parent = Q_NULLPTR);
		void load(const int orderId, int page = 0);
	private:
		QStandardItemModel * _model;
	};

	class ExtOrderView : public PaginationView
	{
		Q_OBJECT

		enum Column {
			Vendor,
			Sender,
			SenderPhone,
			Receiver,
			ReceiveTime,
			Repaier,
			RepayTime,
			RepaierName,
			RepaierPhone
		};
	public:
		ExtOrderView(QWidget *parent = Q_NULLPTR);
		void load(const QDate& fromDate, const QDate& endDate, int page = 0);
		const QVariantMap* itemMap() { return _map; };
	private slots:
		void onRowClicked(const QModelIndex &);
	signals:
		void sendOrderId(int id);
	private:
		QStandardItemModel *_model;
		QVariantMap *_map;
	};
}

class ExtManagePanel : public CssdOverlayPanel
{
	Q_OBJECT

public:
	ExtManagePanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;
	void reset();

private slots:
	void addReception();
	void addReturn();
	void loadOrder(int);
	void onDateButtonToggled(int, bool);

private:
	Internal::ExtView * _view;
	Internal::ExtOrderView * _orderView;
	Composite::Title *_title;

	QButtonGroup * _dateButtons;
	QRadioButton * _weekButton;
	QRadioButton * _monthButton;
	QRadioButton * _yearButton;
	QRadioButton * _customButton;
	QDateEdit * _startDateEdit;
	QDateEdit * _endDateEdit;
};
