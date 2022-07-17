#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QAction* hello_world_action_;//��������
    QMenu* main_menu_;//�����˵�
    QToolBar* main_toolbar_; //����������
    void CreateButtons();//��������

private slots: void HelloWorld(); //�ۺ���
};
#endif // MAINWINDOW_H
