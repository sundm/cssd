#include "addpkgcodedialog.h"
#include "xnotifier.h"
#include "core/constants.h"
#include "barcode.h"
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

AddpkgcodeDialog::AddpkgcodeDialog(QWidget *parent, const QString& pkg_name, const QString& pkg_id)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgCodeEdit(new Ui::FlatEdit)
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 1, _view))
{
	_pkgNameEdit->setText(pkg_name);
	_pkgNameEdit->setReadOnly(true);
	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名:", _pkgNameEdit);
	_package_type_id = pkg_id;

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_pkgCodeEdit);

	QToolButton *addButton = new QToolButton;
	addButton->setIcon(QIcon(":/res/plus-24.png"));
	addButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	hLayout->addWidget(addButton);
	connect(addButton, SIGNAL(clicked()), this, SLOT(addEntry()));

	//QToolButton *minusButton = new QToolButton;
	//minusButton->setIcon(QIcon(":/res/delete-24.png"));
	//minusButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
	//hLayout->addWidget(minusButton);
	//connect(minusButton, SIGNAL(clicked()), this, SLOT(removeEntry()));

	FormGroup * viewGroup = new FormGroup(this);
	QWidget *w = new QWidget(viewGroup);
	w->setLayout(hLayout);
	viewGroup->addRow("", w);
	viewGroup->addRow("",_view);

	setWindowTitle("包内器械卡信息");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addWidget(pkgGroup);
	layout->addWidget(viewGroup);
	layout->addWidget(_commitButton);

	initInstrumentView();

	resize(500, sizeHint().height());

	QTimer::singleShot(0, this, &AddpkgcodeDialog::initData);
}

void AddpkgcodeDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "包内器械卡号");
	//_model->setHeaderData(1, Qt::Horizontal, "数量");
	_view->setModel(_model);
	_view->setMinimumHeight(500);

	_view->setItemDelegate(new SpinBoxDelegate(
		1, Constant::maxPackageCount, Constant::minPackageCount, _view));

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);
}

void AddpkgcodeDialog::handleBarcode(const QString &code) {
	Barcode bc(code);
	if (bc.type() == Barcode::PkgCode) {
		int id = bc.intValue();
	}
}

void AddpkgcodeDialog::initData() {
	QByteArray data("{\"pkg_type_id\":");
	data.append(_package_type_id).append('}');
	post(url(PATH_CARD_SEARCH), QByteArray().append(data), [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取包内器械列表信息: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> orders = resp.getAsList("card_info");

		for (auto &order : orders) {
			QVariantMap map = order.toMap();
			int code = map["card_id"].toInt();

			QList<QStandardItem *> items;
			QStandardItem *insItem = new QStandardItem(QString::number(code));
			insItem->setData(code);
			items << insItem;
			_model->appendRow(items);
		}
	});
}

void AddpkgcodeDialog::addEntry() {
	Barcode bc(_pkgCodeEdit->text());
	if (bc.type() == Barcode::PkgCode) {
		int code = bc.intValue();
		int existRow = findRow(code);
		if (-1 == existRow) {
			QVariantList codes;
			
			QVariantMap code_map;
			code_map.insert("card_id", code);
			code_map.insert("card_name", "");//todo
			codes << code_map;
			

			QVariantMap data;
			data.insert("pkg_type_id", _package_type_id);
			data.insert("cards", codes);

			post(url(PATH_CARD_ADD), data, [this](QNetworkReply *reply) {
				JsonHttpResponse resp(reply);
				if (!resp.success()) {
					XNotifier::warn(QString("添加包内器械表失败: ").append(resp.errorString()));
					return;
				}
			});

			QList<QStandardItem *> items;
			QStandardItem *insItem = new QStandardItem(QString::number(code));
			insItem->setData(code);
			items << insItem;
			_model->appendRow(items);
			
		}
		else {
			XNotifier::warn(QString("输入的编号已存在!"));
			return;
		}
		_pkgCodeEdit->clear();
	}
}

void AddpkgcodeDialog::removeEntry() {
	QItemSelectionModel *selModel = _view->selectionModel();
	QModelIndexList indexes = selModel->selectedRows();
	int countRow = indexes.count();
	for (int i = countRow; i > 0; i--)
		_model->removeRow(indexes.at(i - 1).row());
}

void AddpkgcodeDialog::accept() {
	return QDialog::accept();
}

int AddpkgcodeDialog::findRow(int code) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, code, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}