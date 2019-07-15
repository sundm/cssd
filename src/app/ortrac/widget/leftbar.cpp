#include "leftbar.h"
#include <QButtonGroup>

LeftBar::LeftBar(QWidget *parent)
	: QFrame(parent), _btns(new QButtonGroup(this)), _currentCheckedId(-1)
{
	setupUi(this);

	_btns->addButton(useButton, UseState);
	_btns->addButton(historyButton, HistoryState);
	//_btns->addButton(traceButton, TraceState);

	connect(_btns, SIGNAL(buttonClicked(int)),
		this, SLOT(updateButtonGroupState(int)));
}

LeftBar::~LeftBar()
{
}

void LeftBar::updateButtonGroupState(int id)
{
	if (_currentCheckedId != id) {
		_currentCheckedId = id;
		emit currentChanged(id);
	}
}
