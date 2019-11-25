#include "packageIdpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/addpackagedialog.h"
#include "dialog/modifypackagedialog.h"
#include "dialog/addpkgcodedialog.h"
#include <xui/searchedit.h>

#include <QtWidgets/QtWidgets>

namespace Internal {
	PackageIdAssetView::PackageIdAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Basics + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "包名");
		_model->setHeaderData(Id, Qt::Horizontal, "包UID");
		_model->setHeaderData(Basics, Qt::Horizontal, "所属基础包");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void PackageIdAssetView::load(const QString &kw, int page, int count)
	{
		QVariantMap data;
		if (!kw.isEmpty()) {
			QRegExp re("[a-zA-Z]+");
			data.insert(re.exactMatch(kw) ? "pinyin_code" : "package_type_name", kw);
		}
		_http.post(url(PATH_PKGTPYE_SEARCH), data, [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("获取包列表失败: ").append(resp.errorString()));
				return;
			}

			clear(); // when succeeded

			QList<QVariant> pkgs = resp.getAsList("package_types");
			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				QVariantMap map = pkgs[i].toMap();
				_model->setData(_model->index(i, Name), map["package_name"]);
			}
		});
	}
}

PackageIdPage::PackageIdPage(QWidget *parent)
	: QWidget(parent)
	, _view(new Internal::PackageIdAssetView)
	, _searchBox(new SearchEdit)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reflash()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	/*Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, SIGNAL(clicked()), this, SLOT(editEntry()));*/

	//searchBox->setMinimumWidth(300);
	_searchBox->setPlaceholderText("输入ID搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &PackageIdPage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	//hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 15, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	QTimer::singleShot(0, [this] {_view->load(); });

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

}

void PackageIdPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	editRow(row);
}

void PackageIdPage::reflash()
{
	QTimer::singleShot(0, [this] {_view->load(); });
}

void PackageIdPage::addEntry()
{
	AddpkgcodeDialog d(this);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackageIdPage::editEntry()
{
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	editRow(row);
}

//void PackageIdPage::infoEntry()
//{
//	QModelIndexList indexes = _view->selectionModel()->selectedRows();
//	if (indexes.count() == 0) return;
//	int row = indexes[0].row();
//	QString package_name = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Name)).toString();
//	QString package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Name), 257).toString();
//	AddpkgcodeDialog d(this);
//	if (QDialog::Accepted == d.exec())
//		reflash();
//}

void PackageIdPage::editRow(int row)
{
	QString package_name = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Name)).toString();
	QString package_id = _view->model()->data(_view->model()->index(row, Internal::PackageIdAssetView::Id)).toString();

	AddpkgcodeDialog d(this);
	d.setPackageId(package_id);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackageIdPage::search() {
	_view->load(_searchBox->text());
}
