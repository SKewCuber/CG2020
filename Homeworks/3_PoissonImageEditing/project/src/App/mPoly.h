#pragma once
#include <vector>
#include "shape.h"
#include <Eigen/dense>

using namespace std;
using namespace Eigen;

struct EDGE
{
	double x;      //The abscissa of the intersection between the current scan(in NET, the lowest scanline intersected) line and the edge
	double dx;     //The change in x on this edge from the current scan line to the next scan line
	int ymax;      //The highest scan line crossed by the edge
};

class mPoly :public Shape
{
public:
	mPoly();
	~mPoly();

	void Draw(QPainter& painter);
	bool stop_polygon_ = true;
	MatrixXi inpolygon_;            //点是否在多边形内部（1在，0不在）
	//用扫描线转化算法计算inpolygon_,参考https://blog.csdn.net/xiaowei_cqu/article/details/7712451
	void inPolygon();   
	void set_lrbt();     //计算left_，right_, bottom_, top_

	int left_, right_, bottom_, top_, width_, height_;
};