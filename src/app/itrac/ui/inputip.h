#pragma once

#include <QLineEdit>
#include <QEvent>  

class TLineEditIP : public QLineEdit
{
	Q_OBJECT

#define QTUTL_IP_SIZE 4

signals:
	void signalTextChanged(QLineEdit* pEdit);

public:
	TLineEditIP(QWidget *parent = 0);
	~TLineEditIP();

	void setText(const QString &strIP);
	QString text() const;

public slots:
	void slotTextChanged(QLineEdit* pEdit);

protected:
	void paintEvent(QPaintEvent *event);
	bool eventFilter(QObject *obj, QEvent *ev);

	int getIndex(QLineEdit *pEdit);
	bool isTextValid(const QString &strIP);

private:
	static QString getIPItemStr(unsigned char item);

private:
	QLineEdit *m_lineEidt[4];
};