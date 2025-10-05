#ifndef BOOKMANAGER_H
#define BOOKMANAGER_H

#include <QObject>
#include <QList>
#include <QString>  // 包含所有用到的Qt类
#include "bookdata.h"
#include <QMap>
class BookManager : public QObject
{
    Q_OBJECT
public:
    explicit BookManager(QObject *parent = nullptr);

    // 公共接口：提供给UI调用
    const QList<BookInfo>& getBooks() const;
   void addBook(const BookInfo& newBook);
    void deleteBook(int index);
    void loadBooks(); // 从本地加载书籍列表
    void saveBooks(); // 保存书籍列表到本地
 void setSettingsName(const QString& name); // <-- 确保这行存在！
//书签！！！
    // --- 新增书签管理的公共接口 ---
    void addBookmarks(const QString& bookFilePath, const QList<BookmarkInfo>& bookmarks);
    QList<BookmarkInfo> getBookmarks(const QString& bookFilePath) const;
    void clearBookmarks(const QString& bookFilePath);
signals:
    // 当书籍列表发生任何变化时，就发射这个信号
    void booksChanged();
//书签！！！
    // ★★★ 新增信号：当一本书的书签更新时发射 ★★★
    void bookmarksUpdated(const QString& bookFilePath);

private:
    QList<BookInfo> m_books;
     QString m_settingsName = "DefaultBooks"; // 声明 m_settingsName 变量
//书签！！！
    // ★★★ 使用 QMap 来存储每本书的书签列表 ★★★
    QMap<QString, QList<BookmarkInfo>> m_bookmarks;
};

#endif // BOOKMANAGER_H
