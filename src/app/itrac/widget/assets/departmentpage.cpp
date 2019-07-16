#include "departmentpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"

#include <QtWidgets/QtWidgets>

DepartmentPage::DepartmentPage(QWidget *parent)
	: QWidget(parent), _view(new Internal::DeptAssetView)
{
	QHBoxLayout *hLayout = new QHBoxLayout;

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/add.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/minus.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch(0);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addLayout(hLayout);
	layout->addWidget(_view);

	QTimer::singleShot(0, [this] {_view->load(); });
}

namespace Internal {
	DeptAssetView::DeptAssetView(QWidget *parent /*= nullptr*/)
		:_model(new QStandardItemModel(0, Pinyin + 1, this))
	{
		_model->setHeaderData(Name, Qt::Horizontal, "科室名称");
		_model->setHeaderData(Id, Qt::Horizontal, "编号");
		_model->setHeaderData(Pinyin, Qt::Horizontal, "拼音检索码");
		setModel(_model);

		setEditTriggers(QAbstractItemView::NoEditTriggers);
		QHeaderView *header = horizontalHeader();
		header->setSectionResizeMode(QHeaderView::Stretch);
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
			}
		});
	}

	void DeptAssetView::clear() {
		_model->removeRows(0, _model->rowCount());
	}

}
