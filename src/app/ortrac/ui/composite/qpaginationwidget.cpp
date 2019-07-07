#include "qpaginationwidget.h"
#include <QLineEdit>
#include <QToolButton>
#include <QHBoxLayout>
#include <QRegExpValidator>

namespace {
	unsigned lenDigits(const int n)
	{
		if (-10 < n && n < 10) return 1;
		return 1 + lenDigits(n / 10);
	}
}

QPaginationWidget::QPaginationWidget(QWidget *parent)
	: QWidget(parent), _total(0), _current(0)
{
	homePageButton = createToolButton(":/res/home.png", tr("第一页"));
	lastPageButton = createToolButton(":/res/end.png", tr("最后一页"));
	previousPageButton = createToolButton(":/res/backward.png", tr("上一页"));
	nextPageButton = createToolButton(":/res/forward.png", tr("下一页"));

	pageNavigationLineEdit = new QLineEdit;
	pageNavigationLineEdit->setFixedSize(QSize(100, 22));
	pageNavigationLineEdit->setAlignment(Qt::AlignHCenter);
	pageNavigationLineEdit->setValidator(new QRegExpValidator(pageNavigationLineEdit));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(homePageButton);
	layout->addWidget(previousPageButton);
	layout->addWidget(pageNavigationLineEdit);
	layout->addWidget(nextPageButton);
	layout->addWidget(lastPageButton);
	layout->setSpacing(2);
	layout->setContentsMargins(0, 9, 9, 9);
	layout->setSizeConstraint(QLayout::SetFixedSize);

	connect(homePageButton, SIGNAL(clicked()), this, SLOT(onHomePageButtonClicked()));
	connect(previousPageButton, SIGNAL(clicked()), this, SLOT(onPreviousPageButtonClicked()));
	connect(nextPageButton, SIGNAL(clicked()), this, SLOT(onNextPageButtonClicked()));
	connect(lastPageButton, SIGNAL(clicked()), this, SLOT(onLastPageButtonClicked()));
	connect(pageNavigationLineEdit, SIGNAL(returnPressed()), this, SLOT(onPageNavigationLineEditChanged()));
	connect(this, SIGNAL(currentPageChanged_p()), this, SLOT(updatePageNavigation()));
	connect(this, SIGNAL(totalPageChanged_p()), this, SLOT(updatePageNavigation()));

	enablePageDownAction(false);
	enablePageUpAction(false);
}


QToolButton* QPaginationWidget::createToolButton(const QString &iconFileName, const QString &toolTip)
{
	QToolButton *button = new QToolButton;
	button->setIcon(QIcon(iconFileName));
	button->setIconSize(QSize(20, 20));
	button->setAutoRaise(true);
	button->setToolTip(toolTip);
	return button;
}

void QPaginationWidget::setTotalPages(int total)
{
	if (_total == total)
		return;
	if (total < 0)
		total = 0;

	// total pages changed
	_total = total;

	if (_current > total){ // maybe a stricter filter is setup
		_current = 1;
		emit currentPageChanged_p();
	}

	if (0 == _current && total > 0){
		_current = 1;
	}

	emit totalPageChanged_p();

	pageNavigationLineEdit->setValidator(0);
	QRegExp regx(QString("^\\d{,%1}( ?/ ?\\d{,%1})?$").arg(lenDigits(total)));
	pageNavigationLineEdit->setValidator(new QRegExpValidator(regx, pageNavigationLineEdit));
}

void QPaginationWidget::onHomePageButtonClicked()
{
	_current = 1;
	emit currentPageChanged(_current);
	emit currentPageChanged_p();
}

void QPaginationWidget::onLastPageButtonClicked()
{
	_current = _total;
	emit currentPageChanged(_current);
	emit currentPageChanged_p();
}

void QPaginationWidget::onPreviousPageButtonClicked()
{
	_current--;
	emit currentPageChanged(_current);
	emit currentPageChanged_p();
}

void QPaginationWidget::onNextPageButtonClicked()
{
	_current++;
	emit currentPageChanged(_current);
	emit currentPageChanged_p();
}

void QPaginationWidget::onPageNavigationLineEditChanged()
{
	QString pageString(pageNavigationLineEdit->text());
	int slashPos = pageString.indexOf(QChar('/'));
	int page;

	if (-1 == slashPos)
		page = pageString.toInt();
	else
		page = pageString.left(slashPos).toInt();

	if (page > 0 && page <= _total && page != _current){
		_current = page;
		emit currentPageChanged(_current);
	}

	emit currentPageChanged_p();
}

void QPaginationWidget::enablePageDownAction(bool enable)
{
	lastPageButton->setEnabled(enable);
	nextPageButton->setEnabled(enable);
}

void QPaginationWidget::enablePageUpAction(bool enable)
{
	homePageButton->setEnabled(enable);
	previousPageButton->setEnabled(enable);
}

void QPaginationWidget::updatePageNavigation()
{
	pageNavigationLineEdit->setText(QString("%1 / %2").arg(_current).arg(_total));
	enablePageDownAction(_current < _total);
	enablePageUpAction(_current > 1);
}

int QPaginationWidget::totalPages() const
{
	return _total;
}

int QPaginationWidget::currentPage() const
{
	return _current;
}
