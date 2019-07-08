#ifndef CLINICHOOSE_H
#define CLINICHOOSE_H

#include <QDialog>
#include <qscanner/qscanner.h>

namespace Ui {
	class FlatEdit;
}

class ClinicerChooser : public QDialog, public Scanable
{
    Q_OBJECT

public:
    explicit ClinicerChooser(Scanable* candidate = nullptr, QWidget *parent = nullptr, int deptId = 0);

	int getId() const { return _opId; }
	static int get(Scanable* candidate = nullptr, QWidget *parent = nullptr, int deptId = 0);

protected:
	void handleBarcode(const QString &) override;

private:
	void apply();
	Ui::FlatEdit *_opIdEdit;

	int _deptId;
	int _opId;
};

#endif // !CLINICHOOSE_H
