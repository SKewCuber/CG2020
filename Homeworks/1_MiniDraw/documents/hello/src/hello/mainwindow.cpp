#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include<QToolBar>
#include <qmessagebox.h>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    CreateButtons();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::HelloWorld()
{
    QMessageBox::about(this, tr("Hello world"),tr("Well done! Go ahead."));
}

void MainWindow::CreateButtons()
{
    // 创建动作，标签为Hello world
    //& 的作用：设置此动作的快捷键为标签字符串的首字母，此例为 'H'
    hello_world_action_ = new QAction(tr("&Hello world"), this);  

    

    main_menu_ = menuBar()->addMenu(tr("&Main"));//创建菜单，标签为Main
    
    main_menu_->addAction(hello_world_action_);

    main_toolbar_ = addToolBar(tr("&Main"));

    main_toolbar_->addAction(hello_world_action_);
    connect(hello_world_action_, &QAction::triggered, this, &MainWindow::HelloWorld);
}
