#ifndef TOPBAR_H
#define TOPBAR_H

#include <QFrame>

class SliderBar;
class TopBar : public QFrame
{
    Q_OBJECT
    
public:
    explicit TopBar(QWidget *parent = 0);

signals:
	void currentItemChanged(int, const QString&);

private slots:
	void showWarnings();

private:
	void popupUserInfo();

	SliderBar *_naviBar;
};

#endif // TOPBAR_H
