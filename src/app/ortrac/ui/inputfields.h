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
		FlatEdit(QWidget *parent = nullptr, const QString &placeHolder = QString());
		~FlatEdit();

		void setInputValidator(InputValitor validator);

	protected:
		QSize minimumSizeHint() const override;
	};

} // namespace Ui
