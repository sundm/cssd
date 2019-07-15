#pragma once

#include "ui/loader.h"
#include "ui_issuepanel.h"

class IssuePanel : public Ui::Source
{
	Q_OBJECT

public:
	IssuePanel(QWidget *parent = Q_NULLPTR);
	~IssuePanel();

private:
	Ui::IssuePanel ui;
};
