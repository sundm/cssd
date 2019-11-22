#include "topbar.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/composite/sliderbar.h"
#include "dialog/userinfodialog.h"
#include "dialog/warningdialog.h"
#include "../libs/rfidreader/rfidreader.h"
#include "dialog/rfidreaderconfigerdialog.h"
#include "dialog/rfidconfigerdialog.h"
#include <QtWidgets/QtWidgets>

TopBar::TopBar(QWidget *parent) :
    QFrame(parent), _naviBar(new SliderBar)
{
	setObjectName("TopBar");
	setMinimumHeight(60);

	Core::User &user = Core::currentUser();
	Ui::IconButton *userButton = new Ui::IconButton(":/res/user-64.png");
	userButton->setMinimumSize(QSize(118, 0));
	userButton->setIconSize(QSize(32, 32));
	userButton->setText(user.name);
	userButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	connect(userButton, &QToolButton::clicked, this, &TopBar::popupUserInfo);

	Ui::IconButton *nofityButton= new Ui::IconButton(":/res/bell-64.png");
	nofityButton->setIconSize(QSize(24, 24));
	connect(nofityButton, SIGNAL(clicked()), this, SLOT(showWarnings()));

	Ui::IconButton *settingsButton = new Ui::IconButton(":/res/cog-64.png");
	settingsButton->setIconSize(QSize(24, 24));
	connect(settingsButton, SIGNAL(clicked()), this, SLOT(showRfidConfiger()));

	Ui::IconButton *aboutButton = new Ui::IconButton(":/res/info-64.png");
	aboutButton->setIconSize(QSize(24, 24));

	// setup navigator
	QString items;
	switch (user.role) {
	case Core::User::Admin:
	case Core::User::HeadNurse:
		//items = "使用登记;物品申领;流程管理;历史查询;物资管理;报表统计";
		items = "流程管理;历史查询;物资管理;报表统计";
		break;
	case Core::User::OrUser:
		items = "使用登记;历史查询";
		break;
	case Core::User::ClinicUser:
		items = "物品申领";
		break;
	case Core::User::CssdUser:
		items = "流程管理;历史查询";
		break;
	default:
		break;
	}
	_naviBar->setItems(items);
	_naviBar->setHorizontal(true);
	_naviBar->setLineWidth(3);
	_naviBar->setSpace(100);
	_naviBar->setBarStyle(SliderBar::BarStyle_Line_Bottom);
	connect(_naviBar, SIGNAL(currentItemChanged(int, const QString&)),
		this, SIGNAL(currentItemChanged(int, const QString&)));

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(-1, 0, -1, 0);
	layout->addWidget(userButton);
	layout->addStretch();
	layout->addWidget(_naviBar);
	layout->addStretch();
	layout->addWidget(nofityButton);
	layout->addWidget(settingsButton);
	layout->addWidget(aboutButton);
}

void TopBar::showWarnings() {
	WarningDialog d(this);
	d.exec();
}

void TopBar::showRfidConfiger() {
	ConfigRfidDialog d(this);
	d.exec();
}

void TopBar::popupUserInfo() {
	UserInfoDialog d(this);
	d.exec();
}
