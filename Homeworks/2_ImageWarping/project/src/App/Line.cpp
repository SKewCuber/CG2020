#include "Line.h"

Line::Line()
{
}

Line::~Line()
{
}

void Line::Draw(QPainter& painter)
{
	painter.setRenderHint(QPainter::Antialiasing);  //设置为抗锯齿
	painter.setPen(QPen(Qt::blue, 3));  //设置画笔颜色及粗细
	painter.drawLine(start, end);
}
