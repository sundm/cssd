#pragma once

#include <QDialog>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

class OperatorChooser : public QDialog, public Scanable, public JsonHttpClient
{
    Q_OBJECT

public:
    explicit OperatorChooser(Scanable* candidate = nullptr, QWidget *parent = nullptr);

	int getId() const { return _opId; }
	QString getName() const { return _opName; }
	static int get(Scanable* candidate = nullptr, QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private:
	void search();
	void applyCurrent();

	int _opId;
	QString _opName;
};