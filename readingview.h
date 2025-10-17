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
//#include<QTextToSpeech>
#include "bookdata.h"
#include "dictionaryservice.h"
#include "tooltippopup.h" // ★★★ 直接包含头文件 ★★★
#include "notepadwidget.h"
#include "keywordhighlighter.h"
#include <QCloseEvent>
class QListWidgetItem;
class BookManager;
class QTimer;
class TooltipPopup;
class QMouseEvent;
class QTextCursor;
//听书播放器
//class TtsPlayer; // 前向声明
//class QTextToSpeech; // <--- 在这里添加前向声明
#include <QStringList>
#include <QMediaPlayer>
class TtsPlayer;
class QNetworkAccessManager;
//class QMediaPlayer;
class QNetworkReply;

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
    void onSliderChanged();
    //void toggleChapterList();
    // 新加！！文本查找功能

    void findText(const QString& text, bool caseSensitive = false);
    void findNext();
    void findPrevious();
    // 新加！！翻页功能
    void nextPage();
    void previousPage();

    void notepad();
    void showKeyword();
//听书
    // --- 监听 TtsPlayer 的信号 ---
    void onPlayerPlayRequested(int index);
    void onPlayerPauseRequested();
    void onPlayerResumeRequested();
    void onPlayerStopRequested();
    void onPlayerSentenceChangeRequested(int newIndex);

    // --- 网络请求相关的槽函数 ---
    void onAccessTokenReplyFinished(QNetworkReply *reply);
    void onTtsReplyFinished(QNetworkReply *reply);

    // --- 音频播放器相关的槽函数 ---
    void onPlayerStateChanged(QMediaPlayer::PlaybackState state);
    // ... (其他槽函数)
    void onListenButtonClicked(); // <-- 在这里声明新的槽函数

    // private slots:
    void onPlayerError(QMediaPlayer::Error error, const QString &errorString);

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



//搜索，上下页
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
    QPushButton* m_noteButton;
    QPushButton* m_keywordBtn;

    QString m_lastSearchText;
    bool m_lastSearchCaseSensitive;

    // 新加！！章节列表控制
    QPushButton* m_toggleChapterButton;
    bool m_chapterListVisible;


    KeyWordHighlighter *m_highlighter;

    void saveProgress();

//听书
//private:
  //  TtsPlayer *m_ttsPlayer;
    //QTextToSpeech *m_tts;
   // QStringList m_sentences; // 存储分割好的句子
    //int m_currentSentenceIndex;
    void fetchAccessToken(); // 获取 Access Token 的函数
    void requestSpeech(const QString &text); // 请求语音合成的函数

    void clearHighlights(); // ***【新增】: 用于清除文档中所有搜索高亮***
     TtsPlayer *m_ttsPlayer;
    QNetworkAccessManager *m_networkManager;
    QMediaPlayer *m_player;

    QString m_accessToken;          // 存储获取到的 Access Token
    QStringList m_sentences;        // 分割好的句子列表
    int m_currentSentenceIndex;
    int m_totalSentences;         // <--- 在这里添加这一行，声明总句子数变量

    //阅读中高亮
    void highlightSentence(int index);   // <-- 在 private 部分添加声明
    void unhighlightAll();
};
#endif // READINGVIEW_H
