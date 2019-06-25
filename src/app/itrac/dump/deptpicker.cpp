#include <xui/searchedit.h>
#include <QTableView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QApplication>
#include "deptpicker.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "urls.h"

#define DEPT_ID_ROLE (Qt::UserRole+1)
#define DEPT_PINYIN_ROLE (Qt::UserRole+2)

DeptPicker::DeptPicker(QWidget *parent)
	: QDialog(parent),
	_deptModel(new QStandardItemModel(this)),
	_deptProxyModel(new QSortFilterProxyModel(this))
{
	setWindowTitle("选择科室");
	_searchEdit = new SearchEdit;
	_searchEdit->setMinimumHeight(32);
	_searchEdit->setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(_searchEdit);

	_deptView = new QTableView;
	_deptView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_deptView->setSelectionBehavior(QAbstractItemView::SelectRows);
	_deptView->setSelectionMode(QAbstractItemView::SingleSelection);
	_deptProxyModel->setSourceModel(_deptModel);
	_deptProxyModel->setFilterRole(DEPT_PINYIN_ROLE);
	_deptProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	_deptView->setModel(_deptProxyModel);
	connect(_searchEdit, &QLineEdit::textChanged, _deptProxyModel, &QSortFilterProxyModel::setFilterFixedString);
	connect(_deptView, &QAbstractItemView::doubleClicked, this, &DeptPicker::acceptEntry);

	QHeaderView *header = _deptView->horizontalHeader();
	header->setStretchLastSection(true);
	header->hide();

	_searchEdit->installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_searchEdit);
	layout->addWidget(_deptView);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);
	resize(300, 500);

	//setWindowFlags(Qt::Tool);
	//Qt::WindowModality modal = windowModality();
	//setWindowModality(Qt::ApplicationModal);

	JsonHttpClient().post(url(PATH_DEPT_SEARCH), "{}", [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//XNotifier::warn(this, QString("无法获取科室列表: ").append(resp.errorString()), -1);
			_searchEdit->setReadOnly(true);
			_searchEdit->setText("无法获取科室列表");
			return;
		}

		QList<QVariant> deps = resp.getAsList("department_list");
		for(auto &dep: deps) {
			QVariantMap map = dep.toMap();
			QStandardItem *depItem = new QStandardItem(map["department_name"].toString());
			depItem->setData(map["department_id"], DEPT_ID_ROLE);
			depItem->setData(map["pinyin_code"], DEPT_PINYIN_ROLE);
			_deptModel->appendRow(depItem);
		}
	});
}

DeptPicker::~DeptPicker()
{
}

bool DeptPicker::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == _searchEdit && QEvent::KeyPress == event->type()) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch (keyEvent->key()) {
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			QApplication::sendEvent(_deptView, event);
			return true;
		case Qt::Key_Escape:
			_searchEdit->clear();
			return true;
		case Qt::Key_Enter:
		case Qt::Key_Return:
			acceptCurrentEntry();
			return true;
		}
	}

	return QWidget::eventFilter(obj, event);
}

void DeptPicker::acceptCurrentEntry()
{
	const QModelIndex index = _deptView->currentIndex();
	if (!index.isValid())
		return;
	acceptEntry(index);
}

void DeptPicker::acceptEntry(const QModelIndex &index)
{
	QString deptName = _deptProxyModel->data(index).toString();
	int deptID = _deptProxyModel->data(index, DEPT_ID_ROLE).toInt();
	emit deptPicked(deptID, deptName);
	accept();
}

