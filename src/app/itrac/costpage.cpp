#include "costpage.h"
#include "ui/views.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "dialog/addcostingdialog.h"
#include <QtWidgets/QtWidgets>

Costpage::Costpage(QWidget *parent)
	: QWidget(parent)
	, _dateWidget(new QTreeWidget)
	, _detailView(new TableView)
	, _detailModel(new QStandardItemModel(0, 4, _detailView))
{
	initCostView();

	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &Costpage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &Costpage::add);

	QHBoxLayout *hlayout_t = new QHBoxLayout;
	hlayout_t->addWidget(refreshButton);
	hlayout_t->addWidget(addButton);
	hlayout_t->addStretch();

	QHBoxLayout *hLayout_b = new QHBoxLayout;
	hLayout_b->addWidget(_dateWidget);
	hLayout_b->addWidget(_detailView);
	hLayout_b->setStretch(1, 1);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(hlayout_t);
	mainLayout->addLayout(hLayout_b);

	connect(_dateWidget, SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(onTreeItemDoubleClicked(QTreeWidgetItem *, int)));
	//connect(_dateWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(onTreeItemDoubleClicked(QTreeWidgetItem *, int)));
}

Costpage::~Costpage()
{
}

void Costpage::add()
{
	Addcostingdialog d(this);

	if (d.exec())
	{
		refresh();
	}
}

void Costpage::refresh()
{
	QByteArray data;
	data.append("{}");

	post(url(PATH_COST_GETMONTH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取核算数据: ").append(resp.errorString()));
			return;
		}

		const QString code = resp.getAsString("code");

		if (!code.compare("9000")) {
			_dateWidget->clear();
			QVariantList monthList = resp.getAsList("costing_month");
			QStringList yearList;
			for (auto &month : monthList) {
				QString szMonth = month.toString();
				QString szYear = szMonth.left(4);
				if (!yearList.contains(szYear)) yearList.append(szYear);
			}

			for each (QString var in yearList)
			{
				QTreeWidgetItem *item = new QTreeWidgetItem(_dateWidget, QStringList(var));
				for (auto &month : monthList) {
					QString szMonth = month.toString();
					QString szYear = szMonth.left(4);
					if (!szYear.compare(var))
					{
						QTreeWidgetItem *item1 = new QTreeWidgetItem(item, QStringList(szMonth));
						if (!szMonth.compare(_dateString))
						{
							item1->setSelected(true);
							_dateWidget->expandItem(item1->parent());
						}
							
						item->addChild(item1);
					}
				}
			}
			
		}
		else {
			XNotifier::warn(QString("无法获取核算数据: ").append(resp.getAsString("msg")));
			return;
		}
	});

	if (!_dateString.isEmpty())
		updateDetailView(_dateString);
}

void Costpage::onTreeItemDoubleClicked(QTreeWidgetItem *item, int column)
{
	if (item->childCount())  return;
	
	_dateString = item->text(column);
	updateDetailView(_dateString);
}

void Costpage::updateDetailView(const QString& date)
{
	QByteArray data;
	data.append("{\"month\":\"").append(date).append("\"}");

	post(url(PATH_COST_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取该月明细: ").append(resp.errorString()));
			return;
		}

		const QString code = resp.getAsString("code");
		
		if (!code.compare("9000")) {
			_detailModel->removeRows(0, _detailModel->rowCount());
			QVariantList detailList = resp.getAsList("costings");
			float sum = 0.0f;
			for (auto &detail : detailList) {
				QVariantMap map = detail.toMap();

				QList<QStandardItem *> rowItems;
				QStandardItem *item_name = new QStandardItem();
				item_name->setTextAlignment(Qt::AlignCenter);
				item_name->setText(map["name"].toString());
				rowItems.append(item_name);

				QStandardItem *item_price = new QStandardItem();
				item_price->setTextAlignment(Qt::AlignCenter);
				item_price->setText(map["price"].toString());
				rowItems.append(item_price);

				QStandardItem *item_num = new QStandardItem();
				item_num->setTextAlignment(Qt::AlignCenter);
				item_num->setText(map["num"].toString());
				rowItems.append(item_num);

				QStandardItem *item_total = new QStandardItem();
				item_total->setTextAlignment(Qt::AlignCenter);
				item_total->setText(QString::number(map["total"].toFloat(), 'f', 2));
				sum += map["total"].toFloat();
				rowItems.append(item_total);

				_detailModel->appendRow(rowItems);
			}

			QList<QStandardItem *> rowItems;
			QStandardItem *item_name = new QStandardItem();
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setText("总计");
			rowItems.append(item_name);

			rowItems.append(new QStandardItem());
			rowItems.append(new QStandardItem());

			QStandardItem *item_total = new QStandardItem();
			item_total->setTextAlignment(Qt::AlignCenter);
			item_total->setText(QString::number(sum, 'f', 2));
			rowItems.append(item_total);

			_detailModel->appendRow(rowItems);
		}
		else {
			XNotifier::warn(QString("无法获取该月明细: ").append(resp.getAsString("msg")));
			return;
		}
	
	});
}

void Costpage::initCostView()
{
	_dateWidget->setHeaderLabel("日期");	

	_dateWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	_dateWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

	_detailModel->setHeaderData(0, Qt::Horizontal, "项目名");
	_detailModel->setHeaderData(1, Qt::Horizontal, "单价");
	_detailModel->setHeaderData(2, Qt::Horizontal, "数量");
	_detailModel->setHeaderData(3, Qt::Horizontal, "总价");
	_detailView->setModel(_detailModel);

	_detailView->setSelectionMode(QAbstractItemView::SingleSelection);
	_detailView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_detailView->setContextMenuPolicy(Qt::CustomContextMenu);

	QHeaderView *header = _detailView->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 6);

	refresh();
}