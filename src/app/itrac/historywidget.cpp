#include <QButtonGroup>
#include "historywidget.h"
#include "historypage.h"
#include "core/itracnamespace.h"
#include "widget/history/tracepage.h"

HistoryWidget::HistoryWidget(QWidget *parent)
	: QWidget(parent)
{
	setupUi(this);

	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->addButton(recycleButton, itrac::Recycle);
	btnGroup->addButton(washButton, itrac::Wash);
	btnGroup->addButton(packButton, itrac::Pack);
	btnGroup->addButton(sterileButton, itrac::Sterile);
	btnGroup->addButton(dispatchButton, itrac::Dispatch);
	btnGroup->addButton(useButton, itrac::Use);
	btnGroup->addButton(traceButton, itrac::Trace);
	//traceButton->hide();
	connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &HistoryWidget::showPage);

	tabWidget->setTabsClosable(true);
	tabWidget->setMovable(true);
	connect(tabWidget, &QTabWidget::tabCloseRequested, this, &HistoryWidget::closePage);
}

HistoryWidget::~HistoryWidget()
{
}

void HistoryWidget::showPage(int id)
{
	QWidget* page = _pageMap.value(id, nullptr);
	if (nullptr == page) {
		// create it
		switch (id)
		{
		case itrac::Recycle:
			page = new RecycleHistoryPage;
			tabWidget->addTab(page, "回收查询");
			break;
		case itrac::Wash:
			page = new WashHistoryPage;
			tabWidget->addTab(page, "清洗查询");
			break;
		case itrac::Pack:
			page = new PackHistoryPage;
			tabWidget->addTab(page, "配包查询");
			break;
		case itrac::Sterile:
			page = new SterileHistoryPage;
			tabWidget->addTab(page, "灭菌查询");
			break;
		case itrac::Dispatch:
			/*page = new DevicePage;
			tabWidget->addTab(page, "设备");*/
			break;
		case itrac::Use:
			page = new IssueHistoryPage;
			tabWidget->addTab(page, "使用查询");
			break;
		case itrac::Trace:
			page = new TracePage;
			tabWidget->addTab(page, "信息追溯");
			break;
		default: // this should never happen
			break;
		}
		_pageMap[id] = page;
	}

	tabWidget->setCurrentWidget(page);
}

void HistoryWidget::closePage(int index)
{
	QWidget *page = tabWidget->widget(index);
	tabWidget->removeTab(index);

	int pageIdInMap = _pageMap.key(page, -1);
	if (-1 != pageIdInMap) {
		_pageMap[pageIdInMap] = nullptr;
	}
	delete page;
	page = nullptr;
}
