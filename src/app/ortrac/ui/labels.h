#pragma once

#include <QLabel>

class QPropertyAnimation;

namespace Ui {

	class Title : public QLabel
	{
		Q_OBJECT

	public:
		Title(const QString text, QWidget *parent = nullptr);
		~Title() = default;
	};

	class Description : public QLabel
	{
		Q_OBJECT

	public:
		Description(const QString text, QWidget *parent = nullptr);
		~Description() = default;
	};

	class ErrorLabel : public QLabel {
		Q_OBJECT
	public:
		ErrorLabel(const QString text, QWidget *parent = nullptr);
		~ErrorLabel() = default;
		
		void shake(const QString &error);
	private:
		std::unique_ptr<QPropertyAnimation> _anim;
	};

	class CheckableImageLabel : public QLabel {
		Q_OBJECT
	public:
		CheckableImageLabel(const QPixmap &pixmap = QPixmap(), QWidget *parent = nullptr);
		~CheckableImageLabel() = default;
		void setChecked(bool);
		bool isChecked() const { return _checked; };
	protected:
		void paintEvent(QPaintEvent *);
	private:
		bool _checked;
		QPixmap _pixmap;
	};

} // namespace Ui 
