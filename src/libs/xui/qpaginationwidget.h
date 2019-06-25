#ifndef QPAGINATIONWIDGET_H
#define QPAGINATIONWIDGET_H

#include <QWidget>

class QLineEdit;
class QToolButton;
class QPaginationWidget : public QWidget
{
	Q_OBJECT

public:
	QPaginationWidget(QWidget *parent);
	~QPaginationWidget();

	void setTotalPages(int total);
	int totalPages() const;
	int currentPage() const;

signals:
	void currentPageChanged(int newPage);
	void currentPageChanged_p();
	void totalPageChanged_p();

private slots:
	void onHomePageButtonClicked();
	void onPreviousPageButtonClicked();
	void onNextPageButtonClicked();
	void onLastPageButtonClicked();
	void onPageNavigationLineEditChanged();
	void updatePageNavigation();

private:
	QToolButton* createToolButton(const QString &iconFileName, const QString &toolTip);
	inline void enablePageDownAction(bool);
	inline void enablePageUpAction(bool);

private:
	QToolButton *homePageButton;
	QToolButton *lastPageButton;
	QToolButton *previousPageButton;
	QToolButton *nextPageButton;
	QLineEdit *pageNavigationLineEdit;

	int _total;
	int _current;
};

#endif // QPAGINATIONWIDGET_H
