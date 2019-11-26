#include "cssdpage.h"
#include "mainwindow.h"
#include "core/itracnamespace.h"

#include "ui/buttons.h"
#include "ui/ui_commons.h"
#include "ui/container.h"
#include "widget/overlays/cssd_overlay.h"
#include "widget/overlays/recycle.h"
#include "widget/overlays/clinic.h"
#include "widget/overlays/wash.h"
#include "widget/overlays/ext.h"
#include "widget/overlays/pack.h"
#include "widget/overlays/sterile.h"
#include "widget/overlays/sterileexam.h"
#include "widget/overlays/bd.h"
#include "widget/overlays/dispatch.h"
#include "widget/overlays/clinic_dispatch.h"
#include "widget/overlays/check.h"
#include <QtGui/QtGui>
#include <QtWidgets/QtWidgets>

DoorPlate::DoorPlate() : QGraphicsObject(), _clickable(true) {
	createInterlData();
	createTransformStates();

	//setCacheMode(ItemCoordinateCache);
	setCursor(Qt::PointingHandCursor);
	setToolTip("点击切换工作区域");
}

void DoorPlate::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *) {
	// Make sure to constrain all painting inside the boundaries of boundingRect()
	// to avoid rendering artifacts(as QGraphicsView does not clip the painter for you)
	painter->setRenderHint(QPainter::Antialiasing);

	painter->setPen(Qt::NoPen);
	painter->setBrush(Qt::darkGray);
	painter->drawPolygon(_polygon.translated(0, adjust));

	Data & data = *_data;
	QLinearGradient gradient(boundingRect().topLeft(), boundingRect().bottomRight());
	gradient.setColorAt(0, data.color.darker(150));
	gradient.setColorAt(1, data.color);
	painter->setBrush(gradient);
	painter->drawPolygon(_polygon);

	QRectF textRect = boundingRect().adjusted(10, 8, -10 - sw, -10);
	QFont font;
	font.setPointSizeF(12.);
	painter->setPen(Qt::white);
	painter->setFont(font);
	painter->drawText(textRect, Qt::AlignCenter, data.text);
}

void DoorPlate::mousePressEvent(QGraphicsSceneMouseEvent * event) {
	if (_clickable) {
		emit clicked();
		_clickable = false;
	}
	//qDebug() << "clicked";
}

// must be implemented, QGraphicsObject is an abastract class
QRectF DoorPlate::boundingRect() const {
	// the press event is only triggerred in the bounding rect
	//return QRectF(QPointF(0, -_pixmap.height() / 2), _pixmap.size());
	return QRectF(QPointF(0, -h / 2), QSize(w, h + adjust));
}

void DoorPlate::updateInternalData() {
	++_data;
	update();
}

void DoorPlate::createInterlData() {
	std::vector<Data> vec;
	vec.reserve(4);
	vec.emplace_back("去污区", QColor(212, 98, 98));
	vec.emplace_back("检查包装及灭菌区", QColor(0, 160, 221));
	vec.emplace_back("无菌物品存放区", Qt::darkGreen);
	vec.emplace_back("手术室操作区", QColor(0, 160, 221));
	_data = vec;

	for (auto point : points)
		_polygon << point;
}

void DoorPlate::createTransformStates() {
	QGraphicsRotation* flipRotation = new QGraphicsRotation();
	flipRotation->setAxis(Qt::XAxis);
	QList<QGraphicsTransform *> transformations;
	transformations << flipRotation;
	setTransformations(transformations);

	// The states and animations.
	QStateMachine *machine = new QStateMachine(this);
	QState *s0 = new QState(machine);
	s0->assignProperty(flipRotation, "angle", 0);

	QState *s1 = new QState(machine);
	s1->assignProperty(flipRotation, "angle", 90);

	QAbstractTransition *t1 = s0->addTransition(this, SIGNAL(clicked()), s1);
	QPropertyAnimation *yRotationAnim = new QPropertyAnimation(flipRotation, "angle");
	yRotationAnim->setDuration(300);
	t1->addAnimation(yRotationAnim);

	QState *s2 = new QState(machine);
	QObject::connect(s2, SIGNAL(entered()), this, SLOT(updateInternalData()));
	s2->assignProperty(flipRotation, "angle", -90);
	s1->addTransition(s1, SIGNAL(propertiesAssigned()), s2);

	QAbstractTransition *t2 = s2->addTransition(s0);
	QObject::connect(s0, &QState::propertiesAssigned, this, [this] { _clickable = true; });
	t2->addAnimation(yRotationAnim);

	machine->setInitialState(s0);
	machine->start();
}

/* class DoorPlatePanel*/
DoorPlatePanel::DoorPlatePanel(QWidget *parent) : QGraphicsView(parent) {
	QGraphicsScene *scene = new QGraphicsScene(this);
	scene->setSceneRect(0, -100, 600, 200); //make (0,0) at left-center

	DoorPlate *doorPlate = new DoorPlate();
	connect(doorPlate, &DoorPlate::clicked, this, &DoorPlatePanel::flipped);
	scene->addItem(doorPlate);
	setScene(scene);

	setFrameStyle(QFrame::NoFrame);
	setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
	setCacheMode(CacheBackground);
	//setViewportUpdateMode(FullViewportUpdate);
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}

CssdAreaPanel::CssdAreaPanel(QWidget *parent) : Ui::Source(parent)
, _layout(new QGridLayout(this))
, _signalMapper(new QSignalMapper(this))
, _count(0) {
	_layout->setContentsMargins(0, 0, 0, 0);
	_layout->setSpacing(80);
	connect(_signalMapper, SIGNAL(mapped(int)), this, SLOT(clickCallback(int)));
}

void CssdAreaPanel::clickCallback(int id) {
	MainWindow *wnd = MainWindow::instance();
	if (!wnd) return;

	CssdOverlayPanel *panel = nullptr;
	switch (id) {
	case itrac::OrRecycleAction: panel = new OrRecyclePanel; break;
	case itrac::NoBarcodeRecycleAction: panel = new NoBCRecyclePanel; break;
	case itrac::ClinicRecycleAction: panel = new ClinicPanel; break;
	case itrac::WashAction: panel = new WashPanel; break;
	case itrac::ExtInstrumentRecycleAction: panel = new ExtManagePanel; break;
	case itrac::PackAction: panel = new PackPanel; break;
	case itrac::SterileAction: panel = new SterilePanel; break;
	case itrac::SterileExamAction: panel = new SterileExamPanel; break;
	case itrac::BDAction: panel = new BDPanel; break;
	case itrac::OrDispatchAction: panel = new OrDispatchPanel; break;
	case itrac::ClinicDispatchAction: panel = new ClinicDispatchPanel; break;
	case itrac::PreExamAction: panel = new PreExamPanel; break;
	case itrac::PostExamAction: panel = new PostExamPanel; break;
	default: return;
	}

	QAbstractButton *button = static_cast<QAbstractButton*>(_signalMapper->mapping(id));
	CssdOverlay *overlay = new CssdOverlay(button->text(), wnd->currentPage());
	overlay->setCentralPanel(panel);
	connect(overlay, &CssdOverlay::destroyed, this, [wnd, this] {
		wnd->setTopbarEnabled(true);
		setEnabled(true);
	});

	wnd->setTopbarEnabled(false);
	setEnabled(false);
	overlay->showAnimated();
}

QAbstractButton * CssdAreaPanel::addButton(int id,
	const QString &file,
	const QString &text,
	const QString &desc) {
	Ui::CommandButton *button = new Ui::CommandButton(QIcon(file), text, desc);
	connect(button, SIGNAL(clicked()), _signalMapper, SLOT(map()));
	_signalMapper->setMapping(button, id);
	int row = _count / columnCount, col = _count % columnCount;
	if (col) {
		_layout->addWidget(Ui::createSeperator(Qt::Vertical), row, col * 2 - 1);
	}
	_layout->addWidget(button, row, col * 2 );
	_count++;
	return button;
}

PollutedAreaPanel::PollutedAreaPanel(QWidget *parent) : CssdAreaPanel(parent) {
	addButton(itrac::OrRecycleAction, ":/res/a1-gun.png", "器械回收", "手术室器械扫码回收");
	//addButton(itrac::ClinicRecycleAction, ":/res/a1-recycle.png", "申领回收", "临床申领器械清点回收");
	//addButton(itrac::BorrowRecycleAction, ":/res/a1-return.png", "借用回收", "临床借用器械清点回收");
	//addButton(itrac::NoBarcodeRecycleAction, ":/res/a1-nobarcode.png", "无码回收", "条码丢失器械回收");
	addButton(itrac::ExtInstrumentRecycleAction, ":/res/a1-ext.png", "外来器械登记", "外来器械/植入物管理");
	addButton(itrac::WashAction, ":/res/a1-wash.png", "清洗", "器械装篮、预清洗、清洗");
}

CleanAreaPanel::CleanAreaPanel(QWidget *parent) : CssdAreaPanel(parent) {
	addButton(itrac::PackAction, ":/res/a2-pack.png", "配包打包", "标签打印、打包、审核");
	addButton(itrac::SterileAction, ":/res/a2-sterile.png", "灭菌", "物品包灭菌");
	addButton(itrac::BDAction, ":/res/a2-bd.png", "BD监测", "灭菌器每日BD登记");
}

AsepsisAreaPanel::AsepsisAreaPanel(QWidget *parent) : CssdAreaPanel(parent) {
	addButton(itrac::SterileExamAction, ":/res/a3-audit.png", "灭菌审核", "物品包灭菌审核");
	addButton(itrac::OrDispatchAction, ":/res/a3-ordispatch.png", "物品发放", "手术室专用物品发放");
	//addButton(itrac::ClinicDispatchAction, ":/res/a3-dispatch.png", "申领发放", "临床申领物品发放");
	//addButton(itrac::BorrowDispatchAction, ":/res/a1-return.png", "借用发放", "临床借用物品发放");
}

OperatingAreaPanel::OperatingAreaPanel(QWidget *parent) : CssdAreaPanel(parent) {
	addButton(itrac::PreExamAction, ":/res/dept-64.png", "术前检查", "创建手术、绑定器械包");
	addButton(itrac::PostExamAction, ":/res/dept-64.png", "术后清点", "手术后清点器械包");
}

CssdButtonsPanelLoader::CssdButtonsPanelLoader(QWidget *parent)
	: Ui::Loader(parent)
	, _cur(0) {
}

void CssdButtonsPanelLoader::loadNext() {
	_cur = (_cur + 1) % 3;
	switch (_cur) {
	case 0:
		Ui::Loader::setSourceAnimated(new PollutedAreaPanel);
		break;
	case 1:
		Ui::Loader::setSourceAnimated(new CleanAreaPanel);
		break;
	case 2:
		Ui::Loader::setSourceAnimated(new AsepsisAreaPanel);
		break;
	case 3:
		Ui::Loader::setSourceAnimated(new OperatingAreaPanel);
		break;
	}
}

CssdPage::CssdPage(QWidget *parent) : QWidget(parent) {
	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);

	DoorPlatePanel *doorPlatePanel = new DoorPlatePanel;
	doorPlatePanel->setFixedHeight(200);
	layout->addWidget(doorPlatePanel);

	/*PollutedAreaPanel *panel = new PollutedAreaPanel();
	CssdButtonsPanelLoader *loader = new CssdButtonsPanelLoader;
	loader->setSource(panel);*/
	_loader = new InternalLoader(0);
	Ui::Container *container = new Ui::Container(_loader, Ui::Container::Top);
	connect(doorPlatePanel, &DoorPlatePanel::flipped, _loader, &InternalLoader::loadNext);

	layout->addWidget(container);
}
