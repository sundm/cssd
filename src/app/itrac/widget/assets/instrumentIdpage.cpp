#include "instrumentIdpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "dialog/addinstrumentIddialog.h"
#include <QtWidgets/QtWidgets>

InstrumentIdPage::InstrumentIdPage(QWidget *parent)
	: QWidget(parent), _view(new Internal::InstrumentIdAssetView)
{
	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &InstrumentIdPage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &InstrumentIdPage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, &QToolButton::clicked, this, &InstrumentIdPage::modify);

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

void InstrumentIdPage::refresh() {
	_view->load();
}

void InstrumentIdPage::add() {
	AddInstrumentIdDialog d(this);
	d.exec();
}

void InstrumentIdPage::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();

	QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Name)).toString();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Id)).toString();
	QString basics = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Basics)).toString();

	AddInstrumentIdDialog d(this);
	d.setInfo(id, name, basics);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

void InstrumentIdPage::modify() {
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	QString name = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Name)).toString();
	QString id = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Id)).toString();
	QString basics = _view->model()->data(_view->model()->index(row, Internal::InstrumentIdAssetView::Basics)).toString();

	AddInstrumentIdDialog d(this);
	d.setInfo(id, name, basics);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

namespace Internal {
	InstrumentIdAssetView::InstrumentIdAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Basics + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "器械名");
		_model->setHeaderData(Id, Qt::Horizontal, "器械UID");
		_model->setHeaderData(Basics, Qt::Horizontal, "所属基础器械名");
		setModel(_model);
		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void InstrumentIdAssetView::load(int page /*= 0*/, int count /*= 10*/)
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
			}
		});
	}
}
