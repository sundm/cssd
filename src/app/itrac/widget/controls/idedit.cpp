#include "idedit.h"
#include "idcompleter.h"
#include "dialog/idpicker.h"
#include <QFocusEvent>
#include <QToolButton>
#include <QHBoxLayout>
#include <QAbstractProxyModel>
#include <QTimer>
#include <qdebug>

IdEdit::IdEdit(QWidget *parent)
	: Ui::FlatEdit(QString(), parent)
	, _curId(0)
{
	QToolButton *filterButton = new QToolButton(this);
	filterButton->setCursor(Qt::PointingHandCursor);
	filterButton->setIcon(QIcon(":/res/more-20.png"));
	filterButton->setStyleSheet("QToolButton{padding:0px;}"
		"QToolButton:hover{border:none;}");

	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->addStretch();
	layout->addWidget(filterButton);
	layout->setContentsMargins(0, 8, 0, 7);
	setLayout(layout);

	setTextMargins(0, 0, 28, 0);

	connect(filterButton, &QAbstractButton::clicked, this, &IdEdit::popupFullList);
}

void IdEdit::setCompleter(IdCompleter *completer) {
	//completer->setParent(this); // TODO, neccesary?
	QLineEdit::setCompleter(completer);
	if (completer) {
		connect(completer, SIGNAL(error(const QString&)), this, SIGNAL(error(const QString&)));
		connect(completer, QOverload<const QModelIndex &>::of(&QCompleter::activated),
			[this](const QModelIndex &index) { complete(index); });
	}
}

void IdEdit::reset() {
	// clear internal data
	_curName.clear();
	_curId = 0;
	clear();
	setPlaceholderText("");
}

void IdEdit::focusInEvent(QFocusEvent *e) {
	QLineEdit::focusInEvent(e);
	if (Qt::ActiveWindowFocusReason != e->reason()) {
		clear();
	}
	//qDebug() << "focus in";
}

void IdEdit::focusOutEvent(QFocusEvent *e)
{
	QLineEdit::focusOutEvent(e);
	setPlaceholderText(_curName);
	setText(_curName);
	//qDebug() << "focus out";
}

void IdEdit::complete(const QModelIndex & index) {
	// Be careful, the passed-in index is in the completion model,
	// not the (original) model we populated the completer with.
	/*QAbstractProxyModel *proxyModel =
		static_cast<QAbstractProxyModel *>(_completer->completionModel());
	_curId = _model->data(index, DeptModel::ID_ROLE).toInt();
	_curText = _model->data(index).toString();*/

	IdCompleter *completer = static_cast<IdCompleter *>(this->completer());
	QModelIndex srcIndex =
		static_cast<QAbstractProxyModel *>(
			completer->completionModel())->mapToSource(index);

	int id = completer->id(srcIndex);
	if (id != _curId) {
		_curName = completer->name(srcIndex);
		_curId = id;
		emit changed(id);
	}

	// direct call setText not work, as QLineEdit::focusInEvent 
	// also connects Completer::activated to QLineEdit::setText (every time)
	QTimer::singleShot(0, [this] { setText(_curName); });
}

void IdEdit::popupFullList() {
	IdPicker d(completer()->model(), this);
	d.setWindowTitle("筛选");
	connect(&d, SIGNAL(idPicked(int, const QString &)), this, SLOT(onIdPicked(int, const QString &)));
	d.exec();
}

void IdEdit::setCurrentIdPicked(int id, const QString &name) {
	if (_curId != id) {
		_curId = id;
		_curName = name;
		setText(name);
		emit changed(id);
	}
}

void IdEdit::onIdPicked(int id, const QString &name) {
	if (_curId != id) {
		_curId = id;
		_curName = name;
		setText(name);
		emit changed(id);
	}
}

DeptEdit::DeptEdit(QWidget *parent /*= nullptr*/)
	:IdEdit(parent) {
	setCompleter(new DeptCompleter(this));
}

void DeptEdit::load(DEPTYPE deptType) {
	reset();
	DeptCompleter *completer = static_cast<DeptCompleter *>(this->completer());
	completer->load(deptType);
}

PackageEdit::PackageEdit(QWidget *parent /*= nullptr*/)
	:IdEdit(parent) {
	setCompleter(new PackageCompleter(this));
}

void PackageEdit::loadForDepartment(int deptId) {
	reset();
	PackageCompleter *completer = static_cast<PackageCompleter *>(this->completer());
	completer->loadForDepartment(deptId);
}

void PackageEdit::loadForCategary(const QString &categary) {
	reset();
	PackageCompleter *completer = static_cast<PackageCompleter *>(this->completer());
	completer->loadForCategory(categary);
}

InstrumentEdit::InstrumentEdit(QWidget *parent /*= nullptr*/)
	:IdEdit(parent) {
	setCompleter(new InstrumentCompleter(this));
}

void InstrumentEdit::load() {
	reset();
	InstrumentCompleter *completer = static_cast<InstrumentCompleter *>(this->completer());
	completer->load();
}