#include "minidraw.h"
#include <QColorDialog>
#include <QInputDialog>
#include <QToolBar>

MiniDraw::MiniDraw(QWidget* parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	view_widget_ = new ViewWidget(); // 实例化 ViewWidget 控件窗口
	Creat_Action();
	Creat_ToolBar();
	Creat_Menu();

	setCentralWidget(view_widget_);  //将view_widget_控件设置在主窗口的中心位置
}

void MiniDraw::Creat_Action() {
	Action_About = new QAction(tr("&About"), this);
	connect(Action_About, &QAction::triggered, this, &MiniDraw::AboutBox);

	Action_Line = new QAction(tr("&Line"), this);
	connect(Action_Line, SIGNAL(triggered()), view_widget_, SLOT(setLine()));

	Action_Rect = new QAction(tr("&Rect"), this);
	connect(Action_Rect, &QAction::triggered, view_widget_, &ViewWidget::setRect);

	Action_Ellipse = new QAction(tr("&Ellipse"), this);
	connect(Action_Ellipse, &QAction::triggered, view_widget_, &ViewWidget::setEllipse);

	Action_Polygon = new QAction(tr("&Polygon"), this);
	connect(Action_Polygon, &QAction::triggered, view_widget_, &ViewWidget::setPolygon);

	Action_Color = new QAction(tr("&Color"), this);
	connect(Action_Color, &QAction::triggered, this, &MiniDraw::color_slot);
	connect(this, &MiniDraw::change_color, view_widget_, &ViewWidget::setColor);

	Action_Width = new QAction(tr("&Width"), this);
	connect(Action_Width, &QAction::triggered, this, &MiniDraw::width_slot);
	connect(this, &MiniDraw::change_width, view_widget_, &ViewWidget::setWidth);

}

void MiniDraw::Creat_ToolBar() {
	pToolBar = addToolBar(tr("&Main"));
	pToolBar->addAction(Action_About);
	pToolBar->addAction(Action_Line);
	pToolBar->addAction(Action_Rect);
	pToolBar->addAction(Action_Ellipse);
	pToolBar->addAction(Action_Polygon);
	pToolBar->addAction(Action_Color);
	pToolBar->addAction(Action_Width);
}

void MiniDraw::Creat_Menu() {
	pMenu = menuBar()->addMenu(tr("&Figure Tool"));
	pMenu->addAction(Action_About);
	pMenu->addAction(Action_Line);
	pMenu->addAction(Action_Rect);
	pMenu->addAction(Action_Ellipse);
	pMenu->addAction(Action_Polygon);
	pMenu->addAction(Action_Color);
	pMenu->addAction(Action_Width);
}

void MiniDraw::AboutBox() {
	QMessageBox::about(this, tr("About"), tr("MiniDraw"));
}

void MiniDraw::color_slot()
{
	QColor c = QColorDialog::getColor(Qt::black);
	emit change_color(c);
}

void MiniDraw::width_slot()
{
	int w = QInputDialog::getInt(this, QString("Set Width"), QString("Input width"), true, 1, 20, 1);
	emit change_width(w);
}

MiniDraw::~MiniDraw() {}
