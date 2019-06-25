#ifndef LOGSEARCHEDITOR_H
#define LOGSEARCHEDITOR_H

#include "xui_global.h"
#include <QLineEdit>

#define QT_VER_LE_WITH_CLEARBTN QT_VERSION_CHECK(5, 2, 0)

class QPushButton;

class XUI_EXPORT SearchEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit SearchEdit(QWidget *parent = 0);
	
public slots:
#if (QT_VERSION < QT_VER_LE_WITH_CLEARBTN)
	void clear();
#endif

private slots:
    void doDelayedSearch(const QString &text);
	void onTypingTimeout();

private:
#if (QT_VERSION < QT_VER_LE_WITH_CLEARBTN)
    QPushButton *_clearButton;
#endif
	QTimer *_type_timer;
};

#endif // LOGSEARCHEDITOR_H
