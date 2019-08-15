#include "mainwindow.h"
#include "leftbar.h"
#include "usepanel.h"
#include "issuepanel.h"
#include "core/application.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

MainWindow::MainWindow(QWidget *parent)
	: Ui::Source(parent) 
{

	LeftBar *leftBar = new LeftBar(this);
	leftBar->setMinimumWidth(256);
	leftBar->setMaximumWidth(256);

	QHBoxLayout *layout = new QHBoxLayout(this);
	_loader = new Ui::Loader(this);
	_loader->setSource(new UsePanel);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(leftBar);
	layout->addWidget(_loader);

	connect(leftBar, SIGNAL(currentChanged(int)), this, SLOT(swithToPage(int)));

	//QButtonGroup *btnGroup = new QButtonGroup(this);
	//btnGroup->addButton(useButton, OrPage::Use);
	//btnGroup->addButton(queryButton, OrPage::Query);
	//btnGroup->addButton(tracButton, OrPage::Track);
	//connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &OrPage::showPage);
}

MainWindow::~MainWindow() { 
}

MainWindow * MainWindow::instance() {
	return Core::app()->mainWindow();
}

QWidget * MainWindow::currentPage() const
{
	return _centerWidget->currentWidget();
}

void MainWindow::showAnimated() {
	show();
	const int duration = 500;

	QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
	setGraphicsEffect(eff);

	QPropertyAnimation *anim = new QPropertyAnimation(eff, "opacity");
	anim->setDuration(duration);
	anim->setStartValue(qreal(0));
	anim->setEndValue(qreal(1));
	connect(anim, &QPropertyAnimation::finished, this, [=] {
		setGraphicsEffect(nullptr);
	});
	anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void MainWindow::swithToPage(int index)
{
	switch (index)
	{
	case 0:
		_loader->setSource(new UsePanel);
		break;
	case 1:
		_loader->setSource(new IssuePanel);
		break;
	case 2:
		_loader->setSource(new IssuePanel);
		break;
	default:
		break;
	}
}
