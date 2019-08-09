#include "reprintdialog.h"
#include "xnotifier.h"
#include "core/application.h"
#include "core/net/url.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include "util/printermanager.h"
#include <printer/labelprinter.h>
#include "ui/composite/waitingspinner.h"
#include <QVBoxLayout>
#include <QStandardItemModel>

RePrintDialog::RePrintDialog(QWidget *parent)
	: QDialog(parent)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0,9,_view))
	, _waiter(new WaitingSpinner(this))
{
	_model->setHeaderData(0, Qt::Horizontal, "包ID");
	_model->setHeaderData(1, Qt::Horizontal, "包名");
	_model->setHeaderData(2, Qt::Horizontal, "包来源");
	_model->setHeaderData(3, Qt::Horizontal, "打包类型");
	_model->setHeaderData(4, Qt::Horizontal, "数量");
	_model->setHeaderData(5, Qt::Horizontal, "灭菌日期");
	_model->setHeaderData(6, Qt::Horizontal, "失效日期");
	_model->setHeaderData(7, Qt::Horizontal, "打包员");
	_model->setHeaderData(8, Qt::Horizontal, "审核员");

	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//_view->setSelectionMode(QAbstractItemView::SingleSelection);

	Ui::PrimaryButton *okButton = new Ui::PrimaryButton("重打选中的标签");
	connect(okButton, &QPushButton::clicked, this, &RePrintDialog::accept);

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->addWidget(_view);
	vlayout->addWidget(okButton);

	resize(parent->width()/2, sizeHint().height());
	
	loadSource();
}

RePrintDialog::~RePrintDialog() {
}

void RePrintDialog::accept() {
	LabelPrinter *printer = PrinterManager::currentPrinter();
	if (0 != printer->open(LABEL_PRINTER)) {
		XNotifier::warn("打印机未就绪");
		return;
	}

	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();

	if (0 == indexes.size())
	{
		XNotifier::warn("请先选择需要重打的标签信息");
		return;
	}

	PackageLabel plabel;
	ClinicLabel clabel;
	for each (QModelIndex index in indexes)
	{
		if (1 == _model->item(index.row(), 0)->data().toInt())
		{
			clabel.packageId = _model->item(index.row(), 0)->text();
			clabel.packageName = _model->item(index.row(), 1)->text();
			clabel.packageFrom = _model->item(index.row(), 2)->text();
			clabel.disinDate = _model->item(index.row(), 5)->text();
			clabel.expiryDate = _model->item(index.row(), 6)->text();
			clabel.operatorName = _model->item(index.row(), 7)->text();
			clabel.assessorName = _model->item(index.row(), 8)->text();
			printer->printClinicLabel(clabel);
		}
		else
		{
			plabel.packageId = _model->item(index.row(), 0)->text();
			plabel.packageName = _model->item(index.row(), 1)->text();
			plabel.packageFrom = _model->item(index.row(), 2)->text();
			plabel.packageType = _model->item(index.row(), 3)->text();
			plabel.count = _model->item(index.row(), 4)->text().toInt();
			plabel.disinDate = _model->item(index.row(), 5)->text();
			plabel.expiryDate = _model->item(index.row(), 6)->text();
			plabel.operatorName = _model->item(index.row(), 7)->text();
			plabel.assessorName = _model->item(index.row(), 8)->text();
			printer->printPackageLabel(plabel);
		}
		
	}
	
	printer->close();
	QDialog::accept();
}

void RePrintDialog::loadSource() {
	_view->clear();

	QByteArray data("{}");

	post(url(PATH_PACK_REPRINT), data, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("查询失败: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("pack_info");
		
		for (int i = 0; i != orders.count(); ++i) {
			QVariantMap map = orders[i].toMap();

			QList<QStandardItem *> rowItems;

			QStandardItem *packageId = new QStandardItem();
			packageId->setTextAlignment(Qt::AlignCenter);
			packageId->setText(map["package_id"].toString());
			packageId->setData(map["package_category"].toString());
			rowItems.append(packageId);

			QStandardItem *packageName = new QStandardItem();
			packageName->setTextAlignment(Qt::AlignCenter);
			packageName->setText(map["package_name"].toString());
			rowItems.append(packageName);

			QStandardItem *packageFrom = new QStandardItem();
			packageFrom->setTextAlignment(Qt::AlignCenter);
			packageFrom->setText(map["department_name"].toString());
			rowItems.append(packageFrom);

			QStandardItem *packageType = new QStandardItem();
			packageType->setTextAlignment(Qt::AlignCenter);
			packageType->setText(map["pack_type_name"].toString());
			rowItems.append(packageType);

			QStandardItem *packageNum = new QStandardItem();
			packageNum->setTextAlignment(Qt::AlignCenter);
			packageNum->setText(QString::number(map["instrument_num"].toInt()));
			rowItems.append(packageNum);

			QStandardItem *disinDate = new QStandardItem();
			disinDate->setTextAlignment(Qt::AlignCenter);
			disinDate->setText(map["sterilize_date"].toString());
			rowItems.append(disinDate);

			QStandardItem *expiryDate = new QStandardItem();
			expiryDate->setTextAlignment(Qt::AlignCenter);
			expiryDate->setText(map["expire_date"].toString());
			rowItems.append(expiryDate);

			QStandardItem *operatorName = new QStandardItem();
			operatorName->setTextAlignment(Qt::AlignCenter);
			operatorName->setText(map["operator_name"].toString());
			rowItems.append(operatorName);

			QStandardItem *assessorName = new QStandardItem();
			assessorName->setTextAlignment(Qt::AlignCenter);
			assessorName->setText(map["check_operator_name"].toString());
			rowItems.append(assessorName);

			_model->appendRow(rowItems);
		}
	});
}
