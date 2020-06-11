#include "packagepage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/composite/qpaginationwidget.h"
#include "dialog/addpackagedialog.h"
#include "dialog/modifypackagedialog.h"
#include "dialog/addpkgcodedialog.h"
#include "rdao/dao/PackageDao.h"
#include <xui/searchedit.h>

#include <QtWidgets/QtWidgets>

namespace Internal {
	PackageAssetView::PackageAssetView(QWidget *parent /*= nullptr*/)
		: PaginationView(parent)
		, _model(new QStandardItemModel(0, Department + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "包名");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
		_model->setHeaderData(SteType, Qt::Horizontal, "适用灭菌类型");
		_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);

		QHeaderView *header = horizontalHeader();
		header->setStretchLastSection(true);
		header->resizeSection(0, 350);
		header->resizeSection(1, 250);
		header->resizeSection(2, 250);
		header->resizeSection(3, 250);

		setPageCount(30);
	}

	void PackageAssetView::load(const QString &kw, int page)
	{
		PackageDao dao;
		QList<PackageType> pts;
		_total = 0;
		result_t resp = dao.getPackageTypeList(&pts, kw, &_total, page, _pageCount);
		if (resp.isOk())
		{
			clear(); // when succeeded
			_model->insertRows(0, pts.count());
			for (int i = 0; i != pts.count(); ++i) {
				PackageType pt = pts[i];
				_model->setData(_model->index(i, Name), pt.typeName);
				_model->setData(_model->index(i, Name), pt.typeId, 257);
				_model->setData(_model->index(i, Pinyin), pt.pinyin);
				_model->setData(_model->index(i, PackType), pt.packType.name);
				_model->setData(_model->index(i, PackType), pt.packType.id, 257);
				_model->setData(_model->index(i, SteType), Internal::literalSteType(pt.sterMethod));
				_model->setData(_model->index(i, SteType), Internal::brushForSteType(pt.sterMethod), Qt::BackgroundRole);
				_model->setData(_model->index(i, SteType), pt.sterMethod, 257);
				_model->setData(_model->index(i, Department), pt.dept.name);
				_model->setData(_model->index(i, Department), pt.dept.id, 257);

				_model->setHeaderData(i, Qt::Vertical, (page - 1)*_pageCount + 1 + i);
			}
		}
		else
		{
			XNotifier::warn(QString("获取包列表失败: ").append(resp.msg()));
			return;
		}
	}

	QString literalSteType(int type) {
		switch (type) {
		case 1: return "低温";
		case 2: return "高温";
		default: return "通用";
		}
	}

	QBrush brushForSteType(int type) {
		switch (type) {
		case 1: return QBrush(QColor(173, 216, 230)); //lightsalmon
		case 2: return QBrush(QColor(255, 160, 122)); //lightblue #ADD8E6
		default: return QBrush();
		}
	}

}

PackagePage::PackagePage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::PackageAssetView)
	, _searchBox(new SearchEdit)
	, _paginator(new QPaginationWidget)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reflash()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *deleteButton = new Ui::IconButton(":/res/delete-24.png", "删除");
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(delEntry()));

	//searchBox->setMinimumWidth(300);
	_searchBox->setPlaceholderText("输入包名搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &PackagePage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(deleteButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);
	
	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &PackagePage::doSearch);
	hLayout->addWidget(_paginator);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 15, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	search();

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

}

void PackagePage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	editRow(row);
}

void PackagePage::reflash()
{
	_searchBox->clear();
	doSearch(1);
}

void PackagePage::addEntry()
{
	AddPackageDialog d(this);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackagePage::delEntry()
{
	QMessageBox *messageBox = new QMessageBox(this);
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setWindowTitle("提示");
	messageBox->setText("是否删除当前基础包类型？");
	messageBox->addButton("取消", QMessageBox::RejectRole);
	messageBox->addButton("确定", QMessageBox::AcceptRole);
	if (messageBox->exec() == QDialog::Accepted) {
		QModelIndexList indexes = _view->selectionModel()->selectedRows();
		if (indexes.count() == 0) return;
		int row = indexes[0].row();
		int package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name), 257).toInt();

		PackageDao dao;
		result_t res = dao.delPackageType(package_type_id);
		if (res.isOk())
		{
			reflash();
		}
		else
		{
			XNotifier::warn(res.msg());
			return;
		}
	}
}

void PackagePage::editEntry()
{
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	editRow(row);
}

void PackagePage::editRow(int row)
{
	QString package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name), 257).toString();
	AddPackageDialog d(this);
	d.setInfo(package_type_id);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackagePage::doSearch(int page)
{
	_view->load(_searchBox->text(), page);
}

void PackagePage::search() {
	_view->load(_searchBox->text(), 1);
	_paginator->setTotalPages(_view->totalCount() / _view->pageCount() + (_view->totalCount() % _view->pageCount() > 0));
}
