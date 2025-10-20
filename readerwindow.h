#ifndef READERWINDOW_H
#define READERWINDOW_H

#include <QMainWindow>
#include "bookdata.h"
#include"bookmanager.h"

namespace Ui {
class ReaderWindow;
}

class ReaderWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ReaderWindow(const BookInfo& book,BookManager* bookManager,  QWidget *parent = nullptr);
    //ReaderWindow::ReaderWindow(const BookInfo& book, BookManager* bookManager, QWidget *parent)
    ~ReaderWindow();
    
    // 跳转到指定章节和位置
    void jumpToChapter(int chapterIndex, const QString& position);

private:
    Ui::ReaderWindow *ui;
};

#endif // READERWINDOW_H
