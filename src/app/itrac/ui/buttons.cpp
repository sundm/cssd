#include "buttons.h"

#include "qmath.h"
#include <QVariant>
#include <QStyleOptionButton>
#include <QStylePainter>
#include <QFont>
#include <QTextLayout>

namespace {

// Mix colors a and b with a ratio in the range [0-255]
QColor mergedColors(const QColor &a, const QColor &b, int value = 50)
{
	Q_ASSERT(value >= 0);
	Q_ASSERT(value <= 255);
	QColor tmp = a;
	tmp.setRed((tmp.red() * value) / 255 + (b.red() * (255 - value)) / 255);
	tmp.setGreen((tmp.green() * value) / 255 + (b.green() * (255 - value)) / 255);
	tmp.setBlue((tmp.blue() * value) / 255 + (b.blue() * (255 - value)) / 255);
	return tmp;
}

} // namespace


namespace Ui {

PrimaryButton::PrimaryButton(QWidget *parent) :QPushButton(parent) {
	_size = BtnSize::Small;
	setCursor(Qt::PointingHandCursor);
	setProperty("primary", true);
}

PrimaryButton::PrimaryButton(const QString text, BtnSize size, QWidget *parent)
	: QPushButton(text, parent)
	, _size(size) {
	setCursor(Qt::PointingHandCursor);
	setProperty("primary", true);
}

QSize PrimaryButton::minimumSizeHint() const {
	switch(_size) {
	case BtnSize::Small:  return QSize(160, 33);
	case BtnSize::Normal: return QSize(180, 42);
	case BtnSize::Large:  return QSize(300, 56);
	default: return QSize();
	}
}

/*!
CommandButton Implementation
*/
CommandButton::CommandButton(const QString &text, QWidget *parent)
	: QPushButton(text, parent) {
	init();
}

CommandButton::CommandButton(const QString &text, const QString &desc, QWidget *parent)
	: QPushButton(text, parent) {
	setDescription(desc);
	init();
}

CommandButton::CommandButton(const QIcon &icon, const QString &text,
	const QString &desc, QWidget *parent /*= 0*/)
	: QPushButton(text, parent) {
	setDescription(desc);
	setIcon(icon);
	init();
}

void CommandButton::init() {
	setAttribute(Qt::WA_Hover);
	setCursor(Qt::PointingHandCursor);
	// iconSize holds the maximum size for the icons, Smaller icons will not be scaled up
	setIconSize(QSize(128, 128));
}

QSize CommandButton::sizeHint() const {
	//  Standard size hints from UI specs
	//  Without note: 135, 41
	//  With note: 135, 60
	QSize size = QPushButton::sizeHint();
	QFontMetrics fmt(textFont()), fmd(descriptionFont());
	int textWidth = qMax(qMax(fmt.width(text()), fmd.width(_desc)), 135);
	int buttonWidth = textWidth + textOffset() + RightMargin;
	int heightWithoutDescription = descriptionOffset() + BottomMargin;

	//size.setWidth(qMax(size.width(), buttonWidth));
	size.setWidth(400);
	size.setHeight(qMax(_desc.isEmpty() ? 41 : 60,
		heightWithoutDescription + descriptionHeight(buttonWidth)));
	return size;
}

QSize CommandButton::minimumSizeHint() const {
	QSize size = sizeHint();
	int minimumHeight = qMax(descriptionOffset() + BottomMargin,
		icon().actualSize(iconSize()).height() + TopMargin + BottomMargin);
	size.setHeight(minimumHeight);
	return size;
}

void CommandButton::paintEvent(QPaintEvent *)
{
	QStylePainter p(this);
	p.save();

	QStyleOptionButton option;
	initStyleOption(&option);

	option.text = QString(); // draw this ourselves
	option.icon = QIcon(); // as above
	QSize pixmapSize = icon().actualSize(iconSize());

	int vOffset = isDown() ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical) : 0;
	int hOffset = isDown() ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal) : 0;

	//Draw icon
	p.drawControl(QStyle::CE_PushButton, option);
	if (!icon().isNull())
		p.drawPixmap(LeftMargin + hOffset, TopMargin + vOffset,
			icon().pixmap(pixmapSize, isEnabled() ? QIcon::Normal : QIcon::Disabled,
				isChecked() ? QIcon::On : QIcon::Off));

	//Draw title
	p.drawItemText(textRect().translated(hOffset, vOffset),
		0, option.palette, isEnabled(), text(), QPalette::ButtonText);

	//Draw description
	//int textflags = Qt::TextWordWrap | Qt::ElideRight;
	p.setFont(descriptionFont()); // do not respect qss
	option.palette.setColor(QPalette::ButtonText, QColor(153, 153, 153));
	p.drawItemText(descriptionRect().translated(hOffset, vOffset), 0,
		option.palette, isEnabled(), _desc, QPalette::ButtonText);
	p.restore();
}

QFont CommandButton::textFont() const {
	//QFont f = font();
	//f.setBold(true);
	//f.setPointSizeF(12.0);
	return font(); // respcet qss
}

QFont CommandButton::descriptionFont() const
{
	QFont f = font(); // respect qss
	f.setBold(false);
	f.setPointSizeF(9.0);
	return f;
}

// Calculates the height of the description text based on widget width
int CommandButton::descriptionHeight(int widgetWidth) const
{
	// Calc width of actual paragraph
	int lineWidth = widgetWidth - textOffset() - RightMargin;

	qreal descriptionheight = 0;
	if (!_desc.isEmpty()) {
		QTextLayout layout(_desc);
		layout.setFont(descriptionFont());
		layout.beginLayout();
		while (true) {
			QTextLine line = layout.createLine();
			if (!line.isValid())
				break;
			line.setLineWidth(lineWidth);
			line.setPosition(QPointF(0, descriptionheight));
			descriptionheight += line.height();
		}
		layout.endLayout();
	}
	return qCeil(descriptionheight);
}

QRect CommandButton::textRect() const
{
	QRect r = rect().adjusted(textOffset(), TopMargin, -RightMargin, 0);
	if (_desc.isEmpty())
	{
		QFontMetrics fm(textFont());
		r.setTop(r.top() + qMax(0, (icon().actualSize(iconSize()).height()
			- fm.height()) / 2));
	}

	return r;
}

QRect CommandButton::descriptionRect() const
{
	return rect().adjusted(textOffset(), descriptionOffset(),
		-RightMargin, -BottomMargin);
}

int CommandButton::textOffset() const
{
	return icon().actualSize(iconSize()).width() + LeftMargin + 16;
}

int CommandButton::descriptionOffset() const
{
	QFontMetrics fm(textFont());
	int textBottom = TopMargin + fm.height();
	int buttonHeight = icon().actualSize(iconSize()).height() + TopMargin + BottomMargin;
	return (textBottom * 2 > buttonHeight) ? textBottom : buttonHeight / 2;
}

void CommandButton::setDescription(const QString &description)
{
	_desc = description;
	updateGeometry();
	update();
}

QString CommandButton::description() const
{
	return _desc;
}

bool CommandButton::event(QEvent *e) {
	return QPushButton::event(e);
}

IconButton::IconButton(const QString &fileName, const QString &toolTip, QWidget *parent)
	:QToolButton(parent)
{
	setIcon(QIcon(fileName));
	setToolButtonStyle(Qt::ToolButtonIconOnly);
	if (toolTip.length()) {
		setToolTip(toolTip);
	}
}

} // namespace Ui
