#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include "ChildWindow.h"
#include <vector>

#include "mPoly.h"
#include "Poisson.h"

#include <Eigen\Sparse>
#include <Eigen\Dense>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(ChildWindow* relatewindow)
{
	image_ = new QImage();
	image_backup_ = new QImage();

	draw_status_ = kNone;
	is_choosing_ = false;
	is_pasting_ = false;
	is_mixed_ = false;

	point_start_ = QPoint(0, 0);
	point_end_ = QPoint(0, 0);

	source_window_ = NULL;
	shape_ = NULL;
}

ImageWidget::~ImageWidget(void)
{}

int ImageWidget::ImageWidth()
{
	return image_->width();
}

int ImageWidget::ImageHeight()
{
	return image_->height();
}

void ImageWidget::set_draw_status_to_polygon()
{
	draw_status_ = kPolygon;
	/*for (int i = 0; i < 20; i++)
	{
	   image_->setPixel(i,0,qRgb(0,0,0));
	   image_->setPixel(i,1,qRgb(0,0,0));
	}*/    //test
}

void ImageWidget::set_draw_status_to_freehand()
{
	draw_status_ = kFreehand;
}

void ImageWidget::set_draw_status_to_paste()
{
	draw_status_ = kPaste;
	is_mixed_ = false;
	//点下Paste按钮时计算系数矩阵并完成预分解
	//solver = poisson.Get_Poisson_solver(dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_);
}

void ImageWidget::set_draw_status_to_paste_mixed()
{
	draw_status_ = kPaste;
	is_mixed_ = true;
	//solver = poisson.Get_Poisson_solver(dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_);
}

QImage* ImageWidget::image()
{
	return image_;
}

void ImageWidget::set_source_window(ChildWindow* childwindow)
{
	source_window_ = childwindow;
}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	QRect rect = QRect(0, 0, image_->width(), image_->height());
	painter.drawImage(rect, *image_);

	// Draw choose region
	painter.setBrush(Qt::NoBrush);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.setPen(QPen(Qt::blue, 3));

	if (shape_ != NULL)
	{
		/*painter.drawRect(point_start_.x(), point_start_.y(),
			point_end_.x() - point_start_.x(), point_end_.y() - point_start_.y());*/
		shape_->Draw(painter);
	}
	painter.end();
}

void ImageWidget::mousePressEvent(QMouseEvent* mouseevent)
{
	if (Qt::RightButton == mouseevent->button())  //鼠标右击结束多边形绘制
	{
		stop_polygon_ = true;
		dynamic_cast<mPoly*>(shape_)->stop_polygon_ = true;
		is_choosing_ = false;
		dynamic_cast<mPoly*>(shape_)->set_lrbt();
		dynamic_cast<mPoly*>(shape_)->inPolygon();
		poisson.Get_Poisson_solver(dynamic_cast<mPoly*>(shape_)->inpolygon_);
		return;
	}
	if (Qt::LeftButton == mouseevent->button())
	{
		switch (draw_status_)
		{
			case kPolygon:
			{
				//cout << mouseevent->pos().x()<<','<< mouseevent->pos().y() << endl;
				if (stop_polygon_)
				{
					shape_ = new mPoly;
					QPoint* p = new QPoint(mouseevent->pos());
					shape_->points_.push_back(p);
					point_start_ = mouseevent->pos();
					shape_->set_start(mouseevent->pos());
					stop_polygon_ = false;
					dynamic_cast<mPoly*>(shape_)->stop_polygon_ = false;
				}
				QPoint* p = new QPoint(mouseevent->pos());
				shape_->points_.push_back(p);
				point_end_ = mouseevent->pos();
				shape_->set_end(mouseevent->pos());
				setMouseTracking(true);      //鼠标松开时也追踪鼠标（也触发鼠标移动事件）
				is_choosing_ = true;
				break;
			}
			case kFreehand:
			{
				if (stop_polygon_)
				{
					shape_ = new mPoly;
					QPoint* p = new QPoint(mouseevent->pos());
					shape_->points_.push_back(p);
					point_start_ = mouseevent->pos();
					shape_->set_start(mouseevent->pos());
					stop_polygon_ = false;
					dynamic_cast<mPoly*>(shape_)->stop_polygon_ = false;
				}
				QPoint* p = new QPoint(mouseevent->pos());
				shape_->points_.push_back(p);
				point_end_ = mouseevent->pos();
				shape_->set_end(mouseevent->pos());
				is_choosing_ = true;
				break;
			}
			case kPaste:
			{
				{ //这一块的意图是防止用户打开一个图片后直接点Paste，但不知道为什么会在第二个if挂掉
					//给ChildWindow.h中对imagewidget_的定义加上了初始化=NULL后好了，但明明构造函数里是有初始化的，不懂
					// 现在还有一个问题，选好区域后把这张图片关掉，再Paste，sourcewindow已经没了  
					// 解决方法参考20年67号同学，所有子窗口共享一个剪贴板           CClipboard
					// 
					//cout << "kPaste" << endl;
					if (!source_window_)
					{
						QMessageBox::about(this, tr("Error"), tr("Choose first!"));
						return;
					}
					//cout << "source_window_ exists " << endl;
					if (!source_window_->imagewidget_)
					{
						QMessageBox::about(this, tr("Error"), tr("Choose first!"));
						return;
					}
					//cout << "source_window_->imagewidget_ exists" << endl;
					if (!source_window_->imagewidget_->shape_ )
					{
						QMessageBox::about(this, tr("Error"), tr("Choose first!"));
						return;
					}
					//cout << "source_window_->imagewidget_->shape_ exists" << endl;
				}
				

				

				is_pasting_ = true;

				// Start point in object image
				int xpos = mouseevent->pos().rx();
				int ypos = mouseevent->pos().ry();

				// Start point in source image
				int xsourcepos = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->left_;
				int ysourcepos = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->bottom_;

				// Width and Height of the chosen region
				int w = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->width_;
				int h = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->height_;


				// Restore image
				//*(image_) = *(image_backup_);

				// Paste
				/*for (int i = 0; i < w; i++)
				{
					for (int j = 0; j < h; j++)
					{
						if((xpos + i < image_->width()) && (ypos + j < image_->height()) &&
							dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_(i,j)==1)
						image_->setPixel(xpos + i, ypos + j,
							source_window_->imagewidget_->image()->pixel(xsourcepos + i, ysourcepos + j));
					}
				}*/
				if ((xpos > 0) && (ypos > 0) && (xpos + w < image_->width()) && (ypos + h < image_->height()))
				{
					MatrixXi source_inpolygon_ = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_;
					vector<MatrixXi> Data_f(3, MatrixXi::Zero(w, h)), Data_g(3, MatrixXi::Zero(w, h));
					for (int i = 0; i < w; i++)
					{
						for (int j = 0; j < h; j++)
						{
							Data_f[0](i, j) = qRed(image_->pixel(xpos + i, ypos + j));
							Data_f[1](i, j) = qGreen(image_->pixel(xpos + i, ypos + j));
							Data_f[2](i, j) = qBlue(image_->pixel(xpos + i, ypos + j));
							Data_g[0](i, j) = qRed(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
							Data_g[1](i, j) = qGreen(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
							Data_g[2](i, j) = qBlue(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
						}
					}
					cout << 1 << endl;
					vector<VectorXd> solve(3);
					for (int k = 0; k < 3; k++)
					{
						solve[k] = source_window_->imagewidget_->
							poisson.Poisson_solve(Data_f[k].cast<double>(), Data_g[k].cast<double>(), source_inpolygon_, is_mixed_);
					}
					cout << 2 << endl;
					for (int i = 0; i < w; i++)
					{
						for (int j = 0; j < h; j++)
						{
							int r = floor(solve[0](i* h + j)), g = floor(solve[1](i* h + j)), b = floor(solve[2](i* h + j));
							r = (r > 255) ? 255 : ((r < 0 ? 0 : r));
							g = (g > 255) ? 255 : ((g < 0 ? 0 : g));
							b = (b > 255) ? 255 : ((b < 0 ? 0 : b));
							image_->setPixel(xpos + i, ypos + j, qRgb(r, g, b));
						}
					}
				}
				else
				{
					QMessageBox::about(this, tr("Error"), tr("Cannot put it here !!!"));
					return;
				}
			}

			update();
			break;

			default:
				break;
		}
	}
}

void ImageWidget::mouseMoveEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
		case kPolygon:
			if (is_choosing_)
			{
				point_end_ = mouseevent->pos();
				*(shape_->points_.back()) = mouseevent->pos();
			}
			break;

		case kFreehand:
			if (is_choosing_)
			{
				point_end_ = mouseevent->pos();
				QPoint* p = new QPoint(mouseevent->pos());
				shape_->points_.push_back(p);
			}
			break;


		case kPaste:
			// Paste rectangle region to object image
			if (is_pasting_)
			{
				// Start point in object image
				int xpos = mouseevent->pos().rx();
				int ypos = mouseevent->pos().ry();

				// Start point in source image
				int xsourcepos = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->left_;
				int ysourcepos = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->bottom_;

				// Width and Height of rectangle region
				int w = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->width_;
				int h = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->height_;

				// Paste
				if ((xpos > 0) && (ypos > 0) && (xpos + w < image_->width()) && (ypos + h < image_->height()))
				{
					// Restore image 
					*(image_) = *(image_backup_);

					MatrixXi source_inpolygon_ = dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_;
					vector<MatrixXi> Data_f(3, MatrixXi::Zero(w, h)), Data_g(3, MatrixXi::Zero(w, h));
					for (int i = 0; i < w; i++)
					{
						for (int j = 0; j < h; j++)
						{
							Data_f[0](i, j) = qRed(image_->pixel(xpos + i, ypos + j));
							Data_f[1](i, j) = qGreen(image_->pixel(xpos + i, ypos + j));
							Data_f[2](i, j) = qBlue(image_->pixel(xpos + i, ypos + j));
							Data_g[0](i, j) = qRed(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
							Data_g[1](i, j) = qGreen(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
							Data_g[2](i, j) = qBlue(source_window_->imagewidget_->image_->pixel(xsourcepos + i, ysourcepos + j));
						}
					}
					vector<VectorXd> solve(3);
					for (int k = 0; k < 3; k++)
					{
						solve[k] = source_window_->imagewidget_->
							poisson.Poisson_solve(Data_f[k].cast<double>(), Data_g[k].cast<double>(), source_inpolygon_, is_mixed_);
					}
					for (int i = 0; i < w; i++)
					{
						for (int j = 0; j < h; j++)
						{
							int r = floor(solve[0](i* h + j)), g = floor(solve[1](i* h + j)), b = floor(solve[2](i* h + j));
							r = (r > 255) ? 255 : ((r < 0 ? 0 : r));
							g = (g > 255) ? 255 : ((g < 0 ? 0 : g));
							b = (b > 255) ? 255 : ((b < 0 ? 0 : b));
							image_->setPixel(xpos + i, ypos + j, qRgb(r, g, b));
						}
					}
				}
			}

		default:
			break;
	}

	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* mouseevent)
{
	switch (draw_status_)
	{
		case kPolygon:
			if (is_choosing_)
			{
				point_end_ = mouseevent->pos();
				shape_->set_end(point_end_);
			}
			break;
		case kFreehand:
			if (is_choosing_)
			{
				stop_polygon_ = true;
				dynamic_cast<mPoly*>(shape_)->stop_polygon_ = true;
				is_choosing_ = false;
				dynamic_cast<mPoly*>(shape_)->set_lrbt();
				dynamic_cast<mPoly*>(shape_)->inPolygon();
				poisson.Get_Poisson_solver(dynamic_cast<mPoly*>(shape_)->inpolygon_);
			}
			break;
		case kPaste:
			if (is_pasting_)
			{
				is_pasting_ = false;
				draw_status_ = kNone;
			}
			break;
		default:
			break;
	}

	update();
}

void ImageWidget::Open(QString filename)
{
	// Load file
	if (!filename.isEmpty())
	{
		image_->load(filename);
		*(image_backup_) = *(image_);
	}

	//	setFixedSize(image_->width(), image_->height());
	//	relate_window_->setWindowFlags(Qt::Dialog);
	//	relate_window_->setFixedSize(QSize(image_->width(), image_->height()));
	//	relate_window_->setWindowFlags(Qt::SubWindow);

		//image_->invertPixels(QImage::InvertRgb);
		//*(image_) = image_->mirrored(true, true);
		//*(image_) = image_->rgbSwapped();
	cout << "image size: " << image_->width() << ' ' << image_->height() << endl;
	update();
}

void ImageWidget::Save()
{
	SaveAs();
}

void ImageWidget::SaveAs()
{
	QString filename = QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Images(*.bmp *.png *.jpg)"));
	if (filename.isNull())
	{
		return;
	}

	image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// image_->invertPixels(QImage::InvertRgb);
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(image_));
	int width = image_->width();
	int height = image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}

	}
	else
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(image_) = image_->mirrored(true, true);
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < image_->width(); i++)
	{
		for (int j = 0; j < image_->height(); j++)
		{
			QRgb color = image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	update();
}

void ImageWidget::Restore()
{
	*(image_) = *(image_backup_);
	point_start_ = point_end_ = QPoint(0, 0);
	update();
}


void ImageWidget::Show_inpolygon_()
{
	if (!source_window_)
	{
		QMessageBox::about(this, tr("Error"), tr("Choose first!"));
		return;
	}
	if (!source_window_->imagewidget_)
	{
		QMessageBox::about(this, tr("Error"), tr("Choose first!"));
		return;
	}
	if (!source_window_->imagewidget_->shape_)
	{
		QMessageBox::about(this, tr("Error"), tr("Choose first!"));
		return;
	}
	cout << "show matrix inpolygon_^*:" << endl;
	if (source_window_->imagewidget_->shape_ )
	{
		for (int j = 0; j < dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->height_; j++)
		{
			for (int i = 0; i < dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->width_; i++)
			{
				if (dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_(i, j) != -1)
					cout << dynamic_cast<mPoly*>(source_window_->imagewidget_->shape_)->inpolygon_(i, j);
				else
					cout << 2;
			}
			cout << endl;
		}
	}
	else
	{
		cout << "Choose first!" << endl;
	}
}
