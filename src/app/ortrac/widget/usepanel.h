#pragma once

#include "ui/loader.h"
#include "ui_usepanel.h"
#include <qscanner/qscanner.h>

class UsePanel : public Ui::Source, public Ui::UsePanel, public Scanable
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
};