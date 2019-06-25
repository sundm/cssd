#include "SearchEdit.h"
#include <QTimer>
#include <QToolButton>
#include <QHBoxLayout>

SearchEdit::SearchEdit(QWidget *parent)
	: QLineEdit(parent), _type_timer(new QTimer(this))
{
	// show search icon on the left
	setStyleSheet("QLineEdit{background-image:url(:/xui/search.png);"
		"background-repeat:no-repeat;"
		"background-position:center left;"
		"padding-left:20px;}"
	);
	setMinimumHeight(32);

	// try to start a new search whenever user types something, but the real search
	// should be fired when the user has finished typing for a short period of time,
	// thus the search is delayed for the typing can be very fast. 
	// NOTE: `textEdited` is not emitted when the text is changed programmatically,
	// e.g. setText()
	connect(this, &QLineEdit::textEdited, this, &SearchEdit::doDelayedSearch);

	// do search when typing is timeout
	_type_timer->setSingleShot(true);
	connect(_type_timer, &QTimer::timeout, this, &SearchEdit::onTypingTimeout);

	// setup the clear button, 
	// from 5.2 on, qt introduces QLineEdit::setClearButtonEnabled
#if (QT_VERSION >= QT_VER_LE_WITH_CLEARBTN)
	setClearButtonEnabled(true);
#else
	_clearButton = new QPushButton(this);
	//_clearButton->setFocusPolicy(Qt::StrongFocus);
	_clearButton->setCursor(Qt::ArrowCursor);
	_clearButton->setIcon(QIcon(":/xui/clear.png"));
	_clearButton->setIconSize(QSize(20, 20));
	_clearButton->setStyleSheet("QToolButton{"
		"background:none;"
		"border:none;"
		"padding:0px;}");
	_clearButton->hide();

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addStretch();
	layout->addWidget(_clearButton);
	layout->setContentsMargins(3, 0, 3, 0);
	layout->setSpacing(0);
	setLayout(layout);

	QMargins margins(20, 0, 20, 0);
	setTextMargins(margins);

	connect(_clearButton, &QAbstractButton::clicked, this, &SearchEdit::clear);
#endif
}

#if (QT_VERSION < QT_VER_LE_WITH_CLEARBTN)
void SearchEdit::clear()
{
	setText(""); // = QLineEdit::clear(); 
	_clearButton->hide();
}
#endif

void SearchEdit::doDelayedSearch(const QString& text)
{
#if (QT_VERSION < QT_VER_LE_WITH_CLEARBTN)
	_clearButton->setVisible(!text.isEmpty());
#endif
	_type_timer->start(300);
}

void SearchEdit::onTypingTimeout()
{
}

