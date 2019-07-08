#include "analysepage.h"
#include "core/itracnamespace.h"
#include "analyse/themewidget.h"

#include <QtWidgets/QtWidgets>

AnalysePage::AnalysePage(QWidget *parent)
	: QWidget(parent)
	, _tabWidget(new QTabWidget)
{
	_tabWidget->setTabsClosable(true);
	_tabWidget->setMovable(true);
	connect(_tabWidget, &QTabWidget::tabCloseRequested, this, &AnalysePage::closePage);

	QWidget *leftContainer = new QWidget;
	leftContainer->setMinimumWidth(200);
	leftContainer->setMaximumWidth(200);
	QVBoxLayout *vLayout = new QVBoxLayout(leftContainer);
	vLayout->setContentsMargins(-1, 36, -1, -1);
	QPushButton *kpiButton = new QPushButton("人员绩效");
	vLayout->addWidget(kpiButton);
	QPushButton *deptButton = new QPushButton("科室用包统计");
	vLayout->addWidget(deptButton);
	QPushButton *deviceButton = new QPushButton("设备运转分析");
	vLayout->addWidget(deviceButton);
	QPushButton *realtimeButton = new QPushButton("实时工作量");
	vLayout->addWidget(realtimeButton);
	vLayout->addStretch(0);

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(leftContainer);
	layout->addWidget(_tabWidget);

	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->addButton(kpiButton, KPI);
	btnGroup->addButton(deptButton, Dept);
	btnGroup->addButton(deviceButton, Device);
	btnGroup->addButton(realtimeButton, RealTime);
	connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &AnalysePage::showPage);
}

void AnalysePage::showPage(int id)
{
	QWidget* page = _pageMap.value(id, nullptr);
	if (nullptr == page) {
		// create it
		switch (id)
		{
		//case itrac::DEVICE:
		//	page = new DevicePage;
		//	_tabWidget->addTab(page, "设备管理");
		//	break;
		//case itrac::PROGRAM:
		//	page = new DeviceProgramPage;
		//	_tabWidget->addTab(page, "设备预设程序管理");
		//	break;
		//case itrac::PACKAGE:
		//	page = new PackagePage;
		//	_tabWidget->addTab(page, "包管理");
		//	break;
		//case itrac::INSTRUMENT:
		//	page = new InstrumentPage;
		//	_tabWidget->addTab(page, "器械管理");
		//	break;
		//case itrac::DEPT:
		//	page = new DepartmentPage;
		//	_tabWidget->addTab(page, "科室");
		//	break;
		//case itrac::USER:
		//	page = new UserPage;
		//	_tabWidget->addTab(page, "用户管理");
		//	break;
		//case itrac::COST:
		//	page = new Costpage;
		//	_tabWidget->addTab(page, "成本核算");
		//	break;
		case RealTime:
			page = new ThemeWidget(this);
			_tabWidget->addTab(page, "实时工作量");
			break;
		default: // this should never happen
			break;
		}
		_pageMap[id] = page;
	}

	_tabWidget->setCurrentWidget(page);
}

void AnalysePage::closePage(int index)
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
