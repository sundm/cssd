#include "orpage.h"
#include "overlays/use.h"

#include "ui/loader.h"
#include <QtWidgets/QtWidgets>

OrPage::OrPage(QWidget *parent)
	: QWidget(parent)
{
	QWidget *leftContainer = new QWidget;
	leftContainer->setMinimumWidth(200);
	leftContainer->setMaximumWidth(200);
	QVBoxLayout *vLayout = new QVBoxLayout(leftContainer);
	vLayout->setContentsMargins(-1, 36, -1, -1);
	QPushButton *useButton = new QPushButton("使用登记");
	vLayout->addWidget(useButton);
	QPushButton *queryButton = new QPushButton("查询");
	vLayout->addWidget(queryButton);
	QPushButton *tracButton = new QPushButton("追溯");
	vLayout->addWidget(tracButton);
	vLayout->addStretch(0);

	QHBoxLayout *layout = new QHBoxLayout(this);
	Ui::Loader *loader = new Ui::Loader(this);
	loader->setSource(new UsePanel);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(leftContainer);
	layout->addWidget(loader);

	QButtonGroup *btnGroup = new QButtonGroup(this);
	btnGroup->addButton(useButton, OrPage::Use);
	btnGroup->addButton(queryButton, OrPage::Query);
	btnGroup->addButton(tracButton, OrPage::Track);
	connect(btnGroup, QOverload<int>::of(&QButtonGroup::buttonClicked), this, &OrPage::showPage);
}

OrPage::~OrPage()
{
}

void OrPage::showPage(int id)
{

}
