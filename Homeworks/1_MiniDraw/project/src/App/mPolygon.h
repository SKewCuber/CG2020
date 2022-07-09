#pragma once

#include "Shape.h"
#include <QPoint>

class mPolygon : public Shape {
public:
	mPolygon();
	~mPolygon();

	void Draw(QPainter& painter);
	//void AddPoint(QPoint*);

	void AddPoint(QPoint*);
	void UpdateLastPoint(const QPoint&);

private:
	std::vector<QPoint*> points_;
};

