#pragma once

#include <ui_viewwidget.h>

#include "Shape.h"
#include "Line.h"
#include "Rect.h"
#include"Ellip.h"
#include"mPolygon.h"

#include <qevent.h>
#include <qpainter.h>
#include <QWidget>

#include <vector>

class ViewWidget : public QWidget  //QWidget:
{
	Q_OBJECT

public:
	ViewWidget(QWidget* parent = 0);
	~ViewWidget();

private:
	Ui::ViewWidget ui;

private:
	bool draw_status_;   // 当前绘制状态，true 为绘制当前鼠标拖动的图元，false 为不绘制
	QPoint start_point_;  // 当前图元的起始点
	QPoint end_point_;   // 当前图元的终止点
	QColor color;  //当前图元的线条颜色
	int width;  //当前图元的线条粗细
	Shape::Type type_;
	Shape* shape_;   //这一步画的图形
	std::vector<Shape*> shape_list_;  //之前每一步画的图形
	bool stop_polygon = true;


public:
	void mousePressEvent(QMouseEvent* event);  // 鼠标击发响应函数（左右键，单双击）
	void mouseMoveEvent(QMouseEvent* event);   // 鼠标移动响应函数（其一个重要性质在文档最后有详述）
	void mouseReleaseEvent(QMouseEvent* event);  // 鼠标释放响应函数（左右键，单双击）

public:
	void paintEvent(QPaintEvent*);  // Qt 所有的绘制都只能在此函数中完成
signals:
public slots:
	void setLine();
	void setRect();
	void setEllipse();
	void setPolygon();

	void setColor(QColor c);
	void setWidth(int w);
};
