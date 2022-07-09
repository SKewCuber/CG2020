#include "viewwidget.h"
#include"iostream"
ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	draw_status_ = false;  // 设置初始绘制状态为 – 不绘制
	shape_ = NULL;
	type_ = Shape::kDefault;
	color = Qt::black;
	width = 1;
}

ViewWidget::~ViewWidget()
{
	for (size_t i = 0; i < shape_list_.size(); i++)
	{
		if (shape_list_[i])
		{
			delete shape_list_[i];
			shape_list_[i] = NULL;
		}
	}   
	//Qt 的资源：QAction* QMenu* 等将由 Qt 自己回收，不需要用户自己释放，即使是用户自己为这些资源申请空间也不需要手动释放空间
	//非 Qt 资源： 需由用户自己申请空间，自己释放空间
	//如果new了不delete会造成内存泄露
}

void ViewWidget::setLine()
{
	type_ = Shape::kLine;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setEllipse()
{
	type_ = Shape::kEllipse;
}

void ViewWidget::setPolygon()
{
	type_ = Shape::kPolygon;
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
	if (Qt::RightButton == event->button())  //鼠标右击结束多边形绘制
	{
		stop_polygon = true;
		return;
	}
	if (Qt::LeftButton == event->button())   // 判断是否是鼠标左击
	{
		switch (type_)
		{
		case Shape::kLine:
			shape_ = new Line();
			break;
		case Shape::kDefault:
			break;

		case Shape::kRect:
			shape_ = new Rect();
			break;

		case Shape::kEllipse:
			shape_ = new Ellip();
			break;

	    //左击时为多边形添加一个点，如果是第一次左击则添加两个点（第一条线段的起点和终点），鼠标移动时更新最后一个点
		case Shape::kPolygon:
			if (stop_polygon)
			{
				shape_ = new mPolygon;
				QPoint* p = new QPoint(event->pos());
				//shape_->AddPoint(p);//类"Shape"没有成员"AddPoint"
				dynamic_cast<mPolygon*>(shape_)->AddPoint(p);
				stop_polygon = false;
			}
			QPoint* p = new QPoint(event->pos());
			dynamic_cast<mPolygon*>(shape_)->AddPoint(p);  
			break;
		}
		if (shape_ != NULL)
		{
			draw_status_ = true;  // 设置绘制状态为 – 绘制
			start_point_ = end_point_ = event->pos();  // 将图元初始点设置为当前鼠标击发点
			shape_->set_start(start_point_);
			shape_->set_end(end_point_);
			shape_->set_color(color);
			shape_->set_width(width);
		}
	}
	update();//更新窗口
}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status_ && shape_ != NULL)  // 判断当前绘制状态
	{
		end_point_ = event->pos();  // 若为真，则设置图元终止点位鼠标当前位置
		shape_->set_end(end_point_);
		if (type_ == Shape::kPolygon)
		{
			dynamic_cast<mPolygon*>(shape_)->UpdateLastPoint(end_point_);
		}
	}
	update();
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL)
	{
		if (type_ == Shape::kPolygon && stop_polygon==false)
		{
			//QPoint* p = new QPoint(event->pos());
			////shape_->AddPoint(p);
			//dynamic_cast<mPolygon*>(shape_)->AddPoint(p);
		}
		else
		{
			draw_status_ = false;   // 设置绘制状态为 – 不绘制
			shape_list_.push_back(shape_);  //把这一步画好的图形加入历史图形中;
			shape_ = NULL;
		}
	}
	update();
}

void ViewWidget::paintEvent(QPaintEvent*)
{
	QPainter painter(this);   // 定义painter在this指向的控件（此例为ViewWidget）中绘图


	//重画所有图形
	for (int i = 0; i < shape_list_.size(); i++)  
	{
		shape_list_[i]->Draw(painter);
	}
	//再画上这一步画的图形
	if (shape_ != NULL) {
		shape_->Draw(painter);
	}

	update();  //更新窗口
}

void ViewWidget::setColor(QColor c)
{
	color = c;
}

void ViewWidget::setWidth(int w)
{
	width = w ;
}