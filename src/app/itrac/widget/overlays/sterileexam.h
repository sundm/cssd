#pragma once

#include "cssd_overlay.h"
#include "core/net/jsonhttpclient.h"

class SterileInfoGroup;
class SterileCheckGroup;
class SterileCheckPackageView;

namespace Sterile {
	struct TestInfo;
};

class SterileExamPanel : public CssdOverlayPanel, public JsonHttpClient
{
	Q_OBJECT

public:
	SterileExamPanel(QWidget *parent = nullptr);

protected:
	void handleBarcode(const QString &) override;

private slots:
	void commit();
private:
	void updateSterileInfo(const QString &);
	void reset();

	SterileInfoGroup* _infoGroup;
	SterileCheckGroup *_checkGroup;
	SterileCheckPackageView *_view;
	std::unique_ptr<Sterile::TestInfo> _testInfo;
};
