#pragma once

#include "Shape.h"

//类名取Ellipse会与BOOL Ellipse函数冲突
class Ellip : public Shape {
public:
	Ellip();
	~Ellip();

	void Draw(QPainter& painter);
};

