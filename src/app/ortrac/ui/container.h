#pragma once

#include <QWidget>

namespace Ui {

	class Container;
	class Inner : public QWidget
	{
		Q_OBJECT

	public:
		Inner(QWidget *parent = nullptr);
		Container *container() const;
		void updateSize();

	public slots:
		virtual void submit();

	signals:
		void submitted();

	private:
		Container * _container;
	};

	class Container : public QWidget
	{
		Q_OBJECT
	public:
		enum {Top, Up, Center, Down, Bottom};

		Container(QWidget *inner, int position = Container::Up, QWidget *parent = nullptr);
		~Container() = default;

		QWidget *inner() const { return _inner; }
		void setInner(QWidget *inner, int position = Container::Up);
		virtual void accept();

	signals:
		void accepted();
	
	protected:
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
		void resizeEvent(QResizeEvent *event) override;
		void updateInnerGeometry(QSize);

	private:
		QWidget * _inner;
		int _position;
	};

	class FadeInner : public QWidget {
		Q_OBJECT
	public:
		FadeInner(const QString &title, QWidget *parent = nullptr);
	protected:
		virtual void setupContent(QWidget *);
		void paintEvent(QPaintEvent *event) override;
	private:
		QWidget * _in;
	};

	class FadeContainer : public Container
	{
		Q_OBJECT
	public:
		FadeContainer(QWidget *inner, int position = Container::Up, QWidget *parent = nullptr);
	protected:
		void paintEvent(QPaintEvent *event) override;
		bool eventFilter(QObject *obj, QEvent *event) override;
	};

} // namespace Ui
