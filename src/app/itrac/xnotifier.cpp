#include "xnotifier.h"
#include "widget/mainwindow.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QSpacerItem>
#include <QRect>
#include <xui/xdimmingeffect.h>

void XNotifier::warn(const QString &text, int msecDisplayTime, QWidget *parent)
{
	if (!parent) {
		parent = MainWindow::instance();
	}
	XNotifier notifier(parent);
	notifier.setText(text);
	notifier.setIcon(":/res/itrac-64.png");

	parent->setGraphicsEffect(new XDimmingEffect);
	//notifier.show();
	notifier.exec();
	parent->setGraphicsEffect(nullptr);
}

void XNotifier::error(QWidget *parent, const QString &text, int msecDisplayTime)
{

}

void XNotifier::info(QWidget *parent, const QString &text, int msecDisplayTime)
{

}

XNotifier::XNotifier(QWidget *parent)
	: QDialog(parent)
{
	// hide the title
	setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
	//setWindowOpacity(0.8);
	setStyleSheet("QDialog{background-color:white}"
	"QLabel{color:black;font-size:20px}");

	// layout the informations
	QHBoxLayout *topHLayout = new QHBoxLayout();
	topHLayout->addStretch();

	_icon = new QLabel;
	_icon->setAlignment(Qt::AlignCenter);
	topHLayout->addWidget(_icon);

	_text = new QLabel;
	topHLayout->addWidget(_text);

	QWidget *progressbar = new QWidget;
	topHLayout->addWidget(progressbar);

	topHLayout->addStretch();
	topHLayout->setSpacing(20);

	// layout the close button
	QHBoxLayout *bottomHLayout = new QHBoxLayout();
	bottomHLayout->addStretch();

	QPushButton *closeButton = new QPushButton("我知道了");
	closeButton->setMinimumSize(QSize(200, 38));
	bottomHLayout->addWidget(closeButton);
	connect(closeButton, &QAbstractButton::clicked, this, &QDialog::accept);

	bottomHLayout->addStretch();

	QVBoxLayout *verticalLayout = new QVBoxLayout(this);
	verticalLayout->addLayout(topHLayout);
	verticalLayout->addLayout(bottomHLayout);
	verticalLayout->setSpacing(0);

	this->setLayout(verticalLayout);

	// set geometry
	// TODO: what if parent widget is very small
	//int height = 200;
	//if (parent)
	//{
	//	QPoint parentPos = parent->mapToGlobal(parent->pos()); // works on systems with a second monitor
	//	setGeometry(parentPos.x(),
	//		parentPos.y() + parent->height() / 4,
	//		parent->width(), height);

	//	//resize(parent->width(), height);
	//	//move(0, (parent->height() - height) / 2);
	//}
}

XNotifier::~XNotifier()
{
}

void XNotifier::setText(const QString &text)
{
	_text->setText(text);
}

void XNotifier::setIcon(const QString &fileName)
{
	_icon->setPixmap(QPixmap(fileName));
}

void XNotifier::setDisplayTime(int)
{

}
