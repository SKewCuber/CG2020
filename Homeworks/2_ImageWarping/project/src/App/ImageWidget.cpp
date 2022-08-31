#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>
#include <Eigen/Dense>

using std::cout;
using std::endl;
using namespace Eigen;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_1 = new QImage();
	ptr_image_2 = new QImage();
	ptr_image_backup_ = new QImage();
	shape_ = NULL;
}


ImageWidget::~ImageWidget(void)
{}

void ImageWidget::paintEvent(QPaintEvent* paintevent)
{
	QPainter painter;
	painter.begin(this);

	// Draw background
	painter.setBrush(Qt::lightGray);
	QRect back_rect(0, 0, width(), height());
	painter.drawRect(back_rect);

	// Draw image
	int w = ptr_image_->width(), h = ptr_image_->height();
	if (warp_ == false)
	{
		if (w > width())
		{
			h = width() * h / w;
			w = width();
		}
		if (h > height())
		{
			w = height() * w / h;
			h = height();
		}
		temp = (*ptr_image_).scaled(w, h, Qt::KeepAspectRatio);
		QRect rect = QRect((width() - temp.width()) / 2, (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect, temp);
	}
	else
	{
		if (3.5 * w > width())
		{
			h = width() * h / (3.5 * w);
			w = width() / 3.5;
		}
		if (h > height())
		{
			w = height() * w / h;
			h = height();
		}
		temp = (*ptr_image_).scaled(w, h, Qt::KeepAspectRatio);
		QImage temp1 = (*ptr_image_1).scaled(w, h, Qt::KeepAspectRatio);
		QImage temp2 = (*ptr_image_2).scaled(w, h, Qt::KeepAspectRatio);
		QRect rect = QRect((width() - 3 * temp.width()) / 6, (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect, temp);
		QRect rect1 = QRect((width() - 3 * temp.width()) / 2 + temp.width(), (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect1, temp1);
		QRect rect2 = QRect(5 * (width() - 3 * temp.width()) / 6 + 2 * temp.width(), (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect2, temp2);
	}

	//重画所有图形 
	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}
	//再画上这一步画的图形
	if (shape_)
	{
		shape_->Draw(painter);
	}

	painter.end();
}

void ImageWidget::Open()
{
	// Open file
	QString fileName = QFileDialog::getOpenFileName(this, tr("Read Image"), ".", tr("Images(*.bmp *.png *.jpg)"));

	// Load file
	if (!fileName.isEmpty())
	{
		ptr_image_->load(fileName);
		*(ptr_image_backup_) = *(ptr_image_);
		*(ptr_image_1) = *(ptr_image_);
		*(ptr_image_2) = *(ptr_image_);
		warp_ = false;
	}

	//ptr_image_->invertPixels(QImage::InvertRgb);
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	//*(ptr_image_) = ptr_image_->rgbSwapped();
	cout << "image size: " << ptr_image_->width() << ' ' << ptr_image_->height() << endl;
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

	ptr_image_->save(filename);
}

void ImageWidget::Invert()
{
	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			ptr_image_->setPixel(i, j, qRgb(255 - qRed(color), 255 - qGreen(color), 255 - qBlue(color)));
		}
	}

	// equivalent member function of class QImage
	// ptr_image_->invertPixels(QImage::InvertRgb);
	warp_ = false;
	update();
}

void ImageWidget::Mirror(bool ishorizontal, bool isvertical)
{
	QImage image_tmp(*(ptr_image_));
	int width = ptr_image_->width();
	int height = ptr_image_->height();

	if (ishorizontal)
	{
		if (isvertical)
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, height - 1 - j));
				}
			}
		}
		else			//仅水平翻转			
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(width - 1 - i, j));
				}
			}
		}

	}
	else
	{
		if (isvertical)		//仅垂直翻转
		{
			for (int i = 0; i < width; i++)
			{
				for (int j = 0; j < height; j++)
				{
					ptr_image_->setPixel(i, j, image_tmp.pixel(i, height - 1 - j));
				}
			}
		}
	}

	// equivalent member function of class QImage
	//*(ptr_image_) = ptr_image_->mirrored(true, true);
	warp_ = false;
	update();
}

void ImageWidget::TurnGray()
{
	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			QRgb color = ptr_image_->pixel(i, j);
			int gray_value = (qRed(color) + qGreen(color) + qBlue(color)) / 3;
			ptr_image_->setPixel(i, j, qRgb(gray_value, gray_value, gray_value));
		}
	}

	warp_ = false;
	update();
}

void ImageWidget::Restore()
{
	*(ptr_image_) = *(ptr_image_backup_);
	warp_ = false;
	start_points_.clear();
	end_points_.clear();
	shape_list_.clear();
	if (shape_)
	{
		delete shape_;
		shape_ = NULL;
	}
	*(ptr_image_1) = *(ptr_image_);
	*(ptr_image_2) = *(ptr_image_);
	update();
}

void ImageWidget::Warp()
{
	Restore();
	warp_ = true;
	update();
}

void ImageWidget::mousePressEvent(QMouseEvent* event)
{

	if (Qt::LeftButton == event->button() && warp_)   // 判断是否是鼠标左击
	{
		shape_ = new Line;
		if (shape_ != NULL)
		{
			draw_status_ = true;  // 设置绘制状态为 – 绘制
			shape_->set_start(event->pos());
			shape_->set_end(event->pos());
			QPoint* p = new QPoint(event->pos());
			p->setX((p->x() - (width() - 3 * temp.width()) / 6) * ptr_image_->width() / temp.width());
			p->setY((p->y() - (height() - temp.height()) / 2) * ptr_image_->height() / temp.height());
			cout << "start point:" << p->x() << "," << p->y() << endl;
			start_points_.push_back(p);
		}
	}
	update();
}

void ImageWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (draw_status_ && shape_ != NULL)  // 判断当前绘制状态
	{
		shape_->set_end(event->pos());
		//RBF();
	}
	update();
}

void ImageWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (shape_ != NULL)
	{
		draw_status_ = false;   // 设置绘制状态为 – 不绘制
		shape_list_.push_back(shape_);  //把这一步画好的图形加入历史图形中;
		QPoint* p = new QPoint(event->pos());
		p->setX((p->x() - (width() - 3 * temp.width()) / 6) * ptr_image_->width() / temp.width());
		p->setY((p->y() - (height() - temp.height()) / 2) * ptr_image_->height() / temp.height());
		//整数除法，一定要先*ptr_image_->height() 后/temp.height()，反过来就直接变0了
		cout << "end point:" << p->x() << "," << p->y() << endl;
		end_points_.push_back(p);
		RBF();
		RBF_FG();
	}
	update();
}

void ImageWidget::RBF()
{
	size_t n = start_points_.size();
	float d = 1000;
	MatrixXf A(n, n);
	ptr_image_1->fill(QColor(255, 255, 255));
	for (size_t row = 0; row < n; row++)
	{
		for (size_t col = 0; col < n; col++)
		{
			A(row, col) = 1 / (pow((start_points_[row]->x() - start_points_[col]->x()), 2) + pow((start_points_[row]->y() - start_points_[col]->y()), 2) + d);
		}
	}
	MatrixXf b(n, 2);
	for (size_t i = 0; i < n; i++)
	{
		b(i, 0) = end_points_[i]->x() - start_points_[i]->x();
		b(i, 1) = end_points_[i]->y() - start_points_[i]->y();
	}
	MatrixXf a = A.colPivHouseholderQr().solve(b);

	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			int i2 = 0, j2 = 0;
			for (size_t k = 0; k < n; k++)
			{
				i2 = i2 + a(k, 0) / (pow(i - start_points_[k]->x(), 2) + pow(j - start_points_[k]->y(), 2) + d);
				j2 = j2 + a(k, 1) / (pow(i - start_points_[k]->x(), 2) + pow(j - start_points_[k]->y(), 2) + d);
			}
			if (i2 + i >= 0 && i2 + i < ptr_image_->width() && j2 + j >= 0 && j2 + j < ptr_image_->height())
			{
				ptr_image_1->setPixel(i + i2, j + j2, ptr_image_->pixel(i, j));
			}
		}
	}
	update();
}

void ImageWidget::RBF_FG()
{
	size_t n = start_points_.size();
	float d = 1000;
	MatrixXf A(n, n);
	for (size_t row = 0; row < n; row++)
	{
		for (size_t col = 0; col < n; col++)
		{
			A(row, col) = 1 / (pow((end_points_[row]->x() - end_points_[col]->x()), 2) + pow((end_points_[row]->y() - end_points_[col]->y()), 2) + d);
		}
	}
	MatrixXf b(n, 2);
	for (size_t i = 0; i < n; i++)
	{
		b(i, 0) = start_points_[i]->x() - end_points_[i]->x();
		b(i, 1) = start_points_[i]->y() - end_points_[i]->y();
	}
	MatrixXf a = A.colPivHouseholderQr().solve(b);

	for (int i = 0; i < ptr_image_->width(); i++)
	{
		for (int j = 0; j < ptr_image_->height(); j++)
		{
			int i2 = 0, j2 = 0;
			for (size_t k = 0; k < n; k++)
			{
				i2 = i2 + a(k, 0) / (pow(i - end_points_[k]->x(), 2) + pow(j - end_points_[k]->y(), 2) + d);
				j2 = j2 + a(k, 1) / (pow(i - end_points_[k]->x(), 2) + pow(j - end_points_[k]->y(), 2) + d);
			}
			if (i2 + i >= 0 && i2 + i < ptr_image_->width() && j2 + j >= 0 && j2 + j < ptr_image_->height())
			{
				ptr_image_2->setPixel(i, j, ptr_image_->pixel(i + i2, j + j2));
			}
		}
	}
	update();
}