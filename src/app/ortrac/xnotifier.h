#pragma once

#include <QDialog>

class QLabel;
class XNotifier : public QDialog
{
	Q_OBJECT

public:
	static void warn(const QString &text, int msecDisplayTime = -1, QWidget *parent = nullptr);
	static void error(QWidget *parent, const QString &text, int msecDisplayTime);
	static void info(QWidget *parent, const QString &text, int msecDisplayTime);

//make contructor private to prevent instantiation
private:
	XNotifier(QWidget *parent);
	~XNotifier();

	void setText(const QString &);
	void setIcon(const QString &);
	void setDisplayTime(int);

private:
	QLabel * _icon;
	QLabel * _text;
	int _displayTime;
};
