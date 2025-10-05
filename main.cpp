// main.cpp
#include "mainwindow.h" // 只需要包含主窗口的头文件
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // 创建应用程序对象
    MainWindow w;             // 创建主窗口对象
    w.show();                 // 显示主窗口
    return a.exec();          // 进入事件循环
}
