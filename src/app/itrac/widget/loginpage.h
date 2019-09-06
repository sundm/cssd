#pragma once

#include "ui/container.h"
#include "ui/loader.h"
#include <QWidget>
#include <qscanner/qscanner.h>
#include "core/net/jsonhttpclient.h"

namespace Ui {
	class Picture;
	class Container;
	class FlatEdit;
	class ErrorLabel;
} // namespace Ui 

class WaitingSpinner;

namespace Widget {
	class LoginPanel : public Ui::Inner, public Scanable, public JsonHttpClient
	{
		Q_OBJECT
	public:
		LoginPanel(Ui::Container *container = nullptr);
		void submit() override;

	protected:
		void handleBarcode(const QString &) override;

	private:
		void login(const QString &, const QString &);
		void version();

		const QString getFileMd5(QString filePath);

		Ui::FlatEdit *userEdit;
		Ui::FlatEdit *pwdEdit;
		Ui::ErrorLabel *error;
		WaitingSpinner *_waiter;
	};

	class LoginPage : public Ui::Source
	{
		Q_OBJECT

	public:
		LoginPage(QWidget *parent = nullptr);

	signals:
		void permitted();

	private:
		Ui::Picture * _headPic;
		Ui::Container *_container;
	};

} // namespace Widget 
