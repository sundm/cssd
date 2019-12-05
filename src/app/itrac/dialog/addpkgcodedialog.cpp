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
#include "model/itemdelegate.h"
#include "ui/composite/waitingspinner.h"
#include "rdao/dao/instrumentdao.h"
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

AddpkgcodeDialog::AddpkgcodeDialog(QWidget *parent)
	: QDialog(parent)
	, _pkgNameEdit(new Ui::FlatEdit)
	, _pkgCodeEdit(new Ui::FlatEdit)
	, _pkgEdit(new PackageEdit)
	, _insRfidEdit(new Ui::FlatEdit)
	, _imgLabel(new XPicture(this))
	, _view(new TableView(this))
	, _model(new QStandardItemModel(0, 2, _view))
	, _unview(new TableView(this))
	, _unmodel(new QStandardItemModel(0, 2, _unview))
	, _waiter(new WaitingSpinner(this))
{
	_isModify = false;
	//_pkgNameEdit->setText(pkg_name);
	//_pkgNameEdit->setReadOnly(true);
	FormGroup * pkgGroup = new FormGroup(this);
	pkgGroup->addRow("包名称:", _pkgNameEdit);
	pkgGroup->addRow("包UDI:", _pkgCodeEdit);
	pkgGroup->addRow("包类型:", _pkgEdit);
	//_package_type_id = pkg_id;

	QHBoxLayout *hLayout = new QHBoxLayout;
	hLayout->setContentsMargins(0, 0, 0, 0);
	hLayout->addWidget(_insRfidEdit);

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
	viewGroup->addRow("器械列表",_view);
	viewGroup->addRow("异常器械", _unview);

	setWindowTitle("包内器械信息");
	_commitButton = new Ui::PrimaryButton("确定", Ui::BtnSize::Small);
	connect(_commitButton, SIGNAL(clicked()), this, SLOT(accept()));

	_resetButton = new Ui::PrimaryButton("重置", Ui::BtnSize::Small);
	connect(_resetButton, SIGNAL(clicked()), this, SLOT(reset()));

	_loadImgButton = new Ui::PrimaryButton("加载图片", Ui::BtnSize::Small);
	connect(_loadImgButton, SIGNAL(clicked()), this, SLOT(loadImg()));

	_imgLabel->setFixedHeight(256);
	_imgLabel->setBgColor(QColor(245, 246, 247));
	_imgLabel->setHidden(true);

	QVBoxLayout *avlayout = new QVBoxLayout();
	avlayout->addWidget(pkgGroup);
	avlayout->addWidget(viewGroup);

	QVBoxLayout *bvlayout = new QVBoxLayout();
	bvlayout->addWidget(_imgLabel);
	bvlayout->addWidget(_loadImgButton);

	QHBoxLayout *cLayout = new QHBoxLayout;
	cLayout->addLayout(avlayout);
	cLayout->addLayout(bvlayout);
	cLayout->setStretch(0, 3);
	cLayout->setStretch(1, 2);


	QHBoxLayout *bLayout = new QHBoxLayout;
	bLayout->addWidget(_resetButton);
	bLayout->addWidget(_commitButton);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->addLayout(cLayout);
	layout->addLayout(bLayout);

	initInstrumentView();

	resize(1000, 900);

	connect(_listener, SIGNAL(onTransponder(const QString&)), this, SLOT(onTransponderReceviced(const QString&)));
	connect(_listener, SIGNAL(onBarcode(const QString&)), this, SLOT(onBarcodeReceviced(const QString&)));
	connect(_pkgEdit, SIGNAL(changed(int)), this, SLOT(onPackageTypeChange(int)));

	//QTimer::singleShot(1000, this, &AddpkgcodeDialog::loadPackageInfo);
	QTimer::singleShot(500, [this] {_pkgEdit->loadForDepartment(0);});

	reset();
}

void AddpkgcodeDialog::initInstrumentView() {
	_model->setHeaderData(0, Qt::Horizontal, "器械类型");
	_model->setHeaderData(1, Qt::Horizontal, "器械UDI");

	_view->setModel(_model);
	_view->setMinimumHeight(500);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *header = _view->horizontalHeader();
	header->setStretchLastSection(true);
	header->resizeSection(0, 150);
	header->resizeSection(1, 50);

	_unmodel->setHeaderData(0, Qt::Horizontal, "器械类型");
	_unmodel->setHeaderData(1, Qt::Horizontal, "器械UDI");
	_unview->setModel(_unmodel);
	_unview->setMinimumHeight(200);
	_unview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	QHeaderView *unheader = _unview->horizontalHeader();
	unheader->setStretchLastSection(true);
	unheader->resizeSection(0, 150);
	unheader->resizeSection(1, 50);
}

void AddpkgcodeDialog::onTransponderReceviced(const QString& code)
{
	qDebug() << code;
	TranspondCode tc(code);
	if (tc.type() == TranspondCode::Package && 0 == _step)
	{
		_pkgCodeEdit->setText(code);
		_step = 1;
	}

	if (tc.type() == TranspondCode::Instrument && 1 == _step)
	{
		if (_codeScanedList.contains(code))
			return;

		_codeScanedList.append(code);

		InstrumentDao dao;
		Instrument ins;
		result_t resp = dao.getInstrument(code, &ins);
		if (resp.isOk())
		{
			bool insert = false;
			QString pkg_id = ins.packageUdi;
			if (pkg_id.isEmpty() || pkg_id.compare(_pkgCodeEdit->text()) == 0) {
				for (size_t i = 0; i < _model->rowCount(); i++)
				{
					int typeId = _model->data(_model->index(i, 0), Qt::UserRole + 1).toInt();
					QString udi = _model->data(_model->index(i, 1)).toString();
					if (udi.isEmpty() && ins.typeId == typeId)
					{
						_model->setData(_model->index(i, 0), ins.name);
						_model->setData(_model->index(i, 1), ins.udi);
						insert = true;
						break;
					}
				}
			}
			
			if (!insert)
			{
				if (_codeUnusualList.contains(code)) return;

				_codeUnusualList.append(code);
				_unmodel->insertRows(0, 1);
				_unmodel->setData(_unmodel->index(0, 0), ins.name);
				_unmodel->setData(_unmodel->index(0, 1), ins.udi);
			}
		}
	}
}

void AddpkgcodeDialog::onBarcodeReceviced(const QString& code)
{
	qDebug() << code;
	Barcode bc(code);

	if (bc.type() == Barcode::Commit) {
		accept();
	}

	if (bc.type() == Barcode::Reset) {
		reset();
	}
	
}

void AddpkgcodeDialog::onPackageTypeChange(int id)
{
	loadInstrumentType(id);
	QString name = _pkgEdit->currentName().append("#");
	_pkgNameEdit->setText(name);
}

void AddpkgcodeDialog::loadPackageInfo()
{
	if (_package_id.isEmpty()) return;

	PackageDao dao;
	Package pk;
	result_t resp = dao.getPackage(_package_id, &pk, true);
	if (resp.isOk())
	{
		_pkgNameEdit->setText(pk.name);
		_pkgCodeEdit->setText(pk.udi);
		PackageType pkt;
		dao.getPackageType(pk.typeId, &pkt);
		_pkgEdit->setCurrentIdPicked(pkt.typeId, pkt.name);

		_model->removeRows(0, _model->rowCount());
		_model->insertRows(0, pk.instruments.count());
		int i = 0;
		for each (auto &item in pk.instruments)
		{
			_model->setData(_model->index(i, 0), item.name);
			_model->setData(_model->index(i, 0), item.typeId, Qt::UserRole + 1);
			_model->setData(_model->index(i, 1), item.udi);
			i++;
		}

		QString imgPath = QString("./photo/package/%1.png").arg(_package_id);
		QFile file(imgPath);
		if (file.exists()) {
			_imgLabel->setImage(imgPath);
			_imgLabel->setHidden(false);
		}
	}
	else
		XNotifier::warn(QString("添加包信息失败: ").append(resp.msg()));
}

bool AddpkgcodeDialog::loadInstrumentType(int packageTypeId)
{
	PackageDao dao;
	PackageType pkt;
	result_t resp = dao.getPackageType(packageTypeId, &pkt, true);
	if (resp.isOk())
	{
		_model->removeRows(0, _model->rowCount());
		for each (auto &item in pkt.detail)
		{
			_model->insertRows(0, item.insNum);
			for (size_t i = 0; i < item.insNum; i++)
			{
				_model->setData(_model->index(i, 0), item.insName);
				_model->setData(_model->index(i, 0), item.insTypeId, Qt::UserRole + 1);
			}
		}
		return true;
	}
	else
	{
		return false;
	}
}

void AddpkgcodeDialog::loadImg() {
	QFileDialog *fileDialog = new QFileDialog(this);
	fileDialog->setWindowTitle(tr("打开图片"));
	fileDialog->setDirectory(".");
	fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg)"));
	fileDialog->setFileMode(QFileDialog::ExistingFiles);
	fileDialog->setViewMode(QFileDialog::Detail);

	QStringList fileNames;
	if (fileDialog->exec())
		fileNames = fileDialog->selectedFiles();

	if (fileNames.size() == 0 || fileNames.size() > 1) return;

	_imgFilePath = fileNames.at(0);
	_imgLabel->setImage(_imgFilePath);
	_imgLabel->setHidden(false);
}

void AddpkgcodeDialog::uploadImg(const QString& instrument_id) {
	QString newFileName = QString("./photo/package/%1.png").arg(instrument_id);
	if (!copyFileToPath(_imgFilePath, newFileName, true)) {
		XNotifier::warn(QString("包信息添加成功，拷贝本地包图片失败!"));
		return;
	}
}

bool AddpkgcodeDialog::copyFileToPath(QString sourceDir, QString toDir, bool coverFileIfExist)
{
	toDir.replace("\\", "/");

	if (sourceDir == toDir) {
		return true;
	}

	if (!QFile::exists(sourceDir)) {
		return false;
	}

	QDir *createfile = new QDir;
	bool exist = createfile->exists(toDir);
	if (exist) {
		if (coverFileIfExist) {
			createfile->remove(toDir);
		}
	}

	return QFile::copy(sourceDir, toDir);
}

void AddpkgcodeDialog::accept() {
	
	PackageDao dao;
	Package pk;

	pk.name = _pkgNameEdit->text();
	if (pk.name.isEmpty())
	{
		_pkgNameEdit->setFocus();
		return;
	}

	pk.udi = _pkgCodeEdit->text();
	if (pk.udi.isEmpty())
	{
		_pkgCodeEdit->setFocus();
		return;
	}

	pk.typeId = _pkgEdit->currentId();
	if (pk.typeId <= 0)
	{
		_pkgEdit->setFocus();
		return;
	}

	pk.instruments = getInstruments();

	if (_isModify)
	{
		//todo
	}
	else {
		//todo
		result_t resp = dao.addPackage(pk);
		if (!resp.isOk())
		{
			XNotifier::warn(QString("添加包信息失败: ").append(resp.msg()));
			return;
		}
	}
	return QDialog::accept();
}

const QList<Instrument> AddpkgcodeDialog::getInstruments()
{
	_insList.clear();

	for (size_t i = 0; i < _model->rowCount(); i++)
	{
		Instrument it;
		it.udi = _model->data(_model->index(i, 1)).toString();
		_insList.append(it);
	}
	return _insList;
}

void AddpkgcodeDialog::reset()
{
	//todo
	_step = 0;
	_model->removeRows(0, _model->rowCount());
	_unmodel->removeRows(0, _unmodel->rowCount());

	_codeScanedList.clear();
	_codeUnusualList.clear();
	_insList.clear();

	_pkgNameEdit->clear();
	_pkgCodeEdit->clear();
	_pkgEdit->clear();

}

void AddpkgcodeDialog::setPackageId(const QString &pkgId)
{
	_package_id = pkgId;
	_isModify = true;

	QTimer::singleShot(1000, this, &AddpkgcodeDialog::loadPackageInfo);
}

int AddpkgcodeDialog::findRow(int code) {
	QModelIndexList matches = _model->match(_model->index(0, 0), Qt::UserRole + 1, code, -1);
	for (const QModelIndex &index : matches) {
		return index.row();
	}
	return -1;
}