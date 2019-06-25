#ifndef CONFIRMDIALOG_H
#define CONFIRMDIALOG_H

#include <QDialog>
#include <qscanner/qscanner.h>

class OperatorChooser : public QDialog, public Scanable
{
    Q_OBJECT

public:
    explicit OperatorChooser(Scanable* candidate = nullptr, QWidget *parent = nullptr);

	int getId() const { return _opId; }
	static int get(Scanable* candidate = nullptr, QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private:
	void applyCurrent();

	int _opId;
};

#endif // !CONFIRMDIALOG_H
