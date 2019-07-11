#include "extdialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "core/user.h"
#include "core/net/url.h"
#include "ui/groups.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/buttons.h"
#include "widget/controls/combos.h"
#include "widget/controls/idedit.h"
#include "ui/views.h"
#include "addextdialog.h"
#include <QtWidgets/QtWidgets>
#include <QDateTime>
#include <QTimer>
#include <QSpinBox>

ExtRecvDialog::ExtRecvDialog(QWidget *parent)
	: QDialog(parent)
	, _vendorCombo(new VendorComboBox)
	, _senderEdit(new Ui::FlatEdit)
	, _senderPhoneEdit(new Ui::FlatEdit)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 7, _view))
{
	setWindowTitle("外来器械接收登记");

	initTableView();

	_senderPhoneEdit->setInputValidator(Ui::InputValitor::NumberOnly);
	_senderPhoneEdit->setMaxLength(Constant::maxPhoneNumber);
	

	QString currentDate = QDateTime::currentDateTime().toString("yyyy/MM/dd-HH:mm:ss");
	_receiveTimeLabel = new Ui::NormalLabel(currentDate);

	_receiverEdit = new Ui::NormalLabel(Core::currentUser().name);

	FormGroup * transferGroup = new FormGroup(this);
	transferGroup->addRow("供应商 (*)", _vendorCombo);
	transferGroup->addRow("送货人 (*)", _senderEdit);
	transferGroup->addRow("联系电话 (*)", _senderPhoneEdit);
	transferGroup->addRow("接收人", _receiverEdit);
	transferGroup->addRow("接受时间", _receiveTimeLabel);
	
	Ui::PrimaryButton *commitButton = new Ui::PrimaryButton("提交", Ui::BtnSize::Small);
	connect(commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QHBoxLayout *hlayout = new QHBoxLayout;

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/add.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hlayout->addWidget(addButton);
	connect(addButton, &QPushButton::clicked, this, &ExtRecvDialog::add);

	QToolButton *minusButton = new QToolButton;
	minusButton->setIcon(QIcon(":/res/minus.png"));
	minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hlayout->addWidget(minusButton);
	connect(minusButton, &QPushButton::clicked, this, &ExtRecvDialog::remove);

	hlayout->addStretch();

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(transferGroup);
	layout->addWidget(_view);
	layout->addLayout(hlayout);
	layout->addWidget(commitButton);

	resize(parent ? parent->width() / 2 : 720, sizeHint().height() * 1.5);

	QTimer::singleShot(0, this, &ExtRecvDialog::loadData);
}

void ExtRecvDialog::accept() {
	int vendorId = _vendorCombo->currentData().toInt();
	QString sender = _senderEdit->text();
	QString senderPhone = _senderPhoneEdit->text();
	int receiverId = Core::currentUser().id;
	QString receiver = Core::currentUser().name;

	if (0 == vendorId || sender.isEmpty() || senderPhone.isEmpty()) {
		XNotifier::warn(QString("暂时无法提交登记: 缺少必要字段"));
		return;
	}

	if (0 == _model->rowCount())
	{
		XNotifier::warn(QString("暂时无法提交登记: 未添加任何器械"));
		return;
	}

	QVariantList ext_info_list;
	for (int i = 0; i != _model->rowCount(); i++) {
		QVariantMap ext_info_map;
		ext_info_map.insert("pkg_type_id", _model->item(i, 0)->data().toInt());
		ext_info_map.insert("num", _model->item(i, 1)->text().toInt());
		ext_info_map.insert("doctor_name", _model->item(i, 2)->text());
		ext_info_map.insert("patient_name", _model->item(i, 3)->text());

		if (!_model->item(i, 4)->text().isEmpty())
			ext_info_map.insert("admission", _model->item(i, 4)->text());
		if (!_model->item(i, 5)->text().isEmpty())
			ext_info_map.insert("age", _model->item(i, 5)->text().toInt());
		if (!_model->item(i, 6)->text().isEmpty())
			ext_info_map.insert("gender", _model->item(i, 6)->data().toString());
		ext_info_list << ext_info_map;
	}

	QVariantMap data;
	data.insert("sup_id", vendorId);
	data.insert("send_name", sender);
	data.insert("send_phone", senderPhone);
	data.insert("p_operator_id", receiverId);
	data.insert("p_operator_name", receiver);
	data.insert("ext_info", ext_info_list);

	Url::post(Url::PATH_EXT_ADD, data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("暂时无法提交登记: ").append(resp.errorString()));
			return;
		}
		QDialog::accept();
	});
}

void ExtRecvDialog::add()
{
	AddExtDialog d(this, _model);
	if (QDialog::Accepted == d.exec()) {
		
	}
}

void ExtRecvDialog::remove()
{
	QModelIndexList selectRows = _theSelectionModel->selectedRows();
	for each (QModelIndex index in selectRows)
	{
		_model->removeRow(index.row());
	}
}

void ExtRecvDialog::slotRowDoubleClicked(const QModelIndex &index)
{
	AddExtDialog d(this, _model, index.row());
	if (QDialog::Accepted == d.exec()) {

	}
}

void ExtRecvDialog::initTableView()
{
	_model->setHeaderData(0, Qt::Horizontal, "器械名称");
	_model->setHeaderData(1, Qt::Horizontal, "数量");
	_model->setHeaderData(2, Qt::Horizontal, "医生姓名");
	_model->setHeaderData(3, Qt::Horizontal, "病人姓名");
	_model->setHeaderData(4, Qt::Horizontal, "住院号");
	_model->setHeaderData(5, Qt::Horizontal, "年龄");
	_model->setHeaderData(6, Qt::Horizontal, "性别");

	_theSelectionModel = new QItemSelectionModel(_model);
	_view->setModel(_model);
	_view->setSelectionModel(_theSelectionModel);

	_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), 
		this, SLOT(slotRowDoubleClicked(const QModelIndex &)));
}


void ExtRecvDialog::loadData() {
	//_pkgEdit->loadForCategary("2");
	_vendorCombo->updateEntry();
}
