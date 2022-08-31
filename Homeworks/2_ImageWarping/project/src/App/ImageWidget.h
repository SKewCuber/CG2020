#pragma once
#include <QWidget>
#include <QPoint>
#include "Shape.h"
#include "Line.h"
#include <vector>

QT_BEGIN_NAMESPACE
class QImage;
class QPainter;
QT_END_NAMESPACE

class ImageWidget :
	public QWidget
{
	Q_OBJECT

public:
	ImageWidget(void);
	~ImageWidget(void);

	void mousePressEvent(QMouseEvent* event);  // 鼠标击发响应函数（左右键，单双击）
	void mouseMoveEvent(QMouseEvent* event);   // 鼠标移动响应函数
	void mouseReleaseEvent(QMouseEvent* event);  // 鼠标释放响应函数（左右键，单双击）

protected:
	void paintEvent(QPaintEvent* paintevent);

public slots:
	// File IO
	void Open();												// Open an image file, support ".bmp, .png, .jpg" format
	void Save();												// Save image to current file
	void SaveAs();												// Save image to another file

	// Image processing
	void Invert();												// Invert pixel value in image
	void Mirror(bool horizontal = false, bool vertical = true);		// Mirror image vertically or horizontally
	void TurnGray();											// Turn image to gray-scale map
	void Restore();												// Restore image to origin
	void Warp();                                                // Image warping
	void RBF();
	void RBF_FG();                                              //RBF with gape filling by inverse maping

private:
	QImage* ptr_image_;				// image 
	QImage* ptr_image_1;
	QImage* ptr_image_2;
	QImage  temp;
	QImage* ptr_image_backup_;
	Shape* shape_ = NULL;   //这一步画的图形
	std::vector<Shape*> shape_list_;  //之前每一步画的图形
	bool draw_status_ = false;   // 当前绘制状态，true 为绘制当前鼠标拖动的图元，false 为不绘制
	bool warp_ = false;
	std::vector<QPoint*> start_points_;
	std::vector<QPoint*> end_points_;
};

