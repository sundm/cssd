#include "userpage.h"
#include "xnotifier.h"
#include "dialog/adduserdialog.h"
#include "core/net/url.h"
#include <xui/searchedit.h>
#include <QtWidgets/QtWidgets>

namespace {
	constexpr int AddItemFlag = 9;
}

UserPage::UserPage(QWidget *parent)
	: QWidget(parent)
	, _searchBox(new SearchEdit(this))
	, _view(new QListView(this))
	, _userModel(new QStandardItemModel(_view))
{
	_searchBox->setPlaceholderText("搜索用户");

	QGridLayout *gridLayout = new QGridLayout(this);
	gridLayout->setVerticalSpacing(20);
	gridLayout->addWidget(_searchBox, 0, 1, 1, 1);
	gridLayout->addWidget(_view, 1, 0, 1, 3);

	initListView();
}

void UserPage::initListView()
{
	_view->setModel(_userModel);
	_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	_view->setMovement(QListView::Static);
	_view->setViewMode(QListView::IconMode);
	_view->setResizeMode(QListView::Adjust);
	_view->setFlow(QListView::LeftToRight);
	_view->setWrapping(true);
	_view->setIconSize(QSize(64, 64));
	_view->setSpacing(15);
	//_view->setUniformItemSizes(true);
	_view->setGridSize(QSize(168, 128));
	_view->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(_view, &QListView::clicked, this, &UserPage::onViewItemClicked);
	connect(_view, SIGNAL(customContextMenuRequested(const QPoint &)), this,
		SLOT(showItemContextMenu(const QPoint &)));
	updateView();
}

void UserPage::updateView()
{
	if (_userModel->rowCount() > 0)
		_userModel->clear();

	post(url(PATH_USER_SEARCH), "{}", [=](QNetworkReply *reply) {
		JsonHttpResponse resp(reply);
		if (!resp.success()) {
			XNotifier::warn(QString("无法获取用户列表: ").append(resp.errorString()));
			return;
		}

		QList<QVariant> users = resp.getAsList("user_list");
		for(auto &user: users) {
			QVariantMap map = user.toMap();
			bool isLocked = "1" == map["is_forbidden"].toString();
			QStandardItem* item = new QStandardItem(
				isLocked ? QIcon(":/res/oplocked.png") : QIcon(":/res/operator64.png"),
				QString("%1[%2]").arg(map["name"].toString(), map["dept_name"].toString()));
			item->setData("1" == map["is_admin"].toString(), TypeRole);
			item->setData(map["operator_id"].toInt(), UIDRole);
			item->setData(isLocked, StateRole);
			_userModel->appendRow(item);
		}

		QStandardItem *item = new QStandardItem(QIcon(":/res/add64.png"), tr("添加新用户"));
		item->setData(AddItemFlag, TypeRole);
		item->setFlags(item->flags() & (~Qt::ItemIsSelectable));
		_userModel->appendRow(item);
	});
}

void UserPage::onViewItemClicked(const QModelIndex& index)
{
	int data = _userModel->data(index, Qt::UserRole + 1).toInt();
	if (isAddItem(index)) {
		AddUserDialog d(this);
		if (QDialog::Accepted == d.exec()){
			updateView();
		}
	}
}

void UserPage::showItemContextMenu(const QPoint & pos)
{
	QModelIndex index(_view->indexAt(pos));
	if (index.isValid() && !isAddItem(index)) {
		QString uid(_userModel->data(index, UIDRole).toString());
		int state = _userModel->data(index, StateRole).toInt();

		QMenu contextMenu(tr("Context menu"), this);
		QAction *act = 0;

		if (true) //TODO
			act = contextMenu.addAction(QIcon(":/res/lock.png"), tr("锁定"), this, SLOT(lockUser()));
		else
			act = contextMenu.addAction(QIcon(":/res/unlock.png"), tr("解锁"), this, SLOT(unLockUser()));
		act->setData(uid);

		act = contextMenu.addAction(tr("修改..."), this, SLOT(removeUser()));
		act->setData(uid);

		contextMenu.exec(QCursor::pos());
	}
}

bool UserPage::isAddItem(const QModelIndex &index) const
{
	return (AddItemFlag == _userModel->data(index, TypeRole).toInt());
}

void UserPage::lockUser()
{
	/*if (QAction *action = qobject_cast<QAction*>(sender())) {
		QString uid(action->data().toString());
		if (0 == Opc::OperatorManager::updateUserState(uid, Opc::Locked))
			updateView();
	}*/
}

void UserPage::unLockUser()
{
	//if (QAction *action = qobject_cast<QAction*>(sender())){
	//	QString uid(action->data().toString());
	//	if (0 == Opc::OperatorManager::updateUserState(uid, Opc::Active))
	//		updateView();
	//}
}

void UserPage::removeUser()
{

}
