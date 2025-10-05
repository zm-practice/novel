#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class BookManager; // 前向声明

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionToggleSpace_triggered();

private:
    Ui::MainWindow *ui;
    bool m_isWorkSpace;
    BookManager *m_workBookManager;
    BookManager *m_leisureBookManager;
};
#endif // MAINWINDOW_H
