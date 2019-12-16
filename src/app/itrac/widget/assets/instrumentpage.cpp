#include "instrumentpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/composite/qpaginationwidget.h"
#include "dialog/addinstrumentdialog.h"
#include "rdao/dao/InstrumentDao.h"
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

QString getVipLiteral(const bool &isVip) {
	return isVip ? "是" : "否";
}

InstrumentPage::InstrumentPage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::InstrumentAssetView)
	, _searchBox(new SearchEdit)
	, _paginator(new QPaginationWidget)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &InstrumentPage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &InstrumentPage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, &QToolButton::clicked, this, &InstrumentPage::modify);

	_searchBox->setPlaceholderText("输入器械名搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &InstrumentPage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	//hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);

	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &InstrumentPage::doSearch);
	hLayout->addWidget(_paginator);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

	refresh();
}

void InstrumentPage::refresh() {
	_searchBox->clear();
	doSearch(1);
}

void InstrumentPage::add() {
	AddInstrumentDialog d(this);
	if (d.exec() == QDialog::Accepted)
		refresh();
}

void InstrumentPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name), Qt::UserRole + 1).toString();
	/*QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name)).toString();
	QString pinyin = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Pinyin)).toString();
	QString vip = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Vip)).toString();
	QString implant = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Implant)).toString();

	bool isVIP = false;
	if (0 == vip.compare("是"))
		isVIP = true;

	bool isImplant = false;
	if (0 == implant.compare("是"))
		isImplant = true;
*/
	AddInstrumentDialog d(this);
	d.setInfo(id);
	if (d.exec() == QDialog::Accepted)
		refresh();
}

void InstrumentPage::modify() {
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name), Qt::UserRole + 1).toString();
	/*
	QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name)).toString();
	//QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Id)).toString();
	QString vip = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Vip)).toString();
	QString implant = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Implant)).toString();
	QString pinyin = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Pinyin)).toString();

	bool isVIP = false;
	if (0 == vip.compare("是"))
		isVIP = true;

	bool isImplant = false;
	if (0 == implant.compare("是"))
		isImplant = true;
	*/
	AddInstrumentDialog d(this);
	d.setInfo(id);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

namespace Internal {
	InstrumentAssetView::InstrumentAssetView(QWidget *parent /*= nullptr*/)
		: PaginationView(parent)
		, _model(new QStandardItemModel(0, Vip + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "器械名");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		_model->setHeaderData(Vip, Qt::Horizontal, "是否贵重器械");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
		setPageCount(30);
	}

	void InstrumentAssetView::load(const QString &kw, int page /*= 0*/)
	{
		InstrumentDao dao;
		QList<InstrumentType> ins;
		_total = 0;
		result_t res = dao.getInstrumentTypeList(&ins, &_total, page, _pageCount);
		if (res.isOk())
		{
			clear();
			_model->insertRows(0, ins.count());
			for (int i = 0; i != ins.count(); ++i) {
				InstrumentType inst = ins[i];
				_model->setData(_model->index(i, Name), inst.name);
				_model->setData(_model->index(i, Name), inst.typeId, Qt::UserRole + 1);
				_model->setData(_model->index(i, Pinyin), inst.pinyin);
				_model->setData(_model->index(i, Vip), getVipLiteral(inst.isVip));

				_model->setHeaderData(i, Qt::Vertical, (page - 1)*_pageCount + 1 + i);
			}
		}
		else {
			XNotifier::warn(QString("获取器械列表失败: ").append(res.msg()));
			return;
		}
		//_http.post(url(PATH_INSTRUMENT_SEARCH), "{}", [=](QNetworkReply *reply) {
		//	JsonHttpResponse resp(reply);
		//	if (!resp.success()) {
		//		XNotifier::warn(QString("获取器械列表失败: ").append(resp.errorString()));
		//		return;
		//	}

		//	clear(); // when succeeded

		//	QList<QVariant> pkgs = resp.getAsList("instrument_list");
		//	_model->insertRows(0, pkgs.count());
		//	for (int i = 0; i != pkgs.count(); ++i) {
		//		QVariantMap map = pkgs[i].toMap();
		//		_model->setData(_model->index(i, Name), map["instrument_name"]);
		//		_model->setData(_model->index(i, Pinyin), map["pinyin_code"]);
		//		_model->setData(_model->index(i, Vip), Internal::getVipLiteral(map["is_vip_instrument"].toString()));
		//		_model->setData(_model->index(i, Implant), Internal::getImportLiteral(map["instrument_type"].toString()));
		//	}
		//});
	}
}

void InstrumentPage::doSearch(int page)
{
	_view->load(_searchBox->text(), page);
}

void InstrumentPage::search() {
	_view->load(_searchBox->text(), 1);
	_paginator->setTotalPages(_view->totalCount() / _view->pageCount()  + (_view->totalCount() % _view->pageCount() > 0));
	//_paginator->setTotalPages(1);//todo
}