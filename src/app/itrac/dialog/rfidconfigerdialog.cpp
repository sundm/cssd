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
#include "desktopreaderconfig.h"
#include "../libs/rfidreader/desktopreader.h"
#include <QtWidgets/QtWidgets>
#include <QFile>
#include <QDomDocument>

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
	resize(700, height());

	connect(_view, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(slotRowDoubleClicked(const QModelIndex &)));

	initView();
}

void ConfigRfidDialog::accept() {
	QString xmlFileName = "prepareSetting.xml";
	QFile file(xmlFileName);
	if (!file.open(QFile::ReadOnly | QFile::Text))
	{
		return;
	}

	QDomDocument document;
	QString error;
	int row = 0, column = 0;
	if (!document.setContent(&file, false, &error, &row, &column))
	{
		return;
	}

	file.close();

	if (document.isNull())
	{
		return;
	}

	QDomElement root = document.documentElement();

	QString root_tag_name = root.tagName();
	if (root_tag_name.compare("cssd") == 0)
	{
		QDomNodeList readerNodes = document.elementsByTagName("reader");
		int count = readerNodes.count();
		for (int i = 0; i < count; i++) {
			QDomNode readerNode = readerNodes.at(0);
			root.removeChild(readerNode);
		}

		for each (TSL1128Reader* reader in TSL1128Readers)
		{
			QDomElement element = document.createElement("reader");
			element.setAttribute("type", "1");
			element.setAttribute("name", QString::fromStdString(reader->getName()));
			element.setAttribute("port", QString::fromStdString(reader->getPort()));

			root.appendChild(element);
		}


		if (!DesktopReader::getInstance()->getAddress().empty())
		{
			QDomElement element = document.createElement("reader");
			element.setAttribute("type", "2");
			element.setAttribute("name", QString::fromStdString(DesktopReader::getInstance()->getAddress()));
			element.setAttribute("port", QString::number(DesktopReader::getInstance()->getAntenna()));

			root.appendChild(element);
		}		
	
		if (!file.open(QFile::WriteOnly | QFile::Text))
		{
			return;
		}

		QTextStream out(&file);
		document.save(out, 4);
		file.close();
	}

	
	return QDialog::accept();
}

void ConfigRfidDialog::addEntry() {
	int type = _comBox->currentData().toInt();
	if (type == 1)
	{
		ConfigRfidReaderDialog d(this);
		if (d.exec() == QDialog::Accepted)
		{
			loadReaders();
		}
	}
	else if(type == 2)
	{
		for (int i = 0; i < _model->rowCount(); i++)
		{
			int type = _model->data(_model->index(i, 1), 257).toInt();
			if (type == 2)
			{
				XNotifier::warn(QString("已存在该型号读卡器，无法添加。"), -1, this);
				return;
			}
		}

		DesktopReaderConfigDialog d("192.168.1.100", 4, this);
		if (d.exec() == QDialog::Accepted)
		{
			loadReaders();
		}
	}
	
}

void ConfigRfidDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	if (indexes.size() > 0)
	{
		int row = indexes.at(0).row();
		int type = _model->data(_model->index(row, 1), 257).toInt();
		if (type == 1)
		{
			TSL1128Readers.removeAt(row);
		}
		else if (type == 2)
		{
			DESKTOP_ADDRESS.clear();
			DesktopReader::getInstance()->disconnect();
			DesktopReader::getInstance()->setAddress("");
			DesktopReader::getInstance()->setAntenna(0);
		}
		
		_model->removeRow(row);
	}
	
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
	_view->setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 150);

	_comBox->addItem("tsl-1128", 1);
	_comBox->addItem("桌面检查台", 2);

	loadReaders();
}

void ConfigRfidDialog::loadReaders()
{
	_model->removeRows(0, _model->rowCount());

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

	if (!DESKTOP_ADDRESS.isEmpty() && DesktopReader::getInstance()->getAddress().empty())
	{
		QList<QStandardItem *> items;
		QStandardItem *nameItem = new QStandardItem(DESKTOP_ADDRESS);
		QStandardItem *typeItem = new QStandardItem("桌面检查台");
		typeItem->setData(2);
		QStandardItem *portItem = new QStandardItem(QString::number(DESKTOP_ANTENNA).append("天线"));
		QStandardItem *stateItem = new QStandardItem(DesktopReader::getInstance()->isConnected() ? "已连接" : "已断开");
		stateItem->setData(DesktopReader::getInstance()->isConnected());
		items << nameItem << typeItem << portItem << stateItem;
		_model->appendRow(items);
	}
	else if (!DesktopReader::getInstance()->getAddress().empty())
	{
		QList<QStandardItem *> items;
		QStandardItem *nameItem = new QStandardItem(QString::fromStdString(DesktopReader::getInstance()->getAddress()));
		QStandardItem *typeItem = new QStandardItem("桌面检查台");
		typeItem->setData(2);
		QStandardItem *portItem = new QStandardItem(QString::number(DesktopReader::getInstance()->getAntenna()).append("天线"));
		QStandardItem *stateItem = new QStandardItem(DesktopReader::getInstance()->isConnected() ? "已连接" : "已断开");
		stateItem->setData(DesktopReader::getInstance()->isConnected());
		items << nameItem << typeItem << portItem << stateItem;
		_model->appendRow(items);
	}
}

void ConfigRfidDialog::slotRowDoubleClicked(const QModelIndex &index)
{
	int row = index.row();
	int type = _model->data(_model->index(row, 1), 257).toInt();
	
	if (type == 1)
	{
		ConfigRfidReaderDialog d(this);
		d.setReader(row);
		if (d.exec() == QDialog::Accepted)
		{
			loadReaders();
		}
	}
	
	if (type == 2)
	{
		DesktopReaderConfigDialog d(DESKTOP_ADDRESS, DESKTOP_ANTENNA, this);
		d.exec();
		loadReaders();
	}
}