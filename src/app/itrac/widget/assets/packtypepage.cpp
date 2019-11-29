#include "packtypepage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "core/constants.h"
#include "ui/buttons.h"
#include "dialog/packtypedialog.h"
#include <xui/searchedit.h>
#include "rdao/dao/PackageDao.h"
#include <QtWidgets/QtWidgets>

namespace Internal {
	PacktypeAssetView::PacktypeAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Max + 1, this))
	{
		_model->setHeaderData(Type, Qt::Horizontal, "包装类型");
		_model->setHeaderData(Valid, Qt::Horizontal, "当前有效期(天)");
		_model->setHeaderData(Max, Qt::Horizontal, "最大有效期(天)");
		setModel(_model);
		setSelectionMode(SingleSelection);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void PacktypeAssetView::load()
	{
		PackageDao dao;
		QList<PackType> pkgs;
		result_t resp = dao.getPackTypeList(&pkgs);
		if (resp.isOk())
		{
			clear(); // when succeeded
			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				PackType pt = pkgs[i];
				_model->setData(_model->index(i, Type), pt.name);
				_model->setData(_model->index(i, Type), pt.id, Constant::IdRole);
				_model->setData(_model->index(i, Valid), pt.validPeriod);
				_model->setData(_model->index(i, Max), pt.standardPeriod);
			}
		}
		else
		{
			XNotifier::warn(QString("获取包类型列表失败: ").append(resp.msg()));
			return;
		}
		/*
		QByteArray data;
		data.append("{}");
		_http.post(url(PATH_PACKTYPE_SEARCH), data, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("获取包类型列表失败: ").append(resp.errorString()));
				return;
			}

			clear(); // when succeeded

			QList<QVariant> pkgs = resp.getAsList("pack_types");
			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				QVariantMap map = pkgs[i].toMap();
				_model->setData(_model->index(i, PackType), map["pack_type_name"]);
				_model->setData(_model->index(i, PackType), map["pack_type_id"], Constant::IdRole);
				_model->setData(_model->index(i, Valid), map["valid_date"]);
				_model->setData(_model->index(i, Max), map["max_date"]);
			}
		});
		*/
	}

}

PacktypePage::PacktypePage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::PacktypeAssetView)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reload()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, SIGNAL(clicked()), this, SLOT(modify()));

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 15, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	QTimer::singleShot(0, [this] {_view->load(); });
}


void PacktypePage::reload()
{
	_view->load();
}

void PacktypePage::addEntry()
{
	AddPackTypeDialog d;
	if (d.exec() == QDialog::Accepted)
		reload();
}

void PacktypePage::modify()
{
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	QString name = _view->model()->data(_view->model()->index(row, 0)).toString();
	int typeId = _view->model()->data(_view->model()->index(row, 0), Constant::IdRole).toInt();
	int valid = _view->model()->data(_view->model()->index(row, 1)).toInt();
	int max = _view->model()->data(_view->model()->index(row, 2)).toInt();

	AddPackTypeDialog d;
	d.setInfo(name, typeId, valid, max);
	if (d.exec() == QDialog::Accepted)
		reload();
}