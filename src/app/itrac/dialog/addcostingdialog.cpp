#include "addcostingdialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/inliner.h"
#include "core/net/url.h"
#include "core/assets.h"
#include "ui/buttons.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/views.h"
#include <QtWidgets/QtWidgets>

Addcostingdialog::Addcostingdialog(QWidget *parent)
	: QDialog(parent)
	, _dateEdit(new QDateEdit)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 4, _view))
{
	setWindowTitle("新增成本核算");

	_dateEdit->setDisplayFormat("yyyyMM");
	connect(_dateEdit, &QDateEdit::dateChanged, this, &Addcostingdialog::dateChanged);
	_dateEdit->setDate(QDate::currentDate().addMonths(-1));

	initCostView();

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &Addcostingdialog::accept);

	QPushButton *savePriceButton = new QPushButton("保存模板");
	savePriceButton->setIcon(QIcon(":/res/keyboard-24.png"));
	connect(savePriceButton, &QPushButton::clicked, this, &Addcostingdialog::savePriceTemplate);

	QHBoxLayout *hlayout = new QHBoxLayout;

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/add.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hlayout->addWidget(addButton);
	connect(addButton, &QPushButton::clicked, this, &Addcostingdialog::add);

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/minus.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hlayout->addWidget(minusButton);
	connect(minusButton, &QPushButton::clicked, this, &Addcostingdialog::remove);

	hlayout->addStretch();

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("核算年月"), 0, 0);
	mainLayout->addWidget(new QLabel("核算明细"), 1, 0);
	mainLayout->addWidget(_dateEdit, 0, 1);

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	vLayout->addWidget(_view);
	vLayout->addLayout(hlayout);

	mainLayout->addLayout(vLayout, 1, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 2, 0, 1, 2);

	QHBoxLayout *hLayout = new QHBoxLayout(this);
	hLayout->addWidget(savePriceButton, Qt::AlignHCenter);
	hLayout->addWidget(submitButton, Qt::AlignHCenter);

	mainLayout->addLayout(hLayout, 3, 1);

	resize(parent ? parent->width() / 2 : 720, sizeHint().height() * 1.5);
}

Addcostingdialog::~Addcostingdialog()
{
}

void Addcostingdialog::add()
{
	QList<QStandardItem *> rowItems;

	rowItems.append(new QStandardItem());
	rowItems.append(new QStandardItem());
	rowItems.append(new QStandardItem());

	QStandardItem *item_total = new QStandardItem();
	item_total->setTextAlignment(Qt::AlignCenter);
	item_total->setText(QString::number(0.00f, 'f', 2));
	item_total->setEditable(false);
	rowItems.append(item_total);

	_model->insertRow(_model->rowCount() - 1, rowItems);
}

void Addcostingdialog::reSumTotal()
{
	float sum = 0.0f;
	for (int i = 0; i < _model->rowCount() - 1; i++) {
		sum += _model->item(i, 1)->text().toFloat() * _model->item(i, 2)->text().toInt();
	}

	_model->item(_model->rowCount() - 1, 3)->setText(QString::number(sum, 'f', 2));
}

void Addcostingdialog::remove()
{
	QModelIndexList selectRows = _theSelectionModel->selectedRows();
	for each (QModelIndex index in selectRows)
	{
		int row = index.row();
		if (row == _model->rowCount() - 1) continue;
		_model->removeRow(row);
	}

	reSumTotal();
}

void Addcostingdialog::tableItemChanged(QStandardItem *item)
{
	if (item->column() == 1 || item->column() == 2) {	//重新计算总价

		float sum = _model->item(item->row(), 1)->text().toFloat() * _model->item(item->row(), 2)->text().toInt();

		float total = _model->item(_model->rowCount() - 1, 3)->text().toFloat() - _model->item(item->row(), 3)->text().toFloat() + sum;

		_model->item(item->row(), 3)->setText(QString::number(sum, 'f', 2));
		_model->item(_model->rowCount() - 1, 3)->setText(QString::number(total, 'f', 2));
	}
}

void Addcostingdialog::dateChanged(const QDate &date)
{
	(Q_NULLPTR == _currentMonthPacktypeCountMap)?_currentMonthPacktypeCountMap = new QVariantMap():_currentMonthPacktypeCountMap->clear();
	(Q_NULLPTR == _priceMap) ? _priceMap = new QVariantMap() : _priceMap->clear();

	QByteArray data;
	data.append("{\"month\":\"").append(date.toString("yyyyMM")).append("\"}");

	post(url(PATH_COST_GETALL), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取该月明细: ").append(resp.errorString()));
			return;
		}

		_model->removeRows(0, _model->rowCount());

		const QString code = resp.getAsString("code");

		if (!code.compare("9000")) {
			QVariantList packTypesList = resp.getAsList("pack_types");
			QVariantList pricesList = resp.getAsList("prices");

			if (!packTypesList.isEmpty())
			{
				for (auto &packType : packTypesList) {
					QVariantMap map = packType.toMap();
					_currentMonthPacktypeCountMap->insert(map["pack_type_name"].toString(), map["pack_type_num"].toString());
				}
			}

			if (!pricesList.isEmpty())
			{
				for (auto &price : pricesList) {
					QVariantMap map = price.toMap();
					_priceMap->insert(map["name"].toString(), map["price"].toString());
				}
			}

			QVariantList costingDetailList = resp.getAsList("costings");
			float sum = 0.0f;
			if (!costingDetailList.isEmpty())				//若当月已经存在成本核算数据，则显示并支持修改后重新提交。
			{
				for (auto &detail : costingDetailList) {
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
					item_total->setEditable(false);
					sum += map["total"].toFloat();
					rowItems.append(item_total);

					_model->appendRow(rowItems);
				}
			}
			else {											//若当前尚无成本核算数据，则显示模板数据。
				for (auto &price : pricesList) {
					QVariantMap map = price.toMap();

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
					if (_currentMonthPacktypeCountMap->contains(map["name"].toString()))
					{
						item_num->setText(_currentMonthPacktypeCountMap->value(map["name"].toString()).toString());
						item_price->setEditable(false);
					}
					else
						item_num->setText("0");
					rowItems.append(item_num);

					QStandardItem *item_total = new QStandardItem();
					item_total->setTextAlignment(Qt::AlignCenter);
					float total = item_price->text().toFloat() * item_num->text().toInt();
					item_total->setText(QString::number(total, 'f', 2));
					item_total->setEditable(false);
					sum += total;
					rowItems.append(item_total);

					_model->appendRow(rowItems);
				}
				
				QVariantMap::const_iterator i;
				for (i = _currentMonthPacktypeCountMap->constBegin();
					i != _currentMonthPacktypeCountMap->constEnd();
					++i) {
					if (_priceMap->contains(i.key())) continue;

					QList<QStandardItem *> rowItems;
					QStandardItem *item_name = new QStandardItem();
					item_name->setTextAlignment(Qt::AlignCenter);
					item_name->setText(i.key());
					rowItems.append(item_name);

					QStandardItem *item_price = new QStandardItem();
					item_price->setTextAlignment(Qt::AlignCenter);
					item_price->setText("0.0");
					rowItems.append(item_price);

					QStandardItem *item_num = new QStandardItem();
					item_num->setTextAlignment(Qt::AlignCenter);
					item_num->setText(i.value().toString());
					rowItems.append(item_num);

					QStandardItem *item_total = new QStandardItem();
					item_total->setTextAlignment(Qt::AlignCenter);
					item_total->setText(QString::number(0.00f, 'f', 2));
					item_total->setEditable(false);
					rowItems.append(item_total);

					_model->appendRow(rowItems);
				}
				
			}
			
			

			QList<QStandardItem *> rowItems;
			QStandardItem *item_name = new QStandardItem();
			item_name->setTextAlignment(Qt::AlignCenter);
			item_name->setText("总计");
			item_name->setEditable(false);
			rowItems.append(item_name);

			QStandardItem *item_1 = new QStandardItem();
			item_1->setTextAlignment(Qt::AlignCenter);
			item_1->setText("-");
			item_1->setEditable(false);

			QStandardItem *item_2 = new QStandardItem();
			item_2->setTextAlignment(Qt::AlignCenter);
			item_2->setText("-");
			item_2->setEditable(false);

			rowItems.append(item_1);
			rowItems.append(item_2);

			QStandardItem *item_total = new QStandardItem();
			item_total->setTextAlignment(Qt::AlignCenter);
			item_total->setText(QString::number(sum, 'f', 2));
			item_total->setEditable(false);
			rowItems.append(item_total);

			_model->appendRow(rowItems);
		}
		else {
			XNotifier::warn(QString("无法获取该月明细: ").append(resp.getAsString("msg")));
			return;
		}

	});
}

void Addcostingdialog::accept() {
	QString szDate = _dateEdit->text();
	if (szDate.isEmpty()) {
		_dateEdit->setFocus();
		return;
	}

	QVariantList costings;
	for (int i = 0; i < _model->rowCount() - 1; i++) {
		QVariantMap map;
		map.insert("name", _model->item(i, 0)->text());
		map.insert("price", _model->item(i, 1)->text());
		map.insert("num", _model->item(i, 2)->text());
		map.insert("total", _model->item(i, 3)->text());

		costings.append(map);
	}
	QVariantMap vmap;
	vmap.insert("month", szDate);
	vmap.insert("costings", costings);

	Core::app()->startWaitingOn(this);
	post(url(PATH_COST_ADD), vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//return; // TODO
		}
		else {
			QDialog::accept();
		}
	});
}

void Addcostingdialog::initCostView() {
	_model->setHeaderData(0, Qt::Horizontal, "核算项目");
	_model->setHeaderData(1, Qt::Horizontal, "单价");
	_model->setHeaderData(2, Qt::Horizontal, "数量");
	_model->setHeaderData(3, Qt::Horizontal, "总价");

	_theSelectionModel = new QItemSelectionModel(_model);
	_view->setModel(_model);
	_view->setSelectionModel(_theSelectionModel);
	
	_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	//_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 200);

	connect(_model, SIGNAL(itemChanged(QStandardItem *)), this, SLOT(tableItemChanged(QStandardItem *)));
}

void Addcostingdialog::savePriceTemplate() {
	
	QVariantList prices;
	for (int i = 0; i < _model->rowCount() - 1; i++) {
		QVariantMap map;
		map.insert("name", _model->item(i, 0)->text());
		map.insert("price", _model->item(i, 1)->text());
		prices.append(map);
	}

	QVariantMap vmap;
	vmap.insert("prices", prices);

	Core::app()->startWaitingOn(this);
	post(url(PATH_PRICE_ADD), vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//return; // TODO
		}
		else {
			//return; // TODO
		}
	});
}