#ifndef BOOKMARKSTYLEDIALOG_H
#define BOOKMARKSTYLEDIALOG_H

#include <QDialog>
#include "bookdata.h"

namespace Ui {
class BookmarkStyleDialog;
}

class BookmarkStyleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BookmarkStyleDialog(const BookmarkInfo& currentStyle, QWidget *parent = nullptr);
    ~BookmarkStyleDialog();

    BookmarkInfo getSelectedStyle() const;

private slots:
    void on_iconButton_clicked();
    void on_bgColorButton_clicked();
    void on_ulColorButton_clicked();
    void updateColorButton(QPushButton* button, const QColor& color);

private:
    Ui::BookmarkStyleDialog *ui;
    BookmarkInfo m_style;
};

#endif // BOOKMARKSTYLEDIALOG_H
