#include "ChildWindow.h"
#include "ImageWidget.h"
#include <Eigen/Dense>
#include<iostream>

ChildWindow::ChildWindow(void)
{
	imagewidget_ = new ImageWidget(this);
	setCentralWidget(imagewidget_);
}


ChildWindow::~ChildWindow(void)
{
	delete imagewidget_;
	imagewidget_ = NULL;
}

bool ChildWindow::LoadFile(QString filename)
{
	imagewidget_->Open(filename);
	return true;
}