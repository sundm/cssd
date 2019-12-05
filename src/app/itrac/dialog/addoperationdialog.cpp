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
#include "model/itemdelegate.h"
#include "rdao/dao/surgerydao.h"
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
	, _patientIdEdit(new Ui::FlatEdit)
	, _patientNameEdit(new Ui::FlatEdit)
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
	_operTimeEdit->setDateTime(QDateTime::currentDateTime());

	QRegExp regx("^\\d{8}$");
	QValidator *validator = new QRegExpValidator(regx);
	_operIdEdit->setValidator(validator);
	_patientIdEdit->setValidator(validator);

	_operIdEdit->setText("12345678");
	_operRoomEdit->setText("1#手术间");
	_operNameEdit->setText("测试手术");
	_patientIdEdit->setText("12345678");
	_patientNameEdit->setText("张三");

	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("手术ID (*)", _operIdEdit);
	pkgGroup->addRow("手术室 (*)", _operRoomEdit);
	pkgGroup->addRow("手术时间 (*)", _operTimeEdit);
	pkgGroup->addRow("手术名称 (*)", _operNameEdit);
	pkgGroup->addRow("病人ID号 (*)", _patientIdEdit);
	pkgGroup->addRow("病人姓名 (*)", _patientNameEdit);

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
	Surgery surgery;

	QString operId = _operIdEdit->text();
	if (operId.isEmpty()) {
		_operIdEdit->setFocus();
		return;
	}
	surgery.id = operId.toInt();
		
	QString operRoom = _operRoomEdit->text();
	if (operRoom.isEmpty()) {
		_operRoomEdit->setFocus();
		return;
	}
	surgery.surgeryRoom = operRoom;

	QDateTime dateTime = _operTimeEdit->dateTime();
	QString operName = dateTime.toString("yyyy-MM-dd HH:mm:ss");
	surgery.surgeryTime = dateTime;

	QString surgeryName = _operNameEdit->text();
	if (surgeryName.isEmpty()) {
		_operNameEdit->setFocus();
		return;
	}
	surgery.surgeryName = surgeryName;

	if (_patientIdEdit->text().isEmpty()) {
		_patientIdEdit->setFocus();
		return;
	}
	int patientId = _patientIdEdit->text().toInt();
	surgery.patientId = patientId;

	if (_patientNameEdit->text().isEmpty()) {
		_patientNameEdit->setFocus();
		return;
	}
	QString patientName = _patientNameEdit->text();
	surgery.patientName = patientName;
	
	for (int i = 0; i < _model->rowCount(); i++)
	{
		Surgery::DetailItem item;

		item.pkgTypeId = _model->data(_model->index(i, 0), Qt::UserRole + 1).toInt();
		item.pkgTypeName = _model->data(_model->index(i, 0), Qt::DisplayRole).toString();
		item.pkgNum = _model->data(_model->index(i, 1), Qt::DisplayRole).toInt();

		surgery.detail.append(item);
	}

	SurgeryDao dao;
	
	result_t resp = dao.addSurgery(surgery);

	if (resp.isOk())
	{
		return QDialog::accept();
	}
	else
	{
		XNotifier::warn(QString("添加手术登记失败: ").append(resp.msg()));
	}

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