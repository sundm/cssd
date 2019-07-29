#include "assetpage.h"
#include "devicepage.h"
#include "deviceprogrampage.h"
#include "userpage.h"
#include "assets/packagepage.h"
#include "assets/instrumentpage.h"
#include "assets/departmentpage.h"
#include "assets/recallpage.h"
#include "core/itracnamespace.h"

#include <QtWidgets/QtWidgets>

AssetPage::AssetPage(QWidget *parent)
	: QWidget(parent)
	, _tabWidget(new QTabWidget)
{
	_tabWidget->setTabsClosable(true);
	_tabWidget->setMovable(true);
	connect(_tabWidget, &QTabWidget::tabCloseRequested, this, &AssetPage::closePage);

	QWidget *leftContainer = new QWidget;
	leftContainer->setMinimumWidth(200);
	leftContainer->setMaximumWidth(200);
	QVBoxLayout *vLayout = new QVBoxLayout(leftContainer);
	vLayout->setContentsMargins(-1, 36, -1, -1);
	QPushButton *deviceButton = new QPushButton("设备");
	vLayout->addWidget(deviceButton);
	QPushButton *programButton = new QPushButton("设备预设程序");
	vLayout->addWidget(programButton);
	QPushButton *pkgButton = new QPushButton("包");
	vLayout->addWidget(pkgButton);
	QPushButton *instrumentButton = new QPushButton("器械");
	vLayout->addWidget(instrumentButton);
	QPushButton *deptButton = new QPushButton("科室");
	vLayout->addWidget(deptButton);
	QPushButton *userButton = new QPushButton("用户");
	vLayout->addWidget(userButton);
	QPushButton *recallButton = new QPushButton("物品召回");
	vLayout->addWidget(recallButton);
	vLayout->addStretch(0);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(leftContainer);
	layout->addWidget(_tabWidget);

	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->addButton(deviceButton, itrac::DEVICE);
	btnGroup->addButton(programButton, itrac::PROGRAM);
	btnGroup->addButton(pkgButton, itrac::PACKAGE);
	btnGroup->addButton(instrumentButton, itrac::INSTRUMENT);
	btnGroup->addButton(deptButton, itrac::DEPT);
	btnGroup->addButton(userButton, itrac::USER);
	btnGroup->addButton(recallButton, itrac::RECALL);
	connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &AssetPage::showPage);
}

void AssetPage::showPage(int id)
{
	QWidget* page = _pageMap.value(id, nullptr);
	if (nullptr == page) {
		// create it
		switch (id)
		{
		case itrac::DEVICE:
			page = new DevicePage;
			_tabWidget->addTab(page, "设备管理");
			break;
		case itrac::PROGRAM:
			page = new DeviceProgramPage;
			_tabWidget->addTab(page, "设备预设程序管理");
			break;
		case itrac::PACKAGE:
			page = new PackagePage;
			_tabWidget->addTab(page, "包管理");
			break;
		case itrac::INSTRUMENT:
			page = new InstrumentPage;
			_tabWidget->addTab(page, "器械管理");
			break;
		case itrac::DEPT:
			page = new DepartmentPage;
			_tabWidget->addTab(page, "科室");
			break;
		case itrac::USER:
			page = new UserPage;
			_tabWidget->addTab(page, "用户管理");
			break;
		case itrac::RECALL:
			page = new RecallPage;
			_tabWidget->addTab(page, "物品召回");
			break;
		default: // this should never happen
			break;
		}
		_pageMap[id] = page;
	}

	_tabWidget->setCurrentWidget(page);
}

void AssetPage::closePage(int index)
{
	QWidget *page = _tabWidget->widget(index);
	_tabWidget->removeTab(index);

	int pageIdInMap = _pageMap.key(page, -1);
	if (-1 != pageIdInMap) {
		_pageMap[pageIdInMap] = nullptr;
	}
	delete page;
	page = nullptr;
}
