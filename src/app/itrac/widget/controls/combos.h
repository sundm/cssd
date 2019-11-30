#pragma once

#include <QComboBox>
#include "core/net/jsonhttpclient.h"

class GenderComboBox : public QComboBox
{
	Q_OBJECT

public:
	GenderComboBox(QWidget *parent = nullptr);
};

class ProgramComboBox : public QComboBox
{
	Q_OBJECT

public:
	ProgramComboBox(int programId, QWidget *parent = nullptr);
	void updateEntry(bool force = false);
	int currentProgramId() const;
	const QString currentProgramName();
private:
	int _programId;
	JsonHttpClient _http;
};

class VendorComboBox : public QComboBox
{
	Q_OBJECT

public:
	VendorComboBox(QWidget *parent = nullptr);
	void updateEntry();

private:
	JsonHttpClient _http;
};