#include "mainwindow.h"
#include "topbar.h"
#include "cssdpage.h"
#include "clinicpage.h"
#include "historywidget.h"
#include "widget/assetpage.h"
#include "analysepage.h"

#include "core/application.h"

#include <QVBoxLayout>
#include <QStackedWidget>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QGraphicsOpacityEffect>

MainWindow::MainWindow(QWidget *parent)
	: Ui::Source(parent) {

	//setMinimumHeight(480);

	_topBar = new TopBar(this);
	_centerWidget = new QStackedWidget(this);

	// ownership of the widget is passed to center widget
	//_centerWidget->addWidget(new QWidget);
	_centerWidget->addWidget(new ClinicPage);
	_centerWidget->addWidget(new CssdPage);
	for (int i = IDX_HISTORY; i <= IDX_REPORT; ++i) {
		// for better performance and less memory cost, left the other pages blank,
		// The content widgets are created when required.
		_centerWidget->addWidget(new QWidget());
		_reservedWidgets[i - IDX_HISTORY] = nullptr;
	}
	_centerWidget->setCurrentIndex(0);

	QVBoxLayout *m_pMainLayout = new QVBoxLayout(this);
	m_pMainLayout->addWidget(_topBar);
	m_pMainLayout->addWidget(_centerWidget);
	m_pMainLayout->setSpacing(0);
	m_pMainLayout->setContentsMargins(0, 0, 0, 0);

	connect(_topBar, SIGNAL(currentItemChanged(int, const QString &)),
		this, SLOT(swithToPage(int, const QString &)));
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

void MainWindow::setTopbarEnabled(bool b) {
	_topBar->setEnabled(b);
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

void MainWindow::swithToPage(int index, const QString &item)
{
	Q_UNUSED(item);

	// replace the page if needed
	if (index >= IDX_HISTORY && nullptr == _reservedWidgets[index - IDX_HISTORY]) {
		// Seems QStackedWidget knows a widget is removed when we calling `delete` 
		// on the widget (QStackedWidget::count() is decreased by 1), so we didn't
		// call QStackedWidget::removeWidget() here.
		delete _centerWidget->widget(index);

		switch (index) {
		case IDX_HISTORY:
			_centerWidget->insertWidget(index, new HistoryWidget);
			break;
		case IDX_ASSET:
			_centerWidget->insertWidget(index, new AssetPage);
			break;
		case IDX_REPORT:
			_centerWidget->insertWidget(index, new AnalysePage);
			break;
		default:
			break;
		}
		_reservedWidgets[index - IDX_HISTORY] = _centerWidget->widget(index);
	}
	_centerWidget->setCurrentIndex(index);
}
