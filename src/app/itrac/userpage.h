#ifndef OPERATORWIDGET_H
#define OPERATORWIDGET_H

#include <QWidget>
#include "core/net/jsonhttpclient.h"

class QStandardItemModel;
class QListView;
class SearchEdit;
class UserPage : public QWidget, public JsonHttpClient
{
	Q_OBJECT

public:
	UserPage(QWidget *parent = 0);

	enum{TypeRole = Qt::UserRole+1, UIDRole, StateRole};

	void updateView();

private slots:
	void onViewItemClicked(const QModelIndex&);
	void showItemContextMenu(const QPoint &);
	void lockUser();
	void unLockUser();
	void removeUser();

private:
	void initListView();
	inline bool isAddItem(const QModelIndex &index) const;

	SearchEdit *_searchBox;
	QListView *_view;
	QStandardItemModel *_userModel;
};

#endif // OPERATORWIDGET_H
