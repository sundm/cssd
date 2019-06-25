#pragma once

#include "ui/container.h"
#include "ui/loader.h"
#include <QWidget>
#include <qscanner/qscanner.h>

namespace Ui {
	class Picture;
	class Container;
	class FlatEdit;
	class ErrorLabel;
} // namespace Ui 

namespace Widget {
	class LoginPanel : public Ui::Inner, public Scanable
	{
		Q_OBJECT
	public:
		LoginPanel(Ui::Container *container = nullptr);
		void submit() override;

	protected:
		void handleBarcode(const QString &) override;

	private:
		void login(const QString &, const QString &);

		Ui::FlatEdit *userEdit;
		Ui::FlatEdit *pwdEdit;
		Ui::ErrorLabel *error;
	};

	class LoginPage : public Ui::Source
	{
		Q_OBJECT

	public:
		LoginPage(QWidget *parent = nullptr);

	signals:
		void permitted();

	protected:
		//void hideAnimated() override;

	private:
		Ui::Picture * _headPic;
		Ui::Container *_container;
	};

} // namespace Widget 
