﻿#include "Rect.h"

Rect::Rect()
{
}

Rect::~Rect()
{
}

void Rect::Draw(QPainter& painter)
{
	painter.setRenderHint(QPainter::Antialiasing);  //设置为抗锯齿
	painter.setPen(QPen(color, width));  //设置画笔颜色及粗细
	painter.drawRect(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());
}
