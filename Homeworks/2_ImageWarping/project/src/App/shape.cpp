#include "Shape.h"

Shape::Shape()
{
	start = QPoint(0, 0);
	end = QPoint(0, 0);
	color = Qt::black;
	width = 1;
}

Shape::~Shape()
{
}

void Shape::set_start(QPoint s)
{
	start = s;
}

void Shape::set_end(QPoint e)
{
	end = e;
}

void Shape::set_color(QColor c)
{
	color=c;
}

void Shape::set_width(int w)
{
	width = w;
}
