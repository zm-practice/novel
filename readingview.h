// // readingview.h
// #ifndef READINGVIEW_H
// #define READINGVIEW_H

// #include <QWidget>
// #include <QTextCharFormat>
// #include "bookdata.h"

// //在线查词！！！
// #include "dictionaryservice.h"
// class TooltipPopup; // 使用前向声明

// class QListWidgetItem;
// class BookManager;

// //自动阅读！！！
// class QTimer; // <-- 使用前向声明，而不是 #include <QTimer>
// namespace Ui {
// class ReadingView;
// }

// class ReadingView : public QWidget
// {
//     Q_OBJECT
// public:
//     explicit ReadingView(QWidget *parent = nullptr);
//     ~ReadingView();
//     void loadBook(const BookInfo& book);
//     void setBookManager(BookManager* manager);
// //自动阅读！！！
//     // ★★★ 新增：公共控制接口 ★★★
//     void startAutoScroll();
//     void stopAutoScroll();
//     void setAutoScrollSpeed(int interval);

// private slots:
//     void on_chaptersListWidget_itemClicked(QListWidgetItem *item);
//     void applySettings();
//     void showContextMenu(const QPoint& pos);
//     void addBookmarkFromSelection();
//     void removeBookmark();
//     void changeBookmarkStyle();

// //在线查词！！！
//     // 响应查词结果的槽
//     void onQuerySuccess(const WordDefinition& result);
//     void onQueryError(const QString& errorString);
// //自动阅读！！！
//     // ★★★ 新增：响应内部UI控件的槽函数 ★★★
//     void on_autoScrollButton_toggled(bool checked);
//     void on_speedSlider_valueChanged(int value);
//     void onAutoScrollTimerTimeout();

// private:
//     void parseChapters(const QString& content);
//     void applyBookmarksToChapter(int chapterIndex);

//     Ui::ReadingView *ui;
//     BookManager *m_bookManager = nullptr;
//     BookInfo m_currentBook;
//     int m_currentChapterIndex = -1;
//     QStringList m_chapterContents;
// //自动阅读！！！
//     // ★★★ 新增成员变量 ★★★
//     QTimer *m_autoScrollTimer;


// //在线查词！！！
//     DictionaryService* m_dictionaryService;
//     TooltipPopup* m_tooltipPopup = nullptr; // 初始化为 nullptr
// //在线查词！！！
// protected:
//     // ★★★ 重写鼠标双击事件来触发查词 ★★★
//     void mouseDoubleClickEvent(QMouseEvent *event) override;
// };
// #endif // READINGVIEW_H




#ifndef READINGVIEW_H
#define READINGVIEW_H

#include <QWidget>
#include <QTextCharFormat>
#include<QLineEdit>
#include <QPushButton>
#include "bookdata.h"
#include "dictionaryservice.h"
#include "tooltippopup.h" // ★★★ 直接包含头文件 ★★★

class QListWidgetItem;
class BookManager;
class QTimer;
class TooltipPopup;
class QMouseEvent;
class QTextCursor;

namespace Ui {
class ReadingView;
}

class ReadingView : public QWidget
{
    Q_OBJECT
public:
    explicit ReadingView(QWidget *parent = nullptr);
    ~ReadingView();

    void loadBook(const BookInfo& book);
    void setBookManager(BookManager* manager);

    void startAutoScroll();
    void stopAutoScroll();
    void setAutoScrollSpeed(int interval);

protected:
    //void mouseDoubleClickEvent(QMouseEvent *event) override;
     bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void on_chaptersListWidget_itemClicked(QListWidgetItem *item);
    void applySettings();
    void showContextMenu(const QPoint& pos);
    void addBookmarkFromSelection();
    void removeBookmark();
    void changeBookmarkStyle();
    void onQuerySuccess(const WordDefinition& result);
    void onQueryError(const QString& errorString);
    void on_autoScrollButton_toggled(bool checked);
    void on_speedSlider_valueChanged(int value);
    void onAutoScrollTimerTimeout();
    void toggleChapterList();
    // 新加！！文本查找功能

    void findText(const QString& text, bool caseSensitive = false);
    void findNext();
    void findPrevious();
    // 新加！！翻页功能
    void nextPage();
    void previousPage();


private:
    void parseChapters(const QString& content);
    void applyBookmarksToChapter(int chapterIndex);
    // ★★★ 新增独立的双击处理函数 ★★★
    void handleDoubleClick(QMouseEvent *event);

    Ui::ReadingView *ui;
    BookManager *m_bookManager = nullptr;
    BookInfo m_currentBook;
    int m_currentChapterIndex = -1;
    QStringList m_chapterContents;
    QTimer *m_autoScrollTimer;
    DictionaryService* m_dictionaryService;
    TooltipPopup* m_tooltipPopup;
    QTextCharFormat m_bookmarkFormat;




    // 新加！！辅助函数
    void highlightFoundText(const QTextCursor& cursor);
    void scrollToPosition(int position);//

    int m_currentChapter;
    int m_currentPosition;

    // 新加！！
    QLineEdit* m_searchLineEdit;
    QPushButton* m_searchButton;
    QPushButton* m_searchNextButton;
    QPushButton* m_searchPrevButton;
    QString m_lastSearchText;
    bool m_lastSearchCaseSensitive;

    // 新加！！章节列表控制
    QPushButton* m_toggleChapterButton;
    bool m_chapterListVisible;
};
#endif // READINGVIEW_H
