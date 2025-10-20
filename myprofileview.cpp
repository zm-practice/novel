#include "myprofileview.h"
#include "ui_myprofileview.h"
#include <QDebug>

MyProfileView::MyProfileView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MyProfileView),
    m_bookManager(nullptr)
{
    ui->setupUi(this);

    // 连接信号槽
    connect(ui->notesTreeWidget, &QTreeWidget::itemClicked, this, &MyProfileView::onNotesTreeItemClicked);
    connect(ui->bookmarksTreeWidget, &QTreeWidget::itemClicked, this, &MyProfileView::onBookmarksTreeItemClicked);
}

void MyProfileView::setBookManager(BookManager *bookManager)
{
    m_bookManager = bookManager;
    
    if (m_bookManager) {
        // 连接信号槽
        connect(m_bookManager, &BookManager::bookmarksUpdated, this, &MyProfileView::onBookmarksUpdated);
        
        // 初始加载数据
        loadBookmarks();
        loadNotes();
    }
}

MyProfileView::~MyProfileView()
{
    delete ui;
}

void MyProfileView::refreshContent()
{
    // 清空现有数据
    ui->notesTreeWidget->clear();
    ui->bookmarksTreeWidget->clear();
    m_itemDataMap.clear();

    // 重新加载数据
    loadBookmarks();
    loadNotes();
}

void MyProfileView::onBookmarksUpdated(const QString &bookFilePath)
{
    // 当书签更新时刷新界面
    refreshContent();
}

void MyProfileView::onNotesTreeItemClicked(QTreeWidgetItem *item, int column)
{
    // 检查是否是叶子节点（笔记条目）
    if (item->childCount() == 0 && m_itemDataMap.contains(item)) {
        // 获取关联的数据
        const ItemData &data = m_itemDataMap[item];
        
        // 在预览区域显示笔记内容
        ui->previewTextBrowser->setText(data.content);
        ui->previewTitleLabel->setText("笔记内容预览");
        
        // 设置预览区域的样式
        ui->previewTextBrowser->setStyleSheet("QTextBrowser { background-color: #f5f5f5; border: 1px solid #ddd; padding: 10px; }");
        ui->previewTitleLabel->setStyleSheet("QLabel { font-weight: bold; color: #2c3e50; font-size: 14px; }");
        
        // 单击即可跳转到阅读界面
        if (QApplication::mouseButtons() == Qt::LeftButton) {
            // 查找对应的书籍信息
            for (const BookInfo &book : m_bookManager->getBooks()) {
                if (book.id == data.bookId) {
                    emit openBookRequest(book, data.chapterIndex, data.position);
                    break;
                }
            }
        }
    }
}

void MyProfileView::onBookmarksTreeItemClicked(QTreeWidgetItem *item, int column)
{
    // 检查是否是叶子节点（即书签节点）
    if (item->childCount() == 0 && m_itemDataMap.contains(item)) {
        // 获取关联的数据
        const ItemData &data = m_itemDataMap[item];
        
        // 在预览区域显示书签内容
        ui->previewTextBrowser->setText(data.content);
        ui->previewTitleLabel->setText("书签内容预览");
        
        // 设置预览区域的样式
        ui->previewTextBrowser->setStyleSheet("QTextBrowser { background-color: #f5f5f5; border: 1px solid #ddd; padding: 10px; }");
        ui->previewTitleLabel->setStyleSheet("QLabel { font-weight: bold; color: #2c3e50; font-size: 14px; }");
        
        // 单击即可跳转到阅读界面
        if (QApplication::mouseButtons() == Qt::LeftButton) {
            // 查找对应的书籍信息
            for (const BookInfo &book : m_bookManager->getBooks()) {
                if (book.id == data.bookId) {
                    emit openBookRequest(book, data.chapterIndex, data.position);
                    break;
                }
            }
        }
    }
}

void MyProfileView::loadBookmarks()
{
    // 清空现有数据
    ui->bookmarksTreeWidget->clear();
    
    // 获取所有书籍
    const QList<BookInfo> &books = m_bookManager->getBooks();
    
    // 遍历每本书
    for (const BookInfo &book : books) {
        // 获取该书的所有书签
        QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(book.filePath);
        
        // 过滤出书签类型的书签
        QList<BookmarkInfo> actualBookmarks;
        for (const BookmarkInfo &bookmark : bookmarks) {
            if (bookmark.type == BookmarkInfo::Bookmark) {
                actualBookmarks.append(bookmark);
            }
        }
        
        if (actualBookmarks.isEmpty()) {
            continue;  // 如果没有书签，跳过
        }
        
        // 创建书籍节点
        QTreeWidgetItem *bookItem = new QTreeWidgetItem(ui->bookmarksTreeWidget);
        bookItem->setText(0, book.title);
        bookItem->setIcon(0, QIcon(":/icons/bookshelf.png"));
        
        // 遍历每个书签
        for (const BookmarkInfo &bookmark : actualBookmarks) {
            QTreeWidgetItem *bookmarkItem = new QTreeWidgetItem(bookItem);
            
            // 从文件中提取书签内容
            QString content = bookmark.content.isEmpty() ? 
                QString("书签内容 (位置: %1-%2)").arg(bookmark.startPos).arg(bookmark.endPos) : 
                bookmark.content;
            QString chapterInfo = QString("第%1章").arg(bookmark.chapterIndex + 1);
            
            bookmarkItem->setText(0, chapterInfo + ": " + content.left(20) + "...");
            
            // 存储关联数据
            ItemData data;
            data.bookId = book.id;
            data.chapterIndex = bookmark.chapterIndex;
            data.position = bookmark.startPos;
            data.content = content;
            m_itemDataMap[bookmarkItem] = data;
        }
    }
    
    // 展开树形结构
    ui->bookmarksTreeWidget->expandAll();
}

void MyProfileView::loadNotes()
{
    // 清空现有数据
    ui->notesTreeWidget->clear();
    
    // 获取所有书籍
    const QList<BookInfo> &books = m_bookManager->getBooks();
    
    // 遍历每本书
    for (const BookInfo &book : books) {
        // 获取该书的所有笔记
        QList<BookmarkInfo> notes = m_bookManager->getBookmarks(book.filePath);
        
        // 过滤出笔记类型的书签
        QList<BookmarkInfo> actualNotes;
        for (const BookmarkInfo &bookmark : notes) {
            if (bookmark.type == BookmarkInfo::Note) {
                actualNotes.append(bookmark);
            }
        }
        
        if (actualNotes.isEmpty()) {
            continue;  // 如果没有笔记，跳过
        }
        
        // 创建书籍节点
        QTreeWidgetItem *bookItem = new QTreeWidgetItem(ui->notesTreeWidget);
        bookItem->setText(0, book.title);
        bookItem->setIcon(0, QIcon(":/icons/bookshelf.png"));
        
        // 遍历每个笔记
        for (const BookmarkInfo &note : actualNotes) {
            QTreeWidgetItem *noteItem = new QTreeWidgetItem(bookItem);
            
            // 从文件中提取笔记内容
            QString content = note.content.isEmpty() ? 
                QString("笔记内容 (位置: %1-%2)").arg(note.startPos).arg(note.endPos) : 
                note.content;
            QString chapterInfo = QString("第%1章").arg(note.chapterIndex + 1);
            
            // 显示章节信息和笔记内容的前20个字符
            noteItem->setText(0, chapterInfo + ": " + content.left(20) + "...");
            
            // 存储关联数据
            ItemData data;
            data.bookId = book.id;
            data.chapterIndex = note.chapterIndex;
            data.position = note.startPos;
            data.content = content;
            m_itemDataMap[noteItem] = data;
        }
    }
    
    // 展开树形结构
    ui->notesTreeWidget->expandAll();
}