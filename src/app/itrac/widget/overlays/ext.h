#pragma once

#include "cssd_overlay.h"
#include "ui/views.h"

class QStandardItemModel;

namespace Internal {
	class ExtView : public PaginationView
	{
		enum Column {
			Name,
			Vendor,
			Sender,
			SenderPhone,
			Receiver,
			ReceiveTime,
			Patient,
			Gender,
			Age,
			Identifier,
			Doctor,
			Barcode
		};
	public:
		ExtView(QWidget *parent = Q_NULLPTR);
		void load(int page = 0);
	private:
		QStandardItemModel * _model;
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

private:
	Internal::ExtView * _view;
};
