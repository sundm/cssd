#include "instrumentpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/addinstrumentdialog.h"

#include <QtWidgets/QtWidgets>

InstrumentPage::InstrumentPage(QWidget *parent)
	: QWidget(parent), _view(new Internal::InstrumentAssetView)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &InstrumentPage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &InstrumentPage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, &QToolButton::clicked, this, &InstrumentPage::modify);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->addWidget(refreshButton);
	hLayout->addWidget(addButton);
	hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

	refresh();
}

void InstrumentPage::refresh() {
	_view->load();
}

void InstrumentPage::add() {
	AddInstrumentDialog d(this);
	d.exec();
}

void InstrumentPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();

	QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name)).toString();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Id)).toString();
	QString vip = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Vip)).toString();
	QString implant = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Implant)).toString();
	QString pinyin = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Pinyin)).toString();

	bool isVIP = false;
	if (0 == vip.compare("是"))
		isVIP = true;

	bool isImplant = false;
	if (0 == implant.compare("是"))
		isImplant = true;

	AddInstrumentDialog d(this);
	d.setInfo(id, name, pinyin, isVIP, isImplant);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

void InstrumentPage::modify() {
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Name)).toString();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Id)).toString();
	QString vip = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Vip)).toString();
	QString implant = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Implant)).toString();
	QString pinyin = _view->model()->data(_view->model()->index(row, Internal::InstrumentAssetView::Pinyin)).toString();

	bool isVIP = false;
	if (0 == vip.compare("是"))
		isVIP = true;

	bool isImplant = false;
	if (0 == implant.compare("是"))
		isImplant = true;

	AddInstrumentDialog d(this);
	d.setInfo(id, name, pinyin, isVIP, isImplant);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

namespace Internal {
	InstrumentAssetView::InstrumentAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Pinyin + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "器械名");
		_model->setHeaderData(Id, Qt::Horizontal, "编号");
		_model->setHeaderData(Vip, Qt::Horizontal, "是否贵重器械");
		_model->setHeaderData(Implant, Qt::Horizontal, "是否为植入器械");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void InstrumentAssetView::load(int page /*= 0*/, int count /*= 10*/)
	{
		_http.post(url(PATH_INSTRUMENT_SEARCH), "{}", [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("获取器械列表失败: ").append(resp.errorString()));
				return;
			}

			clear(); // when succeeded

			QList<QVariant> pkgs = resp.getAsList("instrument_list");
			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				QVariantMap map = pkgs[i].toMap();
				_model->setData(_model->index(i, Name), map["instrument_name"]);
				_model->setData(_model->index(i, Id), map["instrument_id"]);
				_model->setData(_model->index(i, Vip), Internal::getVipLiteral(map["is_vip_instrument"].toString()));
				_model->setData(_model->index(i, Implant), Internal::getImportLiteral(map["instrument_type"].toString()));
				_model->setData(_model->index(i, Pinyin), map["pinyin_code"]);
			}
		});
	}

	QString getVipLiteral(const QString &vip) {
		return "0" == vip ? "否" : "是";
	}

	QString getImportLiteral(const QString &import) {
		return "1" == import ? "是" : "否";
	}
}
