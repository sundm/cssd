#include "departmentpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "dialog/adddptdialog.h"
#include "ui/buttons.h"
#include <QtWidgets/QtWidgets>

DepartmentPage::DepartmentPage(QWidget *parent)
	: QWidget(parent), _view(new Internal::DeptAssetView)
{
	QHBoxLayout *hLayout = new QHBoxLayout;

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, SIGNAL(clicked()), this, SLOT(modify()));

	hLayout->addWidget(addButton);
	hLayout->addWidget(modifyButton);
	hLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	QTimer::singleShot(0, [this] {_view->load(); });
}

void DepartmentPage::addEntry() {
	AddDptDialog d(this);
	if (d.exec() == QDialog::Accepted)
	{
		_view->load();
	}
}

void DepartmentPage::modify() {
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();

	QString name = _view->model()->data(_view->model()->index(row, 0)).toString();
	QString id = _view->model()->data(_view->model()->index(row, 1)).toString();
	QString pinyin = _view->model()->data(_view->model()->index(row, 2)).toString();
	QString phone = _view->model()->data(_view->model()->index(row, 3)).toString();

	AddDptDialog d(this);
	d.setDtpInfo(id, name, pinyin, phone);
	if (d.exec() == QDialog::Accepted)
		_view->load();
}

namespace Internal {
	DeptAssetView::DeptAssetView(QWidget *parent /*= nullptr*/)
		: TableView(parent)
		, _model(new QStandardItemModel(0, Phone + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "科室名称");
		_model->setHeaderData(Id, Qt::Horizontal, "编号");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		_model->setHeaderData(Phone, Qt::Horizontal, "联系方式");
		setModel(_model);

		setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	void DeptAssetView::load(int page /*= 0*/, int count /*= 10*/)
	{
		_http.post(url(PATH_DEPT_SEARCH), "{}", [=](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("获取科室列表失败: ").append(resp.errorString()));
				return;
			}

			clear(); // when succeeded

			QList<QVariant> pkgs = resp.getAsList("department_list");
			_model->insertRows(0, pkgs.count());
			for (int i = 0; i != pkgs.count(); ++i) {
				QVariantMap map = pkgs[i].toMap();
				_model->setData(_model->index(i, Id), map["department_id"]);
				_model->setData(_model->index(i, Name), map["department_name"]);
				_model->setData(_model->index(i, Pinyin), map["pinyin_code"]);
				_model->setData(_model->index(i, Phone), map["phone"]);
			}
		});
	}

	void DeptAssetView::clear() {
		_model->removeRows(0, _model->rowCount());
	}

}
