#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QFrame>
#include <QPainter>
#include <QStyleOption>
#include "filterwidget.h"
#include "filtergroup.h"

FilterWidget::FilterWidget(QWidget *parent)
	: QWidget(parent), _layout(new QVBoxLayout(this)), _filterGroup(nullptr)
{
	QFrame *line = new QFrame;
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);

	_layout->addWidget(line);

	QHBoxLayout *hlayout = new QHBoxLayout();
	//hlayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

	QSize minSize(200, 32);
	QPushButton *searchButton = new QPushButton("查询", this);
	searchButton->setMinimumSize(minSize);
	connect(searchButton, &QPushButton::clicked, this, &FilterWidget::doSearch);

	_resetButton = new QPushButton("重置", this);
	_resetButton->setMinimumSize(minSize);

	hlayout->addWidget(searchButton);
	hlayout->addWidget(_resetButton);
	hlayout->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding));

	_layout->addLayout(hlayout);
	_layout->setSpacing(10);
}

FilterWidget::FilterWidget(FilterGroup *filterGroup, QWidget *parent)
	:FilterWidget(parent) // need c++11
{
	setFilterGroup(filterGroup);
}

FilterWidget::~FilterWidget()
{
}

/*
If we subclass from QWidget, we need to provide a paintEvent for
our custom QWidget as below. Otherwise, the default background is
transparent since QWidget::paintEvent does nothing.
The code is a no-operation if there is no stylesheet set.
*/
void FilterWidget::paintEvent(QPaintEvent *event)
{
	/*Q_UNUSED(event);
	QPainter painter(this);
	painter.setPen(Qt::NoPen);
	painter.setBrush(QColor(40, 42, 46));
	painter.drawRect(0, 0, this->width(), this->height());*/
	QStyleOption opt;
	opt.init(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void FilterWidget::setFilterGroup(FilterGroup *fg)
{
	if (_filterGroup) {
		_layout->takeAt(0);
		delete _filterGroup;
	}
	_filterGroup = fg;
	_layout->insertWidget(0, _filterGroup);

	connect(_resetButton, &QPushButton::clicked, _filterGroup, &FilterGroup::reset);
}

void FilterWidget::doSearch()
{
	Filter f = _filterGroup->filter();
	emit filterReady(f);
}
