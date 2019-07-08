#include "titles.h"
#include "ui/labels.h"
#include "ui/buttons.h"

#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

namespace Composite {

Title::Title(const QString &title, bool closable, QWidget *parent)
	: QFrame(parent)
	, _title(new Ui::Title(title))
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addWidget(_title);
	layout->addStretch(1);

	if (closable) {
		QToolButton *closeButton = new QToolButton;
		closeButton->setIcon(QIcon(":/res/close-m.png"));
		layout->addWidget(closeButton);
		//setMinimumWidth(800);
		connect(closeButton, SIGNAL(clicked()), this, SIGNAL(canceled()));
	}
}

void Title::setTitle(const QString &title) {
	_title->setText(title);
}

} // namespace Composite {
