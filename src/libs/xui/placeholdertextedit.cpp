#include <QPaintEvent>
#include <QPainter>
#include <QUrl>
#include "placeholdertextedit.h"

PlaceholderTextEdit::PlaceholderTextEdit(QWidget *parent)
	: QTextEdit(parent), _maxLength(0)
{
	setAttribute(Qt::WA_InputMethodEnabled, true);
	//setAttribute(Qt::WA_KeyCompression, true);
}

QString PlaceholderTextEdit::placeholderText() const
{
	return _placeholderText;
}

void PlaceholderTextEdit::setPlaceholderText(const QString &text)
{
	if (_placeholderText != text) {
		_placeholderText = text;
		if (!hasFocus())
			update();
	}
}

void PlaceholderTextEdit::paintEvent(QPaintEvent *e)
{
	QTextEdit::paintEvent(e);

	QPainter painter(this);
// 	painter.setPen(Qt::black);
// 	painter.drawText(1, 1, "text");

	//if (toPlainText().isEmpty()) {
	//	if (!hasFocus() && !_placeholderText.isEmpty()) {
	//		QPainter p(this);
	//		QRect rc = rect();
	//		p.fillRect(e->rect(), Qt::lightGray);
	//		QFontMetrics fm = fontMetrics();
	//		QString elidedText = fm.elidedText(_placeholderText, Qt::ElideRight, rc.width());
	//		p.setPen(Qt::lightGray);
	//		p.drawText(e->rect(), Qt::AlignLeft|Qt::AlignTop, elidedText);
	//		return;
	//	}
	//}
}

int PlaceholderTextEdit::maxLength()
{
	return _maxLength;
}

void PlaceholderTextEdit::setMaxLength(int length)
{
	_maxLength = length;
	QString curText = toPlainText();
	if ((_maxLength > 0) && (curText.length() > _maxLength))
		setPlainText(curText.left(_maxLength));
}

void PlaceholderTextEdit::keyPressEvent(QKeyEvent *e)
{
	int reservedTextLength = toPlainText().length() - selectedTextLength();
	if ((_maxLength <= 0) || (reservedTextLength < _maxLength))	{
		QTextEdit::keyPressEvent(e);
	} else {
		QString txt = e->text();
		int key = e->key();
		bool strongKey = (key==Qt::Key_Delete)
			|| (key==Qt::Key_Backspace)
			|| (key==Qt::Key_Cancel); // these keys are always available 

// 		bool asciiCtrl = false;
// 		if (!strongKey && key!=Qt::Key_Tab && key!=Qt::Key_Enter) {
// 			if (txt.length() == 1) {
// 				int sym = txt.toAscii().at(0);
// 				if (sym >= 0 && sym <= 31)
// 					asciiCtrl = true;
// 			}
// 		}

		if (txt.isEmpty() || strongKey/* || asciiCtrl*/)	{
			QTextEdit::keyPressEvent(e);
		}
	}
}

void PlaceholderTextEdit::inputMethodEvent(QInputMethodEvent *e)
{
	QString commitStr = e->commitString();
	if (!commitStr.isEmpty()) { // the selected text is removed before string committed
		if (_maxLength > 0)	{
			QString curText = toPlainText();
			int totalLength = curText.length() + commitStr.length();
			if (totalLength > _maxLength)
			{
				int numToDelete = totalLength - _maxLength;
				commitStr = commitStr.left(commitStr.length() - numToDelete);
				e->setCommitString(commitStr);
			}
		}
	}
	QTextEdit::inputMethodEvent(e);
}

void PlaceholderTextEdit::insertFromMimeData(const QMimeData* source)
{
	QMimeData scopy;
	if (source->hasText())
		scopy.setText(source->text());
// 	if (source->hasColor())	scopy.setColorData(source->colorData());
// 	if (source->hasHtml()) scopy.setHtml(source->html());
// 	if (source->hasImage()) scopy.setImageData(source->imageData());
// 	if (source->hasUrls()) scopy.setUrls(source->urls());
	
	if (_maxLength > 0 && source->hasText()) {
		QString textToPaste = source->text();
		QString curText = toPlainText();
		int totalLength = curText.length() + textToPaste.length() - selectedTextLength();
		if (totalLength > _maxLength) {
			int numToDelete = totalLength - _maxLength;
			textToPaste = textToPaste.left(textToPaste.length() - numToDelete);
			scopy.setText(textToPaste);
		}
	}

	QTextEdit::insertFromMimeData(&scopy);
}

int PlaceholderTextEdit::selectedTextLength()
{
	QString text = textCursor().selectedText();
	return text.length();
}
