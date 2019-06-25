#pragma once

#include <QFrame>

namespace Ui {
	class Title;
}

namespace Composite {

	class Title : public QFrame
	{
		Q_OBJECT

	public:
		Title(const QString &title, bool closable = true, QWidget *parent = nullptr);
		~Title() = default;

		void setTitle(const QString &);

	signals:
		void canceled();

	private:
		Ui::Title *_title;
	};

} // namespace Composite
