#include "packagepage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/addpackagedialog.h"
#include <xui/searchedit.h>

#include <QtWidgets/QtWidgets>

namespace Internal {
	PackageAssetView::PackageAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Department + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "包名");
		_model->setHeaderData(PackType, Qt::Horizontal, "包装类型");
		_model->setHeaderData(SteType, Qt::Horizontal, "适用灭菌类型");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		_model->setHeaderData(Department, Qt::Horizontal, "所属科室");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void PackageAssetView::load(const QString &kw, int page, int count)
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
				_model->setData(_model->index(i, Name), map["package_type_id"], 257);
				_model->setData(_model->index(i, PackType), map["pack_type"]);
				int steType = map["sterilize_type"].toInt();
				_model->setData(_model->index(i, SteType), Internal::literalSteType(steType));
				_model->setData(_model->index(i, SteType), Internal::brushForSteType(steType), Qt::BackgroundRole);
				_model->setData(_model->index(i, Pinyin), map["pinyin_code"]);
				_model->setData(_model->index(i, Department), map["department_name"]);
			}
		});
	}

	QString literalSteType(int type) {
		switch (type) {
		case 1: return "高温";
		case 2: return "低温";
		default: return "通用";
		}
	}

	QBrush brushForSteType(int type) {
		switch (type) {
		case 1: return QBrush(QColor(255, 160, 122)); //lightsalmon
		case 2: return QBrush(QColor(173, 216, 230)); //lightblue #ADD8E6
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

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, SIGNAL(clicked()), this, SLOT(editEntry()));

	Ui::IconButton *infoButton = new Ui::IconButton(":/res/info-24.png", "查看包详情");
	connect(infoButton, SIGNAL(clicked()), this, SLOT(infoEntry()));

	//searchBox->setMinimumWidth(300);
	_searchBox->setPlaceholderText("输入包名/拼音码搜索");
	connect(_searchBox, &SearchEdit::returnPressed, this, &PackagePage::search);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(modifyButton);
	hLayout->addWidget(infoButton);
	hLayout->addStretch(0);
	hLayout->addWidget(_searchBox);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 15, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	QTimer::singleShot(0, [this] {_view->load(); });
}

void PackagePage::reflash()
{

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

	QString id = _view->model()->data(_view->model()->index(row, 0), 257).toString();

}

void PackagePage::infoEntry()
{

}

void PackagePage::search() {
	_view->load(_searchBox->text());
}
