#include "leftbar.h"
#include <QButtonGroup>

LeftBar::LeftBar(QWidget *parent)
	: QFrame(parent), _btns(new QButtonGroup(this))
{
	setupUi(this);

	_btns->addButton(useButton, UseState);
	_btns->addButton(historyButton, HistoryState);
	_btns->addButton(traceButton, TraceState);

	connect(_btns, SIGNAL(buttonClicked(int)),
		this, SLOT(updateButtonGroupState(int)));
}

LeftBar::~LeftBar()
{
}

void LeftBar::updateButtonGroupState(int id)
{
	if (_btns->checkedId() != id) {
		emit currentChanged(id);
	}
}
