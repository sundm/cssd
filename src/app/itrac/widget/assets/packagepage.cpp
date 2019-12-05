#include "packagepage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/addpackagedialog.h"
#include "dialog/modifypackagedialog.h"
#include "dialog/addpkgcodedialog.h"
#include "rdao/dao/PackageDao.h"
#include <xui/searchedit.h>

#include <QtWidgets/QtWidgets>

namespace Internal {
	PackageAssetView::PackageAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Department + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "包名");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
		_model->setHeaderData(SteType, Qt::Horizontal, "适用灭菌类型");
		_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void PackageAssetView::load(const QString &kw, int page, int count)
	{
		PackageDao dao;
		QList<PackageType> pts;

		result_t resp = dao.getPackageTypeList(&pts);
		if (resp.isOk())
		{
			clear(); // when succeeded

			_model->insertRows(0, pts.count());
			for (int i = 0; i != pts.count(); ++i) {
				PackageType pt = pts[i];
				_model->setData(_model->index(i, Name), pt.name);
				_model->setData(_model->index(i, Name), pt.typeId, 257);
				_model->setData(_model->index(i, Pinyin), pt.pinyin);
				_model->setData(_model->index(i, PackType), pt.packType.name);
				_model->setData(_model->index(i, PackType), pt.packType.id, 257);
				_model->setData(_model->index(i, SteType), Internal::literalSteType(pt.sterMethod));
				_model->setData(_model->index(i, SteType), Internal::brushForSteType(pt.sterMethod), Qt::BackgroundRole);
				_model->setData(_model->index(i, SteType), pt.sterMethod, 257);
				_model->setData(_model->index(i, Department), pt.dept.name);
				_model->setData(_model->index(i, Department), pt.dept.id, 257);
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
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reflash()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	//Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	//connect(modifyButton, SIGNAL(clicked()), this, SLOT(editEntry()));

	//searchBox->setMinimumWidth(300);
	_searchBox->setPlaceholderText("输入包名搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &PackagePage::search);

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

void PackagePage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	editRow(row);
}

void PackagePage::reflash()
{
	QTimer::singleShot(0, [this] {_view->load(); });
}

void PackagePage::addEntry()
{
	AddPackageDialog d(this);
	if (QDialog::Accepted == d.exec())
		reflash();
}

void PackagePage::editEntry()
{
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	editRow(row);
}

//void PackagePage::infoEntry()
//{
//	QModelIndexList indexes = _view->selectionModel()->selectedRows();
//	if (indexes.count() == 0) return;
//	int row = indexes[0].row();
//	QString package_name = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name)).toString();
//	QString package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name), 257).toString();
//	AddpkgcodeDialog d(this);
//	if (QDialog::Accepted == d.exec())
//		reflash();
//}

void PackagePage::editRow(int row)
{
	QString package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name), 257).toString();
	AddPackageDialog d(this);
	d.setInfo(package_type_id);
	if (QDialog::Accepted == d.exec())
		reflash();
	//QString package_name = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name)).toString();
	//QString package_type_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Name), 257).toString();
	//QString pack_type = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::PackType)).toString();
	//QString package_category = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::PackType), 257).toString();
	//int steType = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::SteType), 257).toInt();
	//QString package_pinyin = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Pinyin)).toString();
	//QString dtp_name = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Department)).toString();
	//QString dtp_id = _view->model()->data(_view->model()->index(row, Internal::PackageAssetView::Department), 257).toString();

	//PackageInfo info;
	//info.package_name = package_name;
	//info.package_type_id = package_type_id;
	//info.pack_type = pack_type;
	//info.package_category = package_category;
	//info.steType = steType;
	//info.package_pinyin = package_pinyin;
	//info.dtp_name = dtp_name;
	//info.dtp_id = dtp_id;

	//QByteArray data("{\"package_type_id\":");
	//data.append(package_type_id).append('}');
	//_http.post(url(PATH_PKGDETAIL_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
	//	JsonHttpResponse resp(reply);
	//	if (!resp.success()) {
	//		XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
	//		return;
	//	}

	//	QList<QVariant> orders = resp.getAsList("instruments");

	//	ModifyPackageDialog d(this, info, orders);
	//	if (QDialog::Accepted == d.exec())
	//		reflash();
	//});
}

void PackagePage::search() {
	_view->load(_searchBox->text());
}
