#pragma once

#include <QThread>

class StartupThread : public QThread
{
	Q_OBJECT

public:
	StartupThread(QObject *parent);
	~StartupThread();

	bool hasError() const;

signals:
	void message(const QString &, bool isError = true);

protected:
	void run();

private:
	bool openDatabase();
	bool _hasError;
};