#include "addcostingdiaalog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/inliner.h"
#include "core/net/url.h"
#include "core/assets.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/views.h"
#include <QtWidgets/QtWidgets>

Addcostingdiaalog::Addcostingdiaalog(QWidget *parent)
	: QDialog(parent)
	, _nameEdit(new Ui::FlatEdit)
	, _typeCombo(new QComboBox)
	, _view(new TableView)
	, _model(new QStandardItemModel(0, 3, _view))
{
	setWindowTitle("添加新设备");

	_typeCombo->addItem("清洗机", WASH_DEVICE);
	_typeCombo->addItem("灭菌器", STERILE_DEVICE);
	_typeCombo->setCurrentIndex(-1);
	connect(_typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
		this, &Addcostingdiaalog::loadPrograms);

	initProgramView();

	QPushButton *submitButton = new QPushButton("提交");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &Addcostingdiaalog::accept);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(new QLabel("设备类型"), 0, 0);
	mainLayout->addWidget(new QLabel("设备名称"), 1, 0);
	mainLayout->addWidget(new QLabel("预设程序"), 2, 0);
	mainLayout->addWidget(_typeCombo, 0, 1);
	mainLayout->addWidget(_nameEdit, 1, 1);
	mainLayout->addWidget(_view, 2, 1);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 3, 0, 1, 2);
	mainLayout->addWidget(submitButton, 4, 0, 1, 2, Qt::AlignHCenter);

	resize(parent ? parent->width() / 3 : 360, sizeHint().height());
}

Addcostingdiaalog::~Addcostingdiaalog()
{
}

void Addcostingdiaalog::accept() {
	if (_typeCombo->currentIndex() == -1) {
		_typeCombo->showPopup();
		return;
	}

	QString name = _nameEdit->text();
	if (name.isEmpty()) {
		_nameEdit->setFocus();
		return;
	}

	QVariantList programs;
	for (int i = 0; i != _model->rowCount(); i++) {
		QStandardItem *item = _model->item(i);
		if (item->checkState() == Qt::Checked) programs.append(item->text());
	}
	QVariantMap vmap;
	vmap.insert("device_type", _typeCombo->currentData());
	vmap.insert("device_name", name);
	vmap.insert("production_date", QDate::currentDate());
	vmap.insert("support_program_ids", programs);

	Core::app()->startWaitingOn(this);
	Url::post(Url::PATH_DEVICE_ADD, vmap, [this](QNetworkReply *reply) {
		Core::app()->stopWaiting();
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			//return; // TODO
		}
		else {
			QDialog::accept();
		}
	});
}

void Addcostingdiaalog::initProgramView() {
	_model->setHeaderData(0, Qt::Horizontal, "编号");
	_model->setHeaderData(1, Qt::Horizontal, "名称");
	_model->setHeaderData(2, Qt::Horizontal, "描述");
	_view->setModel(_model);

	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 200);

	connect(_view, SIGNAL(clicked(const QModelIndex &)), this, SLOT(toggleCheckSate(const QModelIndex &)));
}

void Addcostingdiaalog::loadPrograms(int index) {
	_model->removeRows(0, _model->rowCount());

	QString deviceType = _typeCombo->itemData(index).toString();
	QByteArray data;
	data.append("{\"program_type\":\"").append(deviceType).append("\"}");

	Url::post(Url::PATH_PROGRAM_SEARCH, data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for (auto &program : programs) {
			QVariantMap map = program.toMap();
			QList<QStandardItem *> rowItems;
			QStandardItem *checkItem = new QStandardItem(map["device_program_id"].toString());
			checkItem->setCheckable(true);
			rowItems.append(checkItem);
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["program_desc"].toString()));
			_model->appendRow(rowItems);
		}
	});
}
