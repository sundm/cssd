#pragma once

#include <QComboBox>

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
	ProgramComboBox(int deviceId, QWidget *parent = nullptr);
	void updateEntry(bool force = false);
	int currentProgramId() const;

private:
	int _deviceId;
};

class VendorComboBox : public QComboBox
{
	Q_OBJECT

public:
	VendorComboBox(QWidget *parent = nullptr);
	void updateEntry();
};