#include "shape.h"

Shape::Shape()
{}

Shape::~Shape()
{
	for (size_t i = 0; i < points_.size(); i++)
	{
		if (points_[i])
		{
			delete points_[i];
			points_[i] = NULL;
		}
	}
}

void Shape::set_start(QPoint s)
{
	start = s;
}

void Shape::set_end(QPoint e)
{
	end = e;
}