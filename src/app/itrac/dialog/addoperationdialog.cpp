#include "addoperationdialog.h"
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
#include <QDateTimeEdit>

AddOperatinDialog::AddOperatinDialog(QWidget *parent)
	: QDialog(parent)
	, _operIdEdit(new Ui::FlatEdit)
	, _operRoomEdit(new Ui::FlatEdit)
	, _operTimeEdit(new QDateTimeEdit(QDate::currentDate()))
	, _operNameEdit(new Ui::FlatEdit)
	, _patNameEdit(new Ui::FlatEdit)
	, _packageEdit(new PackageEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
{
	_isModfy = false;

	_model->setHeaderData(0, Qt::Horizontal, "器械包名称");
	_model->setHeaderData(1, Qt::Horizontal, "数量");
	_view->setModel(_model);
	_view->setMinimumHeight(500);

	_view->setItemDelegate(new SpinBoxDelegate(
		1, Constant::maxPackageCount, Constant::minPackageCount, _view));

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	_operTimeEdit->setMinimumDate(QDate::currentDate().addDays(-365));
	_operTimeEdit->setMaximumDate(QDate::currentDate().addDays(365));
	_operTimeEdit->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
	_operTimeEdit->setCalendarPopup(true);

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("手术ID (*)", _operIdEdit);
	pkgGroup->addRow("手术室 (*)", _operRoomEdit);
	pkgGroup->addRow("手术时间 (*)", _operTimeEdit);
	pkgGroup->addRow("手术名称 (*)", _operNameEdit);
	pkgGroup->addRow("病人姓名 (*)", _patNameEdit);
	//pkgGroup->addRow("使用包信息 (*)", _packageEdit);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_packageEdit);

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

	setWindowTitle("添加手术");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addLayout(vlayout);
	layout->addWidget(_commitButton);

	resize(600, 800);

	QTimer::singleShot(500, [this] {_packageEdit->loadForDepartment(0); });

}

void AddOperatinDialog::initData() {
	for (int i = 0; i != _orders.count(); ++i) {
		QVariantMap map = _orders[i].toMap();
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(map["instrument_name"].toString());
		insItem->setData(map["instrument_id"].toInt());
		items << insItem << new QStandardItem(map["instrument_number"].toString());
		_model->appendRow(items);
	}
}

void AddOperatinDialog::accept() {
	QString operId = _operIdEdit->text();
	if (operId.isEmpty()) {
		_operIdEdit->setFocus();
		return;
	}
		
	QString operRoom = _operRoomEdit->text();
	if (operRoom.isEmpty()) {
		_operRoomEdit->setFocus();
		return;
	}
	QDateTime dateTime = _operTimeEdit->dateTime();
	QString operName = dateTime.toString("yyyy-MM-dd HH:mm:ss");

	/*int pack_type_id = _picktypeBox->currentData().toInt();
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
	}*/
	
}

void AddOperatinDialog::setInfo(const QString& operationId) {
	_operation_id = operationId.toInt();
	_isModfy = true;
	setWindowTitle("修改包信息");
	QTimer::singleShot(1000, this, &AddOperatinDialog::initData);
}

void AddOperatinDialog::addEntry() {
	int packageId = _packageEdit->currentId();

	if (packageId == 0) return;

	int existRow = findRow(packageId);
	if (-1 == existRow) {
		QList<QStandardItem *> items;
		QStandardItem *insItem = new QStandardItem(_packageEdit->currentName());
		insItem->setData(_packageEdit->currentId());
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
	_packageEdit->reset();
}

void AddOperatinDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

int AddOperatinDialog::findRow(int packageId) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, packageId, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}