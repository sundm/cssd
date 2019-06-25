#pragma once

#include <QLineEdit>

namespace Ui {
	enum class InputValitor{
		LetterOnly,
		NumberOnly,
		LetterAndNumber,
		Email,
		NoLimit
	};

	class FlatEdit : public QLineEdit
	{
		Q_OBJECT

	public:
		FlatEdit(const QString &placeHolder = QString(), QWidget *parent = nullptr);
		~FlatEdit();

		void setInputValidator(InputValitor validator);

	protected:
		QSize minimumSizeHint() const override;
	};

} // namespace Ui
