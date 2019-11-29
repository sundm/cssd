#include "departmentpage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "dialog/adddptdialog.h"
#include "ui/buttons.h"
#include "rdao/dao/deptdao.h"
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
		DeptDao dao;
		QList<Department> depts;
		result_t resp = dao.getDeptList(&depts);
		if (resp.isOk())
		{
			clear(); // when succeeded

			_model->insertRows(0, depts.count());
			for (int i = 0; i != depts.count(); ++i) {
				Department dept = depts[i];
				_model->setData(_model->index(i, Id), dept.id);
				_model->setData(_model->index(i, Name), dept.name);
				_model->setData(_model->index(i, Pinyin), dept.pinyin);
				_model->setData(_model->index(i, Phone), dept.phone);
			}
		}
		else
		{
			XNotifier::warn(QString("获取科室列表失败: ").append(resp.msg()));
			return;
		}
	}

	void DeptAssetView::clear() {
		_model->removeRows(0, _model->rowCount());
	}

}
