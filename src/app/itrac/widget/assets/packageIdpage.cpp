#include "packageIdpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/composite/qpaginationwidget.h"
#include "dialog/addpackagedialog.h"
#include "dialog/modifypackagedialog.h"
#include "dialog/addpkgcodedialog.h"
#include "dialog/changeUDIdialog.h"
#include <xui/searchedit.h>
#include "rdao/dao/PackageDao.h"
#include <QtWidgets/QtWidgets>

namespace Internal {
	PackageIdAssetView::PackageIdAssetView(QWidget *parent /*= nullptr*/)
		: PaginationView(parent)
		, _model(new QStandardItemModel(0, Cycle + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "包名称");
		_model->setHeaderData(Id, Qt::Horizontal, "包UDI");
		_model->setHeaderData(Alias, Qt::Horizontal, "包别名");
		_model->setHeaderData(Basics, Qt::Horizontal, "所属包类型");
		_model->setHeaderData(Cycle, Qt::Horizontal, "循环次数");
		setModel(_model);

		QHeaderView *header = horizontalHeader();
		header->setStretchLastSection(true);
		header->resizeSection(0, 350);
		header->resizeSection(1, 350);
		header->resizeSection(2, 250);
		header->resizeSection(3, 250);

		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void PackageIdAssetView::load(const QString &kw, int page)
	{
		PackageDao dao;
		QList<Package> pkgs;
		result_t resp = dao.getPackageList(&pkgs, kw, &_total, page, _pageCount);
		_total = 0;
		if (resp.isOk())
		{
			clear(); // when succeeded

			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				Package pk = pkgs[i];
				_model->setData(_model->index(i, Name), pk.name);
				_model->setData(_model->index(i, Name), pk.typeId, Qt::UserRole + 1);
				_model->setData(_model->index(i, Id), pk.udi);

				_model->setData(_model->index(i, Alias), pk.alias);

				_model->setData(_model->index(i, Basics), pk.typeName);
				_model->setData(_model->index(i, Basics), pk.typeId, Qt::UserRole + 1);

				_model->setData(_model->index(i, Cycle), pk.cycle);

				_model->setHeaderData(i, Qt::Vertical, (page - 1)*_pageCount + 1 + i);
				//PackageType pkt;
				//dao.getPackageType(pk.typeId, &pkt);
				
			}
		}
		else
		{
			XNotifier::warn(QString("获取包列表失败: ").append(resp.msg()));
			return;
		}
	}
}

PackageIdPage::PackageIdPage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::PackageIdAssetView)
	, _searchBox(new SearchEdit)
	, _paginator(new QPaginationWidget)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reflash()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *deleteButton = new Ui::IconButton(":/res/delete-24.png", "删除");
	connect(deleteButton, SIGNAL(clicked()), this, SLOT(delEntry()));

	Ui::IconButton *editButton = new Ui::IconButton(":/res/write-24.png", "替换");
	connect(editButton, SIGNAL(clicked()), this, SLOT(editEntry()));

	//searchBox->setMinimumWidth(300);
	_searchBox->setPlaceholderText("输入ID搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &PackageIdPage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(deleteButton);
	hLayout->addWidget(editButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);

	connect(_paginator, &QPaginationWidget::currentPageChanged, this, &PackageIdPage::doSearch);
	hLayout->addWidget(_paginator);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 15, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	search();

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

}

void PackageIdPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	editRow(row);
}

void PackageIdPage::reflash()
{
	_searchBox->clear();
	doSearch(1);
}

void PackageIdPage::addEntry()
{
	AddpkgcodeDialog d(this);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackageIdPage::editEntry()
{
	//QModelIndexList indexes = _view->selectionModel()->selectedRows();
	//if (indexes.count() == 0) return;
	//int row = indexes[0].row();

	//editRow(row);
	//PackageDao dao;
	//dao.changePackageUDI("E2009A9050048AF000000203", "E2009A9050048AF000000204");
	ChangeUDIDialog d(this);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackageIdPage::delEntry()
{
	QMessageBox *messageBox = new QMessageBox(this);
	messageBox->setIcon(QMessageBox::Warning);
	messageBox->setWindowTitle("提示");
	messageBox->setText("是否删除当前包？");
	messageBox->addButton("取消", QMessageBox::RejectRole);
	messageBox->addButton("确定", QMessageBox::AcceptRole);
	if (messageBox->exec() == QDialog::Accepted) {
		QModelIndexList indexes = _view->selectionModel()->selectedRows();
		if (indexes.count() == 0) return;
		int row = indexes[0].row();
		QString package_udi = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Id)).toString();

		PackageDao dao;
		result_t res = dao.delPackage(package_udi);
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

void PackageIdPage::editRow(int row)
{
	QString package_name = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Name)).toString();
	QString package_id = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Id)).toString();

	AddpkgcodeDialog d(this);
	d.setPackageId(package_id);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackageIdPage::doSearch(int page)
{
	_view->load(_searchBox->text(), page);
}

void PackageIdPage::search() {
	_view->load(_searchBox->text(), 1);
	_paginator->setTotalPages(_view->totalCount() / _view->pageCount() + (_view->totalCount() % _view->pageCount() > 0));
	//_paginator->setTotalPages(1);//todo
}
