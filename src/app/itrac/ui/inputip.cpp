#include "inputip.h"  
#include <QRegExpValidator>  
#include <QPainter>  
#include <QHBoxLayout>  
#include <QKeyEvent>  
#include <QMessageBox>  
#include <QDebug>

TLineEditIP::TLineEditIP(QWidget *parent)
	:QLineEdit(parent)
{
	QRegExp rx("(2[0-5]{2}|2[0-4][0-9]|1?[0-9]{1,2})");
	QHBoxLayout *pHBox = new QHBoxLayout(this);
	pHBox->setSpacing(4);
	pHBox->setContentsMargins(4, 4, 2, 4);
	for (int i = 0; i < 4; i++)
	{
		m_lineEidt[i] = new QLineEdit(this);
		m_lineEidt[i]->setFrame(false);
		m_lineEidt[i]->setMaxLength(3);
		m_lineEidt[i]->setTextMargins(0, 0, 0, 0);
		m_lineEidt[i]->setAlignment(Qt::AlignCenter);
		m_lineEidt[i]->installEventFilter(this);
		m_lineEidt[i]->setValidator(new QRegExpValidator(rx, this));
		m_lineEidt[i]->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
		m_lineEidt[i]->setStyleSheet("QLineEdit{font: 14px;border:none;}");
		m_lineEidt[i]->setAttribute(Qt::WA_MacShowFocusRect, 0);
		m_lineEidt[i]->setContextMenuPolicy(Qt::NoContextMenu);
		pHBox->addWidget(m_lineEidt[i]);
	}
	this->setReadOnly(true);
	this->setContextMenuPolicy(Qt::NoContextMenu);
	installEventFilter(this);
	connect(this, SIGNAL(signalTextChanged(QLineEdit*)),
		this, SLOT(slotTextChanged(QLineEdit*)),
		Qt::QueuedConnection);
}

TLineEditIP::~TLineEditIP()
{

}

void TLineEditIP::paintEvent(QPaintEvent *event)
{
	QLineEdit::paintEvent(event);
	QPainter painter(this);
	QBrush brush;
	brush.setStyle(Qt::BrushStyle::SolidPattern);
	brush.setColor(Qt::black);
	painter.setBrush(brush);

	int width = 0;
	for (int i = 0; i < 3; i++)
	{
		width += m_lineEidt[i]->width() + (i == 0 ? 4 : 5);//布局的间隔  
		painter.drawEllipse(width, height() / 2 - 1, 1, 1);
	}
}

QString TLineEditIP::getIPItemStr(unsigned char item)
{
	return QString("%1\n").arg(item);
}

void TLineEditIP::slotTextChanged(QLineEdit* pEdit)
{
	for (unsigned int i = 0; i != QTUTL_IP_SIZE; ++i)
	{
		if (pEdit == m_lineEidt[i])
		{
			if (pEdit->text().size() == QTUTL_IP_SIZE - 1 &&
				pEdit->text().size() == pEdit->cursorPosition())
			{
				if (i + 1 != QTUTL_IP_SIZE)
				{
					m_lineEidt[i + 1]->setFocus();
					m_lineEidt[i + 1]->selectAll();
				}
			}
		}
	}
}

int TLineEditIP::getIndex(QLineEdit *pEdit)
{
	int index = -1;
	for (int i = 0; i < 4; i++)
	{
		if (pEdit == m_lineEidt[i])
			index = i;
	}
	return index;
}

bool TLineEditIP::eventFilter(QObject *obj, QEvent *event)
{
	bool bRes = QLineEdit::eventFilter(obj, event);
	if (event->type() == QEvent::Show)
	{
		if (obj == this)
		{
			m_lineEidt[0]->setFocus();
			m_lineEidt[0]->setCursorPosition(m_lineEidt[0]->text().size());
			m_lineEidt[0]->selectAll();
		}
	}
	else if (event->type() == QEvent::KeyPress)
	{
		QKeyEvent* pEvent = dynamic_cast<QKeyEvent*>(event);
		if (pEvent)
		{
			for (unsigned int i = 0; i != QTUTL_IP_SIZE; ++i)
			{
				QLineEdit* pEdit = m_lineEidt[i];
				if (pEdit == obj)
				{
					switch (pEvent->key())
					{
					case Qt::Key_Left:
						if (pEdit->cursorPosition() == 0)
						{
							// user wants to move to previous item 
							m_lineEidt[i > 0 ? i - 1 : QTUTL_IP_SIZE - 1]->setFocus();
							m_lineEidt[i > 0 ? i - 1 : QTUTL_IP_SIZE - 1]->setCursorPosition(m_lineEidt[i > 0 ? i - 1 : QTUTL_IP_SIZE - 1]->text().size());
							m_lineEidt[i > 0 ? i - 1 : QTUTL_IP_SIZE - 1]->selectAll();
						}
						break;
					case Qt::Key_Right :
						if (pEdit->text().isEmpty() || (pEdit->text().size() == pEdit->cursorPosition()))
						{
							// user wants to move to next item                        
							m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setFocus();
							m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setCursorPosition(0);
						}
						break;
					case Qt::Key_Period:
						if (pEdit->text().isEmpty())
							pEdit->setText("0");
						
						// user wants to move to next item                        
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setFocus();
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setCursorPosition(0);
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->selectAll();
						break;
					case Qt::Key_Return:
					case Qt::Key_Tab:
					{
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setFocus();
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->setCursorPosition(0);
						m_lineEidt[i + 1 != QTUTL_IP_SIZE ? i + 1 : 0]->selectAll();
						return true;
					}
					case Qt::Key_Backspace:
					{
						QString strText = pEdit->text();
						if (!strText.length() || strText.length() && !pEdit->cursorPosition())
						{
							int index = getIndex(pEdit);
							if (index != -1 && index != 0)
							{
								m_lineEidt[index - 1]->setFocus();
								int length = m_lineEidt[index - 1]->text().length();
								m_lineEidt[index - 1]->setCursorPosition(length ? length : 0);
							}
						}
						return QLineEdit::eventFilter(obj, event);
					}
					default:
						emit signalTextChanged(pEdit);
					}
					break;
				}
			}
		}
	}
	return bRes;
}
void TLineEditIP::setText(const QString &strIP)
{
	if (!isTextValid(strIP))
	{
		QMessageBox::warning(this, "Attention", "Your IP Address is Invalid!", QMessageBox::StandardButton::Ok);
		return;
	}
	else
	{
		int i = 0;
		QStringList ipList = strIP.split(".");
		foreach(const QString &ip, ipList)
		{
			m_lineEidt[i]->setText(ip);
			i++;
		}
	}
	m_lineEidt[0]->selectAll();
}

bool TLineEditIP::isTextValid(const QString &strIP)
{
	QRegExp rx2("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
	if (!rx2.exactMatch(strIP))
		return false;
	return true;
}

QString TLineEditIP::text() const
{
	return QString("%1.%2.%3.%4")
		.arg(m_lineEidt[0]->text().isEmpty() ? "0" : m_lineEidt[0]->text())
		.arg(m_lineEidt[1]->text().isEmpty() ? "0" : m_lineEidt[1]->text())
		.arg(m_lineEidt[2]->text().isEmpty() ? "0" : m_lineEidt[2]->text())
		.arg(m_lineEidt[3]->text().isEmpty() ? "0" : m_lineEidt[3]->text());
}