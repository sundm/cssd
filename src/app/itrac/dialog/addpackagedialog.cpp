#include "addpackagedialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include "ui/views.h"
#include "model/spinboxdelegate.h"
#include <thirdparty/qjson/src/parser.h>
#include <xui/images.h>
#include <xui/imageviewer.h>
#include <qhttpmultipart.h>
#include <qprocess.h>
#include <QNetworkreply>
#include <QtWidgets/QtWidgets>
#include <QDateTime>
#include <QTimer>
#include <QSpinBox>

AddPackageDialog::AddPackageDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgPYCodeEdit(new Ui::FlatEdit)
	, _pkgtypeBox(new QComboBox)
	, _picktypeBox(new QComboBox)
	, _stertypeBox(new QComboBox)
	, _deptEdit(new DeptEdit)
	, _insEdit(new InstrumentEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
{
	_isModfy = false;

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名 (*)", _pkgNameEdit);
	pkgGroup->addRow("拼音检索码 (*)", _pkgPYCodeEdit);
	pkgGroup->addRow("包类型 (*)", _pkgtypeBox);
	pkgGroup->addRow("打包类型 (*)", _picktypeBox);
	pkgGroup->addRow("高低温灭菌 (*)", _stertypeBox);
	pkgGroup->addRow("所属科室 (*)", _deptEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_insEdit);

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/plus-24.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/delete-24.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));
	
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->addLayout(hLayout);
	vlayout->addWidget(_view);

	setWindowTitle("添加包信息");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addLayout(vlayout);
	layout->addWidget(_commitButton);

	resize(600, 800);

	initInstrumentView();

	QTimer::singleShot(0, this, &AddPackageDialog::initData);

}

void AddPackageDialog::initData() {
	_pkgtypeBox->addItem(QString("手术包"), 0);
	_pkgtypeBox->addItem(QString("临床包"), 1);
	_pkgtypeBox->addItem(QString("外来包"), 2);
	_pkgtypeBox->addItem(QString("敷料包"), 3);
	_pkgtypeBox->addItem(QString("通用包"), 4);

	_stertypeBox->addItem(QString("通用"), 0);
	_stertypeBox->addItem(QString("高温"), 1);
	_stertypeBox->addItem(QString("低温"), 2);

	_picktypeBox->clear();

	QString data = QString("{}");

	post(url(PATH_PACKTYPE_SEARCH), QByteArray().append(data), [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
		}

		_picktypeBox->addItem(QString("不限"), 0);

		QList<QVariant> devices = resp.getAsList("pack_types");
		for (auto &device : devices) {
			QVariantMap map = device.toMap();
			_picktypeBox->addItem(map["pack_type_name"].toString(), map["pack_type_id"].toInt());
		}

	});

	_deptEdit->load(DeptEdit::ALL);
	_insEdit->load();
}

void AddPackageDialog::accept() {
	QString package_type_name = _pkgNameEdit->text();
	if (package_type_name.isEmpty()) {
		_pkgNameEdit->setFocus();
		return;
	}
		

	QString pinyin_code = _pkgPYCodeEdit->text();
	if (pinyin_code.isEmpty()) {
		_pkgPYCodeEdit->setFocus();
		return;
	}
		

	QString package_category = _pkgtypeBox->currentData().toString();

	int pack_type_id = _picktypeBox->currentData().toInt();
	if (0 == pack_type_id) {
		_picktypeBox->showPopup();
		return;
	}

	int sterilize_type = _stertypeBox->currentData().toInt();

	int department_id = _deptEdit->currentId();
	if (_deptEdit->currentName().isEmpty()) {
		_deptEdit->setFocus();
		return;
	}

	QVariantMap data;
	data.insert("package_type_name", package_type_name);
	data.insert("package_category", package_category);
	data.insert("pinyin_code", pinyin_code);
	data.insert("pack_type_id", pack_type_id);
	data.insert("department_id", department_id);
	data.insert("sterilize_type", sterilize_type);
	if (_isModfy)
	{
		data.insert("package_type_id", _package_type_id);
		post(url(PATH_PKGTPYE_MODIFY), data, [this](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("更新包记录失败: ").append(resp.errorString()));
				return;
			}

			return QDialog::accept();
		});
	}
	else
	{
		post(url(PATH_PKGTPYE_ADD), data, [this](QNetworkReply *reply) {
			JsonHttpResponse resp(reply);
			if (!resp.success()) {
				XNotifier::warn(QString("添加包记录失败: ").append(resp.errorString()));
				return;
			}
			return QDialog::accept();

		});
	}
	
}

void AddPackageDialog::setInfo(const QString& pkg_type_id) {
	_package_type_id = pkg_type_id.toInt();
	_isModfy = true;
	setWindowTitle("修改包信息");
	QTimer::singleShot(1000, this, &AddPackageDialog::initPackageInfo);
}

void AddPackageDialog::initPackageInfo()
{
	QVariantMap data;
	data.insert("package_type_id", _package_type_id);

	post(url(PATH_PKGTPYE_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("获取包信息失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> pkgs = resp.getAsList("package_types");
		if (pkgs.count() != 1)
		{
			XNotifier::warn(QString("获取包信息失败。"));
			return;
		}

		for (int i = 0; i != pkgs.count(); ++i) {
			QVariantMap map = pkgs[i].toMap();

			_package_type_id = map["package_type_id"].toInt();

			QString package_name = map["package_name"].toString();
			_pkgNameEdit->setText(package_name);
			_pkgNameEdit->setReadOnly(true);

			QString pinyin_code = map["pinyin_code"].toString();
			_pkgPYCodeEdit->setText(pinyin_code);

			int package_category = map["package_category"].toInt();
			_pkgtypeBox->setCurrentIndex(_pkgtypeBox->findData(package_category));

			QString pack_type = map["pack_type"].toString();
			_picktypeBox->setCurrentText(pack_type);

			int sterilize_type = map["sterilize_type"].toInt();
			_stertypeBox->setCurrentIndex(_stertypeBox->findData(sterilize_type));

			int dep_id = map["department_id"].toInt();
			QString dep_name = map["department_name"].toString();
			_deptEdit->setCurrentIdPicked(dep_id, dep_name);
		}
	});
}

void AddPackageDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "器械名称");
	_model->setHeaderData(1, Qt::Horizontal, "数量");
	_view->setModel(_model);
	_view->setMinimumHeight(500);

	_view->setItemDelegate(new SpinBoxDelegate(
		1, Constant::maxPackageCount, Constant::minPackageCount, _view));

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	for (int i = 0; i != _orders.count(); ++i) {
		QVariantMap map = _orders[i].toMap();
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(map["instrument_name"].toString());
		insItem->setData(map["instrument_id"].toInt());
		items << insItem << new QStandardItem(map["instrument_number"].toString());
		_model->appendRow(items);
	}
}

void AddPackageDialog::addEntry() {
	int insId = _insEdit->currentId();

	if (insId == 0) return;

	int existRow = findRow(insId);
	if (-1 == existRow) {
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(_insEdit->currentName());
		insItem->setData(_insEdit->currentId());
		items << insItem << new QStandardItem("1");
		_model->appendRow(items);
	}
	else {
		QStandardItem *countItem = _model->item(existRow, 1);
		int count = countItem->text().toInt();
		if (count < Constant::maxPackageCount) {
			countItem->setText(QString::number(count + 1));
			_view->selectRow(existRow);
		}
	}
	_insEdit->reset();
}

void AddPackageDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

int AddPackageDialog::findRow(int insId) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, insId, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}