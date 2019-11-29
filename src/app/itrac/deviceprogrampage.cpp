#include "deviceprogrampage.h"
#include "inliner.h"
#include "xnotifier.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "dialog/programdialog.h"
#include "rdao/dao/devicedao.h"
#include <QtWidgets/QtWidgets>

DeviceProgramPage::DeviceProgramPage(QWidget *parent)
	: QWidget(parent)
	, _filterComboBox(new QComboBox(this))
	, _view(new TableView(this))
	, _programModel(new QStandardItemModel(0, 4, _view))
{
	initProgramView();

	_filterComboBox->addItem("全部", "");
	_filterComboBox->addItem("仅清洗程序", WASH_DEVICE);
	_filterComboBox->addItem("仅灭菌程序", STERILE_DEVICE);
	_filterComboBox->setCurrentIndex(0);
	connect(_filterComboBox, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
		this, &DeviceProgramPage::onFilterChanged);

	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, &QToolButton::clicked, this, &DeviceProgramPage::refresh);

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, &QToolButton::clicked, this, &DeviceProgramPage::add);

	Ui::IconButton *modifyButton = new Ui::IconButton(":/res/write-24.png", "修改");
	connect(modifyButton, &QToolButton::clicked, this, &DeviceProgramPage::modify);

	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->addWidget(_filterComboBox);
	hlayout->addWidget(refreshButton);
	hlayout->addWidget(addButton);
	//hlayout->addWidget(modifyButton);
	hlayout->addStretch();

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(hlayout);
	mainLayout->addWidget(_view);

	updateProgramView();
}

void DeviceProgramPage::onFilterChanged(int index)
{
	QString deviceType = _filterComboBox->itemData(index).toString();
	updateProgramView(deviceType);
}

void DeviceProgramPage::showProgramItemContextMenu(const QPoint&) {

}

void DeviceProgramPage::refresh() {
	QString programType = _filterComboBox->currentData().toString();
	updateProgramView(programType);
}

void DeviceProgramPage::add() {
	AddProgramDialog d(this);
	if (QDialog::Accepted == d.exec()) {
		updateProgramView();
		_filterComboBox->setCurrentIndex(0);
	}
}

void DeviceProgramPage::modify() {

}

void DeviceProgramPage::initProgramView()
{
	_programModel->setHeaderData(0, Qt::Horizontal, "编号");
	_programModel->setHeaderData(1, Qt::Horizontal, "类型");
	_programModel->setHeaderData(2, Qt::Horizontal, "名称");
	_programModel->setHeaderData(3, Qt::Horizontal, "描述");
	_view->setModel(_programModel);

	_view->setSelectionBehavior(QAbstractItemView::SelectRows);
	_view->setSelectionMode(QAbstractItemView::SingleSelection);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_view, &QWidget::customContextMenuRequested,
		this, &DeviceProgramPage::showProgramItemContextMenu);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 260);
	header->resizeSection(1, 260);
	header->resizeSection(2, 360);
}

void DeviceProgramPage::updateProgramView(const QString& programType /*= QString()*/)
{
	_programModel->removeRows(0, _programModel->rowCount());
	DeviceDao dao;
	QList<Program> programs;
	result_t resp = dao.getAllPrograms(&programs);
	if (resp.isOk())
	{
		for (auto &program : programs) {
			int typeValue = program.category;
			QStandardItem *typeItem = new QStandardItem(literal_program_type(typeValue));
			typeItem->setData(typeValue);

			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(QString::number(program.id)));
			rowItems.append(typeItem);
			rowItems.append(new QStandardItem(program.name));
			rowItems.append(new QStandardItem(program.remark));
			_programModel->appendRow(rowItems);
		}
	}
	else
	{
		XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.msg()));
		return;
	}
	/*
	QByteArray data;
	if (programType.isEmpty())
		data.append("{}");
	else
		data.append("{\"program_type\":\"").append(programType).append("\"}");

	post(url(PATH_PROGRAM_SEARCH), data, [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取设备程序列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> programs = resp.getAsList("programs");
		for(auto &program: programs) {
			QVariantMap map = program.toMap();
			QString typeValue = map["program_type"].toString();
			QStandardItem *typeItem = new QStandardItem(literal_program_type(typeValue));
			typeItem->setData(typeValue);

			QList<QStandardItem *> rowItems;
			rowItems.append(new QStandardItem(map["device_program_id"].toString()));
			rowItems.append(typeItem);
			rowItems.append(new QStandardItem(map["program_name"].toString()));
			rowItems.append(new QStandardItem(map["program_desc"].toString()));
			_programModel->appendRow(rowItems);
		}
	});
	*/
}
