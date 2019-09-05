#ifndef OPERATORWIDGET_H
#define OPERATORWIDGET_H

#include <QWidget>
#include "core/net/jsonhttpclient.h"
#include "core/user.h"

class QStandardItemModel;
class QListView;
class SearchEdit;

enum States {Unlocked, Locked};

class UserPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	UserPage(QWidget *parent = 0);

	enum { TypeRole = Qt::UserRole + 1, UIDRole, StateRole, UserNameRole, UserGenderRole, UserPhoneRole, DptNameRole, DptIdRole, LoginTimeRole };

	void updateView();

private slots:
	void onViewItemClicked(const QModelIndex&);
	void onViewItemDoubleClicked(const QModelIndex&);

	void showItemContextMenu(const QPoint &);
	void lockUser();
	void unLockUser();
	void removeUser();

private:
	void initListView();
	void updateUserState(const QString &userId, const States& state);
	inline bool isAddItem(const QModelIndex &index) const;

	SearchEdit *_searchBox;
	QListView *_view;
	QStandardItemModel *_userModel;

	Core::User _user;
};

#endif // OPERATORWIDGET_H
