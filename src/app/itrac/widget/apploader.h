#pragma once

#include "ui/loader.h"

class AppLoader : public Ui::Loader
{
	Q_OBJECT

public:
	AppLoader(QWidget *parent = nullptr);
	~AppLoader() = default;

	void loadMainContent();

private slots:
	void toggleFullScreen();
};
