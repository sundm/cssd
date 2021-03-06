#include "analysepage.h"
#include "core/itracnamespace.h"
#include "analyse/kpipage.h"
#include "analyse/deptstatisticpage.h"
#include "analyse/themewidget.h"
#include "analyse/costpage.h"
#include "analyse/washpage.h"
#include "analyse/sterilepage.h"
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
	//QPushButton *deviceButton = new QPushButton("设备运转分析");
	//vLayout->addWidget(deviceButton);
	QPushButton *costButton = new QPushButton("成本核算");
	vLayout->addWidget(costButton);
	QPushButton *washButton = new QPushButton("清洗异常统计");
	vLayout->addWidget(washButton);
	QPushButton *sterileButton = new QPushButton("灭菌异常统计");
	vLayout->addWidget(sterileButton);
	QPushButton *realtimeButton = new QPushButton("更多统计");
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
	btnGroup->addButton(costButton, Cost);
	btnGroup->addButton(washButton, Wash);
	btnGroup->addButton(sterileButton, Sterile);
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
		case KPI:
			page = new KpiPage(this);
			_tabWidget->addTab(page, "人员绩效");
			break;
		case Dept:
			page = new DeptStatisticsPage(this);
			_tabWidget->addTab(page, "科室用包统计");
			break;
		case Cost:
			page = new Costpage(this);
			_tabWidget->addTab(page, "成本核算");
			break;
		case Wash:
			page = new WashPage(this);
			_tabWidget->addTab(page, "清洗异常统计");
			break;
		case Sterile:
			page = new SterilePage(this);
			_tabWidget->addTab(page, "灭菌异常统计");
			break;
		case RealTime:
			page = new ThemeWidget(this);
			_tabWidget->addTab(page, "更多统计");
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
