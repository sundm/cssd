#include "instrumentIdpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/composite/qpaginationwidget.h"
#include "dialog/addinstrumentIddialog.h"
#include "dialog/batchaddinstrumentIddialog.h"
#include "rdao/dao/InstrumentDao.h"
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

InstrumentIdPage::InstrumentIdPage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::InstrumentIdAssetView)
	, _searchBox(new SearchEdit)
	, _paginator(new QPaginationWidget)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &InstrumentIdPage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &InstrumentIdPage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/datchadd.png", "批量添加");
	connect(modifyButton, &QToolButton::clicked, this, &InstrumentIdPage::modify);

	_searchBox->setPlaceholderText("输入器械名搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &InstrumentIdPage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);

	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &InstrumentIdPage::doSearch);
	hLayout->addWidget(_paginator);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

	refresh();
}

void InstrumentIdPage::refresh() {
	_searchBox->clear();
	doSearch(1);
}

void InstrumentIdPage::add() {
	AddInstrumentIdDialog d(this);
	connect(&d, SIGNAL(reload()), this, SLOT(refresh()));
	d.exec();
}

void InstrumentIdPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Id)).toString();

	AddInstrumentIdDialog d(this);
	d.setInfo(id);
	if (d.exec() == QDialog::Accepted)
		refresh();
}

void InstrumentIdPage::modify() {
	BatchAddInstrumentIdDialog d(this);
	connect(&d, SIGNAL(reload()), this, SLOT(refresh()));
	d.exec();
	/*QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Id)).toString();

	AddInstrumentIdDialog d(this);
	d.setInfo(id);
	if (d.exec() == QDialog::Accepted)
		_view->load();*/
}

namespace Internal {
	InstrumentIdAssetView::InstrumentIdAssetView(QWidget *parent /*= nullptr*/)
		: PaginationView(parent)
		, _model(new QStandardItemModel(0, Id + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "器械名");
		_model->setHeaderData(Id, Qt::Horizontal, "器械UDI");
		//_model->setHeaderData(Basics, Qt::Horizontal, "所属基础器械名");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);

		setPageCount(30);
	}

	void InstrumentIdAssetView::load(const QString &kw, int page /*= 0*/)
	{
		InstrumentDao dao;
		QList<Instrument> ins;
		result_t res = dao.getInstrumentList(&ins, page, _pageCount);
		if (res.isOk())
		{
			clear();
			_model->insertRows(0, ins.count());
			for (int i = 0; i != ins.count(); ++i) {
				Instrument it = ins[i];
				_model->setData(_model->index(i, Name), it.name);
				_model->setData(_model->index(i, Name), it.typeId, Qt::UserRole + 1);
				_model->setData(_model->index(i, Id), it.udi);

				_model->setHeaderData(i, Qt::Vertical, (page - 1)*_pageCount + 1 + i);
				//InstrumentType ity;
				//dao.getInstrumentType(it.typeId, &ity);
				//_model->setData(_model->index(i, Basics), ity.name);
				//_model->setData(_model->index(i, Basics), it.typeId, Qt::UserRole + 1);
			}
		}
		else
		{
			XNotifier::warn(QString("获取器械列表失败: ").append(res.msg()));
			return;
		}
	}
}

void InstrumentIdPage::doSearch(int page)
{
	_view->load(_searchBox->text(), page);
}

void InstrumentIdPage::search() {
	_view->load(_searchBox->text(), 1);
	//_paginator->setTotalPages(count / pageCount + (count % pageCount > 0));
	_paginator->setTotalPages(1);//todo
}