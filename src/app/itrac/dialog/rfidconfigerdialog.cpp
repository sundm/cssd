#include "rfidconfigerdialog.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/labels.h"
#include "ui/inputfields.h"
#include "ui/ui_commons.h"
#include "ui/groups.h"
#include "xnotifier.h"
#include "ui/views.h"
#include "rfidreaderconfigerdialog.h"
#include <QtWidgets/QtWidgets>

ConfigRfidDialog::ConfigRfidDialog(QWidget *parent)
	: QDialog(parent)
	, _comBox(new QComboBox(this))
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 4, _view))
{
	setWindowTitle("配置RFID扫描器");

	QPushButton *submitButton = new QPushButton("确定");
	submitButton->setIcon(QIcon(":/res/check-24.png"));
	submitButton->setDefault(true);
	connect(submitButton, &QPushButton::clicked, this, &ConfigRfidDialog::accept);

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_comBox);

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

	FormGroup * viewGroup = new FormGroup(this);
	QWidget *w = new QWidget(viewGroup);
	w->setLayout(hLayout);
	viewGroup->addRow("添加RFID扫描枪", w);
	viewGroup->addRow("RFID扫描枪列表", _view);

	QGridLayout *mainLayout = new QGridLayout(this);
	mainLayout->setVerticalSpacing(15);
	mainLayout->addWidget(viewGroup, 0, 0);
	mainLayout->addWidget(Ui::createSeperator(Qt::Horizontal), 1, 0, 1, 2);
	mainLayout->addWidget(submitButton, 2, 0, 1, 2, Qt::AlignHCenter);

	setFixedHeight(sizeHint().height());
	resize(800, height());

	initView();
}

void ConfigRfidDialog::accept() {
	/*REMEMBER_READER = _rememberMeBox->isChecked();
	QSettings *configIni = new QSettings("prepareSettings.ini", QSettings::IniFormat);
	configIni->setValue("port/remember", REMEMBER_READER);
	configIni->setValue("port/name", LAST_COM);*/
	return QDialog::accept();
}

void ConfigRfidDialog::addEntry() {
	ConfigRfidReaderDialog d(this);
	d.exec();
	/*int insId = _insEdit->currentId();

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
	_insEdit->reset();*/
}

void ConfigRfidDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

void ConfigRfidDialog::initView()
{
	_model->setHeaderData(0, Qt::Horizontal, "名称");
	_model->setHeaderData(1, Qt::Horizontal, "类型");
	_model->setHeaderData(2, Qt::Horizontal, "参数");
	_model->setHeaderData(3, Qt::Horizontal, "状态");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setSelectionBehavior(QAbstractItemView::SelectRows);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	QStringList readerTypeList;
	readerTypeList.append("TSL-1128");
	_comBox->addItems(readerTypeList);


	for each (TSL1128Reader* reader in TSL1128Readers)
	{
		QList<QStandardItem *> items;
		QStandardItem *nameItem = new QStandardItem(QString::fromStdString(reader->getName()));
		QStandardItem *typeItem = new QStandardItem("TSL-1128");
		typeItem->setData(1);
		QStandardItem *portItem = new QStandardItem(QString::fromStdString(reader->getPort()));
		QStandardItem *stateItem = new QStandardItem(reader->isConnected() ? "已连接" : "已断开");
		stateItem->setData(reader->isConnected());
		items << nameItem << typeItem << portItem << stateItem;
		_model->appendRow(items);
	}
}