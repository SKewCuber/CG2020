#include "mPolygon.h"

mPolygon::mPolygon()
{
}

mPolygon::~mPolygon()
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

void mPolygon::Draw(QPainter& painter)
{
	painter.setRenderHint(QPainter::Antialiasing);  //设置为抗锯齿
	painter.setPen(QPen(color, width));  //设置画笔颜色及粗细
	for (size_t i = 1; i < points_.size(); i++)
	{
		painter.drawLine(*points_[i-1], *points_[i]);
	}
	painter.drawLine(*points_.back(), *points_[0]);
}

void mPolygon::AddPoint(QPoint* p)
{
	points_.push_back(p);
}

void mPolygon::UpdateLastPoint(const QPoint& p)
{
	*(points_.back()) = p;
}