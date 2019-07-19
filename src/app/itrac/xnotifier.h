#pragma once

#include <QDialog>

class QLabel;
class QTimer;

class XNotifier : public QDialog
{
	Q_OBJECT

public:
	static void warn(const QString &text, int msecDisplayTime = -1, QWidget *parent = nullptr);
	static void error(QWidget *parent, const QString &text, int msecDisplayTime);
	static void info(QWidget *parent, const QString &text, int msecDisplayTime);

	void startTimer();

private slots:
	void update();

//make contructor private to prevent instantiation
private:
	XNotifier(QWidget *parent);
	~XNotifier();

	void setText(const QString &);
	void setIcon(const QString &);
	void setDisplayTime(int);

private:
	QPushButton *_closeButton;
	QLabel * _icon;
	QLabel * _text;
	int _displayTime = 3;

	QTimer *m_pTimer;
};
