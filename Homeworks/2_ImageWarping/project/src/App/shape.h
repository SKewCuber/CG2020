#pragma once

#include <QtGui>

class Shape
{
public:
	Shape();
	// 父类的析构函数必须是 virtual 的
	// 否则当用父类指针指向子类的实例并删除该实例时，将只会调用父类的析构函数
	// 而不调用子类的析构函数。会造成内存泄漏
	virtual ~Shape();
	virtual void Draw(QPainter& paint) = 0;
	//virtual void AddPoint(QPoint*);  
	void set_start(QPoint s);
	void set_end(QPoint e);
	void set_color(QColor c);
	void set_width(int w);


public:
	enum Type  // 推荐用枚举类代表图元类型
	{
		kDefault = 0,
		kLine = 1,
		kRect = 2,
		kEllipse= 3,
		kPolygon = 4,
	};

protected:
	QPoint start;
	QPoint end;
	QColor color;
	int width;
};

