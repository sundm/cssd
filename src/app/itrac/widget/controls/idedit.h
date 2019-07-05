#pragma once

#include "ui/inputfields.h"

class IdCompleter;
class IdEdit : public Ui::FlatEdit
{
	Q_OBJECT
public:
	IdEdit(QWidget *parent = nullptr);

	void setCompleter(IdCompleter *completer);
	void reset();
	int currentId() const { return _curId; };
	QString currentName() const { return _curName; };

	void setCurrentIdPicked(int id, const QString &name);

signals:
	void error(const QString &);
	void changed(int id);

protected:
	void focusInEvent(QFocusEvent *) override;
	void focusOutEvent(QFocusEvent *) override;

protected slots:
	virtual void complete(const QModelIndex & index);
	void popupFullList();
	void onIdPicked(int, const QString &);

protected:
	int _curId;
	QString _curName;
};

class DeptEdit : public IdEdit {
	Q_OBJECT
public:
	DeptEdit(QWidget *parent = nullptr);
	void load();
};

class PackageEdit : public IdEdit {
	Q_OBJECT
public:
	PackageEdit(QWidget *parent = nullptr);

public slots:
	void loadForDepartment(int deptId);
	void loadForCategary(const QString &catogary);
};