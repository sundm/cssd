#include "abnormaldialog.h"
#include "xnotifier.h"
#include "core/barcode.h"
#include "core/application.h"
#include "core/net/url.h"
#include "core/user.h"
#include "ui/buttons.h"
#include "ui/views.h"
#include <printer/labelprinter.h>
#include "regexpinputdialog.h"
#include "ui/composite/waitingspinner.h"
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QtWidgets/QtWidgets>

Result::Result() {
	wet = 1;
	inside = 1;
}

QString Result::toString(int verdict) {
	switch (verdict)
	{
	case 1: return "1";
	case 0: return "0";
	default: return QString();
	}
}

CheckItem::CheckItem(const QString &title, QWidget *parent /*= nullptr*/)
	:QGroupBox(parent)
{
	QVBoxLayout  *layout = new QVBoxLayout(this);
	QLabel *titleLabel = new QLabel(title);
	titleLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
	layout->addWidget(titleLabel);
	reset();
}

void CheckItem::reset() {

	QLayout *layout = this->layout();
	QLayoutItem *child;
	while ((child = layout->takeAt(1)) != 0) {
		delete child;
	}

	QRadioButton * qualifiedButton = new QRadioButton("合格");
	QRadioButton * unQualifiedButton = new QRadioButton("不合格");
	connect(qualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = 1; });
	connect(unQualifiedButton, &QRadioButton::clicked, this, [this] {_verdict = 0; });
	layout->addWidget(qualifiedButton);
	layout->addWidget(unQualifiedButton);

	qualifiedButton->setChecked(true);
	_verdict = 1;

	setStyleSheet("background-color:#eeeeee;");
}

int CheckItem::verdict() const {
	return _verdict;
}

SterileCheckGroup::SterileCheckGroup(QWidget *parent /*= nullptr*/)
{
	QGridLayout *layout = new QGridLayout(this);

	_wetItem = new CheckItem("湿包监测审核");
	layout->addWidget(_wetItem, 0, 0);

	_insideItem = new CheckItem("包内卡监测审核");
	layout->addWidget(_insideItem, 0, 1);
}

Result SterileCheckGroup::verdicts() const
{
	Result result;

	result.wet = _wetItem->verdict();
	result.inside = _insideItem->verdict();
	return result;
}

void SterileCheckGroup::reset() {
	_wetItem->reset();
	_insideItem->reset();
}


AbnormalDialog::AbnormalDialog(QWidget *parent)
	: QDialog(parent)
	, _view(new TableView(this))
	, _checkGroup(new SterileCheckGroup)
	, _model(new QStandardItemModel(0,5,_view))
	, _waiter(new WaitingSpinner(this))
{
	_model->setHeaderData(0, Qt::Horizontal, "包条码");
	_model->setHeaderData(1, Qt::Horizontal, "包名");
	_model->setHeaderData(2, Qt::Horizontal, "包装类型");
	_model->setHeaderData(3, Qt::Horizontal, "目标科室");
	_model->setHeaderData(4, Qt::Horizontal, "失效日期");

	_view->setModel(_model);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//_view->setSelectionMode(QAbstractItemView::SingleSelection);

	Ui::PrimaryButton *okButton = new Ui::PrimaryButton("提交异常");
	connect(okButton, &QPushButton::clicked, this, &AbnormalDialog::accept);

	QHBoxLayout *hLayout = new QHBoxLayout;

	Ui::IconButton *addButton = new Ui::IconButton(":/res/plus-24.png", "手工添加");
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	Ui::IconButton *minusButton = new Ui::IconButton(":/res/delete-24.png", "删除选中");
	hLayout->addWidget(minusButton);
	connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	hLayout->addStretch();

	QVBoxLayout *vlayout = new QVBoxLayout(this);
	vlayout->addLayout(hLayout);
	vlayout->addWidget(_view);
	vlayout->addWidget(_checkGroup);
	vlayout->addWidget(okButton);

	resize(parent->width()/2, sizeHint().height());
}

AbnormalDialog::~AbnormalDialog() {
}

void AbnormalDialog::addEntry() {
	bool ok;
	QRegExp regExp("\\d{8,}");
	QString code = RegExpInputDialog::getText(this, "手工输入条码", "请输入包条码", "", regExp, &ok);
	if (ok) {
		handleBarcode(code);
	}
}

void AbnormalDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_view->model()->removeRow(indexes.at(i - 1).row());
}

void AbnormalDialog::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::Package) {
		loadSource(code);
	}
}

void AbnormalDialog::accept() {
	

	Result verdicts = _checkGroup->verdicts();
	int opId = Core::currentUser().id;
	if (0 == opId) return;

	QVariantMap vmap;
	vmap.insert("operator_id", opId);
	vmap.insert("inside_result", verdicts.toString(verdicts.wet));
	vmap.insert("inside_fail", verdicts.toString(verdicts.inside));

	QVariantList rows;

	for (int i = 0; i < _model->rowCount(); i++)
	{
		rows << _model->item(i, 0)->text();
	}

	vmap.insert("package_ids", rows);

	post(url(PATH_PKG_PUSH), vmap, [this](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("异常登记失败: ").append(resp.errorString()));
			return;
		}
		else {
			XNotifier::warn("异常登记成功!");
			return QDialog::accept();
		}
	});

}

void AbnormalDialog::loadSource(const QString &id) {
	
	QString data = QString("{\"package_id\":\"%1\"}").arg(id);
	post(url(PATH_PKG_INFO), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包信息: ").append(resp.errorString()));
			return;
		}

		QString state = resp.getAsString("state");
		if (state == "X") {
			XNotifier::warn("该包已被召回");
			return;
		}
		if (state != "I") {
			XNotifier::warn("该包处于不可使用状态，请检查包的来源");
			return;
		}

		QList<QStandardItem *> rowItems;
		rowItems << new QStandardItem(id);
		rowItems << new QStandardItem(resp.getAsString("package_type_name"));
		rowItems << new QStandardItem(resp.getAsString("pack_type_name"));
		rowItems << new QStandardItem(resp.getAsString("from_department_name"));
		rowItems << new QStandardItem(resp.getAsString("valid_date"));
		_model->appendRow(rowItems);
	});
}
