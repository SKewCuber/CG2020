//freehand本质上也是多边形，所以不再多写一个freehand类，只是对鼠标响应稍作修改
#include "mPoly.h"
#include "iostream"
#include <Eigen/dense>

using namespace std;
using namespace Eigen;
mPoly::mPoly()
{}

mPoly::~mPoly()
{

}

void mPoly::Draw(QPainter& painter)
{
	painter.setRenderHint(QPainter::Antialiasing);  //设置为抗锯齿
	painter.setPen(QPen(Qt::blue, 3));  //设置画笔颜色及粗细
	for (size_t i = 1; i < points_.size(); i++)
	{
		painter.drawLine(*points_[i - 1], *points_[i]);
	}
	if (stop_polygon_)     //画完时才把最后一条边连上
	{
		painter.drawLine(*points_.back(), *points_[0]);
	}
}

void mPoly::set_lrbt()
{
	left_ = 100000, right_ = -1;
	bottom_ = 100000, top_ = -1;
	for (size_t i = 0; i < points_.size(); i++)
	{
		if (left_ > points_[i]->x())
			left_ = points_[i]->x();
		if (right_ < points_[i]->x())
			right_ = points_[i]->x();
		if (bottom_ > points_[i]->y())
			bottom_ = points_[i]->y();
		if (top_ < points_[i]->y())
			top_ = points_[i]->y();
	}
	width_= right_ - left_ + 1;
	height_= top_ - bottom_ + 1;
}


//inpolygon_的第i行第j列对应图像的(i,j)像素，差一个转置（inpolygon_矩阵转置再输出就是图像的轮廓，见ImageWidget.cpp的testtest）
void mPoly::inPolygon()
{
	int w = width_;
	int h = height_;
	inpolygon_ = MatrixXi::Zero(w, h);
	vector<vector<EDGE>> NET(h, vector<EDGE>(0));

	//get NET from vertices
	for (size_t i = 0; i < points_.size(); i++)
	{
		inpolygon_(points_[i]->x()-left_, points_[i]->y()-bottom_) = -1;

		size_t next = (i + 1 < points_.size()) ? (i + 1) : 0;
		EDGE tmp;
		if (points_[next]->y() != points_[i]->y())
		{
			tmp.dx = (points_[next]->x() - points_[i]->x()) / (double)(points_[next]->y() - points_[i]->y());
			if (points_[next]->y() > points_[i]->y())
			{
				tmp.ymax = points_[next]->y()-bottom_;
				tmp.x = points_[i]->x()-left_;
				NET[points_[i]->y()-bottom_].push_back(tmp);
			}
			else
			{
				tmp.ymax = points_[i]->y()-bottom_;
				tmp.x = points_[next]->x()-left_;
				NET[points_[next]->y()-bottom_].push_back(tmp);
			}
		}
		else
			//NET中无需存储水平边，此处只是把边界点的inpolygon_赋为-1
		{
			int k = (points_[i]->x() < points_[next]->x()) ? 1 : -1;
			int x = points_[i]->x();
			int y = points_[i]->y();
			while (x != points_[next]->x())
			{
				inpolygon_(x-left_, y-bottom_) = -1;
				x += k;
			}
		}
	}

	vector<int>  tmpET(0);
	//扫描线纵坐标i，把第i层的交点(横坐标)填入tmpET，再记录第i层的多边形内部点
	for (int i = 1; i < top_-bottom_; i++)
	{
		tmpET = vector<int>(0);
		for (int j = 0; j <= i; j++)
		{
			for (int k = 0; k < NET[j].size(); k++)
			{
				//如果NET中有一条边NET[j][k]的ymax大于i，且j<=i，则他与扫描线i相交，把交点(横坐标值)放到tmpET中(按从小到大的次序插入)
				if (NET[j][k].ymax > i)
				{
					int x = floor(NET[j][k].x + NET[j][k].dx * (i - j));  //交点的横坐标值
					//针对斜率绝对值较小的情况，画出边界
					int xx = x, kk = (NET[j][k].dx > 0) ? 1 : -1;
					while (xx != floor(NET[j][k].x + NET[j][k].dx * (i + 1 - j)))
					{
						inpolygon_(xx, i) = -1;
						xx+=kk;
					}


					if (tmpET.empty() || x >= tmpET.back())
					{
						tmpET.push_back(x);
					}
					else
					{
						size_t n = tmpET.size() - 1;
						while (n > 0 && x < tmpET[n])
							n--;
						//为什么把if合并到上面的while，改写成n>=0会数组下标越界？
						if (n == 0 && x < tmpET[n])
							n--;
						tmpET.insert(tmpET.begin() + (n + 1), x);  //小细节：若先+n再+1，会越界
					}
				}
			}
		}

		for (int j = 0; j < tmpET.size() - 1; j += 2)
		{
			inpolygon_(tmpET[j], i) = -1;
			inpolygon_(tmpET[j + 1], i) = -1;
			for (int x = tmpET[j] + 1; x < tmpET[j + 1]; x++)
			{
				if (inpolygon_(x, i) == 0)
				{
					inpolygon_(x, i) = 1;
				}
			}
		}
	}

}