#pragma once

#include "ui/loader.h"
#include "ui_usepanel.h"
#include "core/net/jsonhttpclient.h"
#include <qscanner/qscanner.h>

class UsePanel : public Ui::Source, public Ui::UsePanel, public Scanable, public JsonHttpClient
{
	Q_OBJECT

public:
	UsePanel(QWidget *parent = Q_NULLPTR);
	~UsePanel();

	void handleBarcode(const QString &) override;

private slots:
	void addEntry();
	void remove();
	void submit();
	void reset();
	void abnormal();
	void showImageViewer();
	void showDetail(const QModelIndex &);

private:
	void showImage(const QString &);
};