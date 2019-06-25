#include <qstandarditemmodel>
#include "SelectPkgDialog.h"
#include "jsonhttpclient.h"
#include "jsonhttpresponse.h"
#include "deptpicker.h"
#include "urls.h"
#include "xnotifier.h"

SelectPkgDialog::SelectPkgDialog(QWidget *parent)
	:QDialog(parent),
	_depModel(new QStandardItemModel(this)),
	_pkgModel(new QStandardItemModel(this)),
	_searchDeptWidget(nullptr),
	_currentDeptId(0)
{
	setupUi(this);
	filterPkgEdit->setPlaceholderText("包筛选...");
	filterPkgEdit->setClearButtonEnabled(true);

	pkgView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	pkgView->setModel(_pkgModel);

	connect(searchButton, &QAbstractButton::clicked, this, &SelectPkgDialog::searchDept);
}

SelectPkgDialog::~SelectPkgDialog()
{
}

void SelectPkgDialog::accept()
{
	QModelIndex pkgIndex = pkgView->currentIndex();
	if (!pkgIndex.isValid())
		return;

	QDialog::accept();

	QString pkgName = _pkgModel->data(pkgIndex).toString();
	int pkgId = _pkgModel->data(pkgIndex, Qt::UserRole + 1).toInt();
	emit packageSelected(deptLabel->text(), _currentDeptId, pkgName, pkgId);
}

void SelectPkgDialog::searchDept()
{
	if (!_searchDeptWidget) {
		_searchDeptWidget = new DeptPicker(this);
		connect(_searchDeptWidget, &DeptPicker::deptPicked, this, &SelectPkgDialog::onDeptChanged);
	}
	
	_searchDeptWidget->exec();
}

void SelectPkgDialog::onDeptChanged(int id, const QString &name) {
	_currentDeptId = id;
	deptLabel->setText(name);
	_pkgModel->removeRows(0, _pkgModel->rowCount());

	QByteArray data("{\"department_id\":");
	data.append(QString::number(id)).append('}');
	JsonHttpClient().post(url(PATH_PKGTPYE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(this, QString("无法获取器械列表，请关闭对话框重试：").append(resp.errorString()), -1);
			return;
		}

		QList<QVariant> instruments = resp.getAsList("package_types");
		foreach(auto &instrument, instruments) {
			QVariantMap map = instrument.toMap();
			QStandardItem *pkgItem = new QStandardItem(map["package_name"].toString());
			pkgItem->setData(map["package_type_id"]);
			_pkgModel->appendRow(pkgItem);
		}
	});
}