// #include "bookmanager.h"
// #include <QFileInfo>
// #include <QSettings>

// BookManager::BookManager(QObject *parent) : QObject(parent)
// {
//     // 构造函数保持为空，或者只做通用初始化。
//     // loadBooks() 将在 setSettingsName() 中被调用。
// }

// // ★★★ 实现缺失的 setSettingsName 函数 ★★★
// void BookManager::setSettingsName(const QString &name)
// {
//     m_settingsName = name;
//     loadBooks(); // 当名字被设置后，立刻加载对应的数据
// }

// const QList<BookInfo>& BookManager::getBooks() const
// {
//     return m_books;
// }

// // ...
// // ★★★ 核心改动 ★★★
// void BookManager::addBook(const BookInfo &newBook)
// {
//     m_books.append(newBook);
//     saveBooks();
//     emit booksChanged();
// }
// // ...

// void BookManager::deleteBook(int index)
// {
//     if (index >= 0 && index < m_books.size()) {
//         m_books.removeAt(index);
//         saveBooks();
//         emit booksChanged();
//     }
// }

// // ★★★ 修改 loadBooks 函数，让它使用 m_settingsName ★★★
// void BookManager::loadBooks()
// {
//     QSettings settings("MyNovelReader", "UserData");

//     // --- 1. 加载书籍列表到 m_books ---
//     int bookSize = settings.beginReadArray(m_settingsName);
//     m_books.clear();
//     for (int i = 0; i < bookSize; ++i) {
//         settings.setArrayIndex(i);
//         BookInfo book;
//         book.title = settings.value("title").toString();
//         book.filePath = settings.value("filePath").toString();
//         book.coverPath = settings.value("coverPath").toString();
//         m_books.append(book);
//     }
//     settings.endArray();
//     emit booksChanged(); // 通知UI书籍列表已更新

//     // --- 2. 加载书签数据到 m_bookmarks ---
//     m_bookmarks.clear();
//     settings.beginGroup("Bookmarks");
//     // 获取所有已经保存了书签的书籍路径 (这些路径是 group 的 key)
//     const QStringList bookPathsWithBookmarks = settings.childKeys();
//     for (const QString& bookPath : bookPathsWithBookmarks) {
//         QList<BookmarkInfo> bookmarks;
//         int bookmarkSize = settings.beginReadArray(bookPath);
//         for (int i = 0; i < bookmarkSize; ++i) {
//             settings.setArrayIndex(i);
//             BookmarkInfo bookmark;
//             bookmark.chapterIndex = settings.value("chapterIndex").toInt();
//             bookmark.startPos = settings.value("startPos").toInt();
//             bookmark.endPos = settings.value("endPos").toInt();
//             bookmark.iconPath = settings.value("iconPath").toString();
//             bookmark.backgroundColor = settings.value("bgColor").value<QColor>();
//             bookmark.underlineStyle = settings.value("ulStyle").toInt();
//             bookmark.underlineColor = settings.value("ulColor").value<QColor>();
//             bookmarks.append(bookmark);
//         }
//         settings.endArray();
//         m_bookmarks.insert(bookPath, bookmarks);
//     }
//     settings.endGroup();
// }
// // ★★★ 修改 saveBooks 函数，让它使用 m_settingsName ★★★
// void BookManager::saveBooks()
// {
//     // ★★★ 在函数开头只定义一次 settings 对象 ★★★
//     QSettings settings("MyNovelReader", "UserData");

//     // --- 1. 保存书籍列表 ---
//     settings.beginWriteArray(m_settingsName);
//     for (int i = 0; i < m_books.size(); ++i) {
//         settings.setArrayIndex(i);
//         settings.setValue("title", m_books[i].title);
//         settings.setValue("filePath", m_books[i].filePath);
//         settings.setValue("coverPath", m_books[i].coverPath);
//     }
//     settings.endArray();

//     // --- 2. 保存书签数据 ---
//     settings.beginGroup("Bookmarks");
//     settings.remove(""); // 清除旧的所有书签，避免脏数据
//     // 遍历 m_bookmarks (这是一个 QMap)
//     for (auto it = m_bookmarks.constBegin(); it != m_bookmarks.constEnd(); ++it) {
//         // it.key() 是书籍文件路径, it.value() 是 QList<BookmarkInfo>
//         settings.beginWriteArray(it.key());
//         const QList<BookmarkInfo>& bookmarks = it.value();
//         for (int i = 0; i < bookmarks.size(); ++i) {
//             settings.setArrayIndex(i);
//             const auto& currentBookmark = bookmarks[i];
//             settings.setValue("chapterIndex", currentBookmark.chapterIndex);
//             settings.setValue("startPos", currentBookmark.startPos);
//             settings.setValue("endPos", currentBookmark.endPos);
//             settings.setValue("iconPath", currentBookmark.iconPath);
//             settings.setValue("bgColor", currentBookmark.backgroundColor);
//             settings.setValue("ulStyle", currentBookmark.underlineStyle);
//             settings.setValue("ulColor", currentBookmark.underlineColor);
//         }
//         settings.endArray();
//     }
//     settings.endGroup();
// }
// //书签！！！
// // ★★★ 实现书签管理的新接口 ★★★
// void BookManager::addBookmarks(const QString &bookFilePath, const QList<BookmarkInfo> &bookmarks)
// {
//     m_bookmarks[bookFilePath] = bookmarks;
//     saveBooks(); // 保存
//     emit bookmarksUpdated(bookFilePath); // 发射信号
// }

// QList<BookmarkInfo> BookManager::getBookmarks(const QString &bookFilePath) const
// {
//     return m_bookmarks.value(bookFilePath);
// }

// void BookManager::clearBookmarks(const QString &bookFilePath)
// {
//     m_bookmarks.remove(bookFilePath);
//     saveBooks();
//     emit bookmarksUpdated(bookFilePath);
// }




#include "bookmanager.h"
#include <QFileInfo>
#include <QSettings>
#include <QColor>

BookManager::BookManager(QObject *parent) : QObject(parent) {}

void BookManager::setSettingsName(const QString &name)
{
    m_settingsName = name;
    loadBooks();
}

const QList<BookInfo>& BookManager::getBooks() const { return m_books; }

void BookManager::addBook(const BookInfo &newBook)
{
    m_books.append(newBook);
    saveBooks();
    emit booksChanged();
}

void BookManager::deleteBook(int index)
{
    if (index >= 0 && index < m_books.size()) {
        m_books.removeAt(index);
        saveBooks();
        emit booksChanged();
    }
}

void BookManager::loadBooks()
{
    QSettings settings("MyNovelReader", "UserData");

    // --- 1. 只加载书籍列表 ---
    int bookSize = settings.beginReadArray(m_settingsName);
    m_books.clear();
    for (int i = 0; i < bookSize; ++i) {
        settings.setArrayIndex(i);
        BookInfo book;
        book.title = settings.value("title").toString();
        book.filePath = settings.value("filePath").toString();
        book.coverPath = settings.value("coverPath").toString();
        m_books.append(book);
    }
    settings.endArray();
    emit booksChanged();

    // --- 2. 独立加载所有书签 ---
    m_bookmarks.clear();
    settings.beginGroup("Bookmarks");
    const QStringList bookPaths = settings.childKeys();
    for (const QString& bookPath : bookPaths) {
        QList<BookmarkInfo> bookmarks;
        int bookmarkSize = settings.beginReadArray(bookPath);
        for (int i = 0; i < bookmarkSize; ++i) {
            settings.setArrayIndex(i);
            BookmarkInfo bookmark;
            bookmark.chapterIndex = settings.value("chapterIndex").toInt();
            bookmark.startPos = settings.value("startPos").toInt();
            bookmark.endPos = settings.value("endPos").toInt();
            bookmark.iconPath = settings.value("iconPath").toString();
            bookmark.backgroundColor = settings.value("bgColor").value<QColor>();
            bookmark.underlineStyle = settings.value("ulStyle").toInt();
            bookmark.underlineColor = settings.value("ulColor").value<QColor>();
            bookmarks.append(bookmark);
        }
        settings.endArray();
        m_bookmarks.insert(bookPath, bookmarks);
    }
    settings.endGroup();
}

void BookManager::saveBooks()
{
    QSettings settings("MyNovelReader", "UserData");

    // --- 1. 保存书籍列表 ---
    settings.beginWriteArray(m_settingsName);
    for (int i = 0; i < m_books.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("title", m_books[i].title);
        settings.setValue("filePath", m_books[i].filePath);
        settings.setValue("coverPath", m_books[i].coverPath);
    }
    settings.endArray();

    // --- 2. 保存书签 ---
    settings.beginGroup("Bookmarks");
    settings.remove("");
    for (auto it = m_bookmarks.constBegin(); it != m_bookmarks.constEnd(); ++it) {
        settings.beginWriteArray(it.key());
        for (int i = 0; i < it.value().size(); ++i) {
            settings.setArrayIndex(i);
            const auto& currentBookmark = it.value()[i];
            settings.setValue("chapterIndex", currentBookmark.chapterIndex);
            settings.setValue("startPos", currentBookmark.startPos);
            settings.setValue("endPos", currentBookmark.endPos);
            settings.setValue("iconPath", currentBookmark.iconPath);
            settings.setValue("bgColor", currentBookmark.backgroundColor);
            settings.setValue("ulStyle", currentBookmark.underlineStyle);
            settings.setValue("ulColor", currentBookmark.underlineColor);
        }
        settings.endArray();
    }
    settings.endGroup();
}

void BookManager::addBookmarks(const QString &bookFilePath, const QList<BookmarkInfo> &bookmarks)
{
    m_bookmarks[bookFilePath] = bookmarks;
    saveBooks();
    emit bookmarksUpdated(bookFilePath);
}

QList<BookmarkInfo> BookManager::getBookmarks(const QString &bookFilePath) const
{
    return m_bookmarks.value(bookFilePath);
}

void BookManager::clearBookmarks(const QString &bookFilePath)
{
    m_bookmarks.remove(bookFilePath);
    saveBooks();
    emit bookmarksUpdated(bookFilePath);
}
