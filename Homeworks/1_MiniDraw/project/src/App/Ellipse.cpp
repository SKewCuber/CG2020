#include "Ellipse.h"

Ellipse::Ellipse()
{
}

Ellipse::~Ellipse()
{
}

void Ellipse::Draw(QPainter& painter)
{
	painter.drawEllipse(start.x(), start.y(),
		end.x() - start.x(), end.y() - start.y());
}
