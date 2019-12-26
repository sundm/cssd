#include "recallpage.h"
#include "ui/views.h"
#include "barcode.h"
#include "ui/buttons.h"
#include "core/net/url.h"
#include "dialog/addrecalldialog.h"
#include "xnotifier.h"
#include <QtWidgets/QtWidgets>


RecallPage::RecallPage(QWidget *parent)
	: QWidget(parent)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, Reason + 1, _view))
{
	_font.setPointSize(12);

	_model->setHeaderData(Device, Qt::Horizontal, "灭菌器名称");
	_model->setHeaderData(Cycle, Qt::Horizontal, "灭菌锅次");
	_model->setHeaderData(PackageNum, Qt::Horizontal, "涉及包数量");
	_model->setHeaderData(Reason, Qt::Horizontal, "召回原因");
	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 350);
	header->resizeSection(1, 150);
	header->resizeSection(2, 150);
	header->resizeSection(3, 350);

	Ui::IconButton *refreshButton = new Ui::IconButton(":/res/refresh-24.png", "刷新");
	connect(refreshButton, SIGNAL(clicked()), this, SLOT(reflash()));

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "添加");
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *delButton = new Ui::IconButton(":/res/forbidden-24.png", "删除");
	connect(delButton, SIGNAL(clicked()), this, SLOT(delEntry()));

	QHBoxLayout *htlayout = new QHBoxLayout;
	htlayout->addWidget(refreshButton);
	htlayout->addWidget(addButton);
	htlayout->addWidget(delButton);
	htlayout->addStretch(0);

	QLabel *label = new QLabel(QString("系统自动列出的召回信息不可删除。"));
	Ui::PrimaryButton *recallButton = new Ui::PrimaryButton("召回", Ui::BtnSize::Small);
	connect(recallButton, SIGNAL(clicked()), this, SLOT(recall()));

	QHBoxLayout *hmlayout = new QHBoxLayout;
	hmlayout->addWidget(label);
	hmlayout->addStretch();
	hmlayout->addWidget(recallButton);
	

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addLayout(htlayout);
	mainLayout->addWidget(_view);
	mainLayout->addLayout(hmlayout);

	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));

	QTimer::singleShot(200, [this] { reflash(); });
}

RecallPage::~RecallPage()
{
}

void RecallPage::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;

	Barcode bc(code);
	if (bc.type() == Barcode::Commit) {
		recall();
	}

	if (bc.type() == Barcode::Reset) {
		reflash();
	}
}

void RecallPage::reflash()
{
	if (true)
	{
		_view->clear(); // when succeeded

		_model->insertRows(0, 1);

		for (int i = 0; i !=1; ++i) {

			_model->setData(_model->index(i, Device), QString("1号高温灭菌器"));
			_model->setData(_model->index(i, Device), 130004, Qt::UserRole + 1);

			_model->setData(_model->index(i, Cycle), QString("2"));

			_model->setData(_model->index(i, PackageNum), 6);

			_model->setData(_model->index(i, Reason), QString("生物审核失败"));
			_model->setData(_model->index(i, Reason), false, Qt::UserRole + 1);
		}

		for (int i = 0; i < _model->rowCount(); i++)
		{
			for (int j = 0; j < _model->columnCount(); j++)
			{
				_model->item(i, j)->setTextAlignment(Qt::AlignCenter);
				_model->item(i, j)->setFont(_font);
			}
			
		}
	}
	else
	{
		//XNotifier::warn(QString("获取召回列表失败: ").append(resp.msg()));
		return;
	}
}

void RecallPage::addEntry()
{
	AddRecallDialog d(this);
	connect(&d, SIGNAL(addRecall(const RecallInfo&)), this, SLOT(onAddRecall(const RecallInfo&)));
	if (d.exec() == QDialog::Accepted)
	{
		//todo
	}
}

void RecallPage::onAddRecall(const RecallInfo& info)
{
	int row = _model->rowCount();
	_model->insertRows(row, 1);

	_model->setData(_model->index(row, Device), info.deviceName);
	_model->setData(_model->index(row, Device), info.deviceId, Qt::UserRole + 1);

	_model->setData(_model->index(row, Cycle), info.cycle);

	_model->setData(_model->index(row, PackageNum), info.pkgNums);

	_model->setData(_model->index(row, Reason), info.reason);
	_model->setData(_model->index(row, Reason), true, Qt::UserRole + 1);


	for (int j = 0; j < _model->columnCount(); j++)
	{
		_model->item(row, j)->setTextAlignment(Qt::AlignCenter);
		_model->item(row, j)->setFont(_font);
	}

	
}

void RecallPage::delEntry()
{
	QModelIndexList indexes = _view->selectionModel()->selectedRows();
	if (indexes.count() == 0) return;
	int row = indexes[0].row();
	bool isDel = _view->model()->data(_view->model()->index(row, Reason), Qt::UserRole + 1).toBool();
	if (isDel)
	{
		_model->removeRow(row);
	}
	else
	{
		XNotifier::warn(QString("系统自动列出的召回信息不可删除。"));
		return;
	}

}


void RecallPage::recall() 
{
	
}
