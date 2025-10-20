#ifndef MYPROFILEVIEW_H
#define MYPROFILEVIEW_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "bookmanager.h"
#include "bookdata.h"

namespace Ui {
class MyProfileView;
}

class MyProfileView : public QWidget
{
    Q_OBJECT

public:
    explicit MyProfileView(QWidget *parent = nullptr);
    ~MyProfileView();

    void refreshContent(); // 刷新内容
    void setBookManager(BookManager *bookManager); // 设置BookManager

signals:
    void openBookRequest(const BookInfo& book, int chapterIndex, int position); // 打开书籍请求信号

private slots:
    void onBookmarksUpdated(const QString& bookFilePath); // 书签更新槽函数
    void onNotesTreeItemClicked(QTreeWidgetItem *item, int column); // 笔记树项点击槽函数
    void onBookmarksTreeItemClicked(QTreeWidgetItem *item, int column); // 书签树项点击槽函数

private:
    Ui::MyProfileView *ui;
    BookManager *m_bookManager;

    void loadBookmarks(); // 加载书签
    void loadNotes(); // 加载笔记
    
    // 存储树形项与数据的映射关系
    struct ItemData {
        QString bookId;
        int chapterIndex;
        int position;
        QString content;
    };
    QMap<QTreeWidgetItem*, ItemData> m_itemDataMap;
};

#endif // MYPROFILEVIEW_H