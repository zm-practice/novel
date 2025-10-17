#ifndef SIDENAVIGATIONVIEW_H
#define SIDENAVIGATIONVIEW_H
//时钟
#include "timerwindow.h"//***新加***
#include <QWidget>
#include <QListWidgetItem>
// 前向声明 BookManager，避免在头文件中引入完整的类定义，
// 这样可以加快编译速度，并减少循环依赖的风险。
class BookManager;
class QListWidgetItem;
// 这是由 sidenavigationview.ui 生成的UI类的“预告”
namespace Ui {
class SideNavigationView;
}

class SideNavigationView : public QWidget
{
    Q_OBJECT

public:
    // 构造函数和析构函数
    explicit SideNavigationView(QWidget *parent = nullptr);
    ~SideNavigationView();

    // --- 公共接口 ---
    // 这个函数是“桥梁”，让 MainWindow 可以把 BookManager 对象传递进来
    void setBookManager(BookManager *manager);

private slots:
    // --- 新增的槽函数 ---
    // 响应书架页面的“添加”按钮点击
    void on_addButton_clicked();

    // 响应书架页面的“删除”按钮点击
    void on_deleteButton_clicked();

    // 响应 BookManager 发出的 booksChanged() 信号，用于刷新界面
    void refreshBookList();


    // --- 保留的槽函数 ---
    // 响应导航栏的“书架”按钮点击
    void on_bookshelfButton_clicked();

    // 响应导航栏的“我的”按钮点击
    void on_myProfileButton_clicked();
    void on_bookListWidget_itemDoubleClicked(QListWidgetItem *item);

    //时钟
    void showTimerWindow();//***新加***
private:
    // 指向UI类的指针
    Ui::SideNavigationView *ui;

    // 持有一个指向 BookManager 对象的指针，以便调用其功能
    BookManager *m_bookManager = nullptr;

    //time
    TimerWindow *m_timerWindow = nullptr;//***新加***
};

#endif // SIDENAVIGATIONVIEW_H
