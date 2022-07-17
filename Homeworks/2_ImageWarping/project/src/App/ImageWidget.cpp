#include "ImageWidget.h"
#include <QImage>
#include <QPainter>
#include <QtWidgets> 
#include <iostream>

using std::cout;
using std::endl;

ImageWidget::ImageWidget(void)
{
	ptr_image_ = new QImage();
	ptr_image_1 = new QImage();
	ptr_image_2 = new QImage();
	ptr_image_backup_ = new QImage();
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
		QImage temp = (*ptr_image_).scaled(w, h, Qt::KeepAspectRatio);
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
		QImage temp = (*ptr_image_).scaled(w, h, Qt::KeepAspectRatio);
		QImage temp1 = (*ptr_image_1).scaled(w, h, Qt::KeepAspectRatio);
		QImage temp2 = (*ptr_image_2).scaled(w, h, Qt::KeepAspectRatio);
		QRect rect = QRect((width() - 3 * temp.width()) / 6, (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect, temp);
		QRect rect1 = QRect((width() - 3 * temp.width()) / 2 + temp.width(), (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect1, temp1);
		QRect rect2 = QRect(5 * (width() - 3 * temp.width()) / 6 + 2 * temp.width(), (height() - temp.height()) / 2, temp.width(), temp.height());
		painter.drawImage(rect2, temp2);
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
	update();
}

void ImageWidget::Warp()
{
	warp_ = true;

	update();
}