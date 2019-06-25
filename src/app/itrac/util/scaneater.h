#ifndef SCANEATER_H
#define SCANEATER_H

#include <QObject>

class QTimer;
class ScanEater : public QObject
{
	Q_OBJECT

public:
	ScanEater(QObject *parent);

protected:
	bool eventFilter(QObject *obj, QEvent *event);

signals:
	void dataReceived(const QString &);

private slots:
	void resetKeyPressStatus();

private:
	QTimer * _timer;
	QString _curText;
	bool _receiving;
};

#endif // !SCANEATER_H

