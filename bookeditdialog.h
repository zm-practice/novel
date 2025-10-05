#ifndef BOOKEDITDIALOG_H
#define BOOKEDITDIALOG_H

#include <QDialog>
#include "bookdata.h" // 包含书籍信息结构体
namespace Ui {
class BookEditDialog;
}

class BookEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookEditDialog(QWidget *parent = nullptr);
    ~BookEditDialog();
    // 公共接口：用于设置初始信息和获取最终结果
    void setBookInfo(const BookInfo& info);
    BookInfo getBookInfo() const;
private slots:
    void on_coverButton_clicked();

private:
    Ui::BookEditDialog *ui;
    BookInfo m_bookInfo; // 用一个成员变量来存储书籍信息
};

#endif // BOOKEDITDIALOG_H
