#include "idpicker.h"
#include "core/constants.h"
#include <xui/searchedit.h>
#include <QListView>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QKeyEvent>
#include <QApplication>

IdPicker::IdPicker(QAbstractItemModel *srcModel, QWidget *parent)
	: QDialog(parent),
	_proxyModel(new QSortFilterProxyModel(this))
{
	_searchEdit = new SearchEdit(this);
	//_searchEdit->setMinimumHeight(32);
	_searchEdit->setFocusPolicy(Qt::StrongFocus);
	setFocusProxy(_searchEdit);

	_view = new QListView(this);
	_proxyModel->setSourceModel(srcModel);
	_proxyModel->setFilterRole(Constant::PinyinRole);
	_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
	_view->setModel(_proxyModel);
	connect(_searchEdit, &QLineEdit::textChanged, _proxyModel, &QSortFilterProxyModel::setFilterFixedString);
	connect(_view, &QAbstractItemView::doubleClicked, this, &IdPicker::acceptEntry);

	_searchEdit->installEventFilter(this);

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_searchEdit);
	layout->addWidget(_view);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
	setLayout(layout);
	resize(300, 500);

	//setWindowFlags(Qt::Tool);
	//Qt::WindowModality modal = windowModality();
	//setWindowModality(Qt::ApplicationModal);
}

bool IdPicker::eventFilter(QObject *obj, QEvent *event)
{
	if (obj == _searchEdit && QEvent::KeyPress == event->type()) {
		QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
		switch (keyEvent->key()) {
		case Qt::Key_Up:
		case Qt::Key_Down:
		case Qt::Key_PageUp:
		case Qt::Key_PageDown:
			QApplication::sendEvent(_view, event);
			return true;
		case Qt::Key_Escape:
			_searchEdit->clear();
			return true;
		case Qt::Key_Enter:
		case Qt::Key_Return:
			acceptCurrentEntry();
			return true;
		}
	}

	return QWidget::eventFilter(obj, event);
}

void IdPicker::acceptCurrentEntry()
{
	const QModelIndex index = _view->currentIndex();
	if (!index.isValid())
		return;
	acceptEntry(index);
}

void IdPicker::acceptEntry(const QModelIndex &index)
{
	QString name = _proxyModel->data(index).toString();
	int id = _proxyModel->data(index, Constant::IdRole).toInt();
	emit idPicked(id, name);
	accept();
}

