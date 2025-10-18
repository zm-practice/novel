// main.cpp
#include "mainwindow.h" // 只需要包含主窗口的头文件
#include <QApplication>
#include <QFile> // 需要包含这个头文件
#include <QTextStream>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv); // 创建应用程序对象

    // 加载QSS样式文件
    QFile file(":/style.qss"); // 假设您将QSS代码保存为style.qss并添加到了资源文件中
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        a.setStyleSheet(stream.readAll());
        file.close();
    }
    MainWindow w;             // 创建主窗口对象
    w.show();                 // 显示主窗口
    return a.exec();          // 进入事件循环
}
