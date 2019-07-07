#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include "ui/loader.h"

class TopBar;
class QStackedWidget;

class MainWindow : public Ui::Source
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();
	static MainWindow *instance();

	//enum { IDX_OR, IDX_CLINIC, IDX_CSSD, IDX_HISTORY, IDX_ASSET, IDX_REPORT };
	enum { IDX_CLINIC, IDX_CSSD, IDX_OR, IDX_HISTORY, IDX_ASSET, IDX_REPORT };
	QWidget *currentPage() const;

protected:
	void showAnimated() override;

private slots:
	void swithToPage(int index);

private:
	TopBar *_topBar;
    QStackedWidget *_centerWidget;
	QWidget* _reservedWidgets[IDX_REPORT - IDX_HISTORY + 1];
};

#endif // MAINWIDGET_H
