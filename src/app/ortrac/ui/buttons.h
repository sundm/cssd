#pragma once

#include <QPushButton>
#include <QToolButton>

namespace Ui {

	enum class BtnSize {
		Small,
		Normal,
		Large,
	};

	class PrimaryButton : public QPushButton
	{
		Q_OBJECT

	public:
		PrimaryButton(QWidget *parent = nullptr);
		PrimaryButton(const QString text, BtnSize size = BtnSize::Normal, QWidget *parent = nullptr);
		~PrimaryButton() = default;

	protected:
		QSize minimumSizeHint() const override;

	private:
		BtnSize _size;
	};

	class CommandButton : public QPushButton {
		Q_OBJECT
	public:
		explicit CommandButton(const QString &text, QWidget *parent = 0);
		CommandButton(const QString &text, const QString &desc, QWidget *parent = 0);
		CommandButton(const QIcon &icon, const QString &text, const QString &desc, QWidget *parent = 0);
		QString description() const;
		void setDescription(const QString &description);

	protected:
		QSize sizeHint() const override;
		QSize minimumSizeHint() const override;
		bool event(QEvent *e) override;
		void paintEvent(QPaintEvent *) override;

	private:
		void init();
		QFont textFont() const;
		QFont descriptionFont() const;
		QRect textRect() const;
		QRect descriptionRect() const;
		int descriptionHeight(int widgetWidth) const;
		int textOffset() const;
		int descriptionOffset() const;

		QString _desc;

		static constexpr int TopMargin = 10;
		static constexpr int BottomMargin = 10;
		static constexpr int LeftMargin = 8;
		static constexpr int RightMargin = 8;
	};

	class IconButton : public QToolButton {
		Q_OBJECT
	public:
		IconButton(const QString &fileName, const QString &toolTip = QString(), QWidget *parent = Q_NULLPTR);
	};
} // namespace Ui
