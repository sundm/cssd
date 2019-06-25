#include "importextdialog.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include <xernel/xtimescope.h>
#include <QVBoxLayout>
#include <QStandardItemModel>

ImportExtDialog::ImportExtDialog(QWidget *parent)
	: QDialog(parent)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0,7,_view))
{
	_model->setHeaderData(0, Qt::Horizontal, "器械名称");
	_model->setHeaderData(1, Qt::Horizontal, "供应商");
	_model->setHeaderData(2, Qt::Horizontal, "送货人");
	_model->setHeaderData(3, Qt::Horizontal, "接收人");
	_model->setHeaderData(4, Qt::Horizontal, "接收时间");
	_model->setHeaderData(5, Qt::Horizontal, "患者姓名");
	_model->setHeaderData(6, Qt::Horizontal, "住院号");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(_view);

	Ui::PrimaryButton *okButton = new Ui::PrimaryButton("确定");
	connect(okButton, &QPushButton::clicked, this, &ImportExtDialog::accept);
	layout->addWidget(okButton);

	resize(parent->width()/2, sizeHint().height());
	
	loadExt();
}

ImportExtDialog::~ImportExtDialog() {
}

void ImportExtDialog::accept() {
}

void ImportExtDialog::loadExt() {
	QVariantMap vmap;
	XDateScope timeScope(XPeriod::RecentWeek);
	vmap.insert("start_time", timeScope.from);
	vmap.insert("end_time", timeScope.to);

	Url::post(Url::PATH_EXT_SEARCH, vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("ext_info");
		_model->insertRows(0, orders.count());
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();
			_model->setData(_model->index(i, 0), map["pkg_type_name"]);
			_model->setData(_model->index(i, 1), map["sup_name"]);
			_model->setData(_model->index(i, 2), map["send_name"]);
			_model->setData(_model->index(i, 3), map["p_operator_name"]);
			_model->setData(_model->index(i, 4), map["p_date"]);
			_model->setData(_model->index(i, 5), map["patient_name"]);
			_model->setData(_model->index(i, 6), map["admission"]);
		}
	});
}
