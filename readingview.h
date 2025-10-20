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
#include<QBuffer>
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

    void saveProgress();

    // readingview.h 的 private 部分
    // ========== 新增：侧边栏控制槽函数 ==========
    void toggleChapterPanel();      // 切换目录面板
    void toggleSettingsPanel();     // 切换设置面板
    void updateProgressLabel();     // 更新进度标签
private:
    void parseChapters(const QString& content);
    void applyBookmarksToChapter(int chapterIndex);
    void handleDoubleClick(QMouseEvent *event);
    void fetchAccessToken();
    void requestSpeech(const QString &text);
    void clearHighlights();
    void clearSearchHighlights();
    void highlightSentence(int index);
    void unhighlightAll();
    void highlightFoundText(const QTextCursor& cursor);
    void scrollToPosition(int position);
    void applyKeywordHighlighting();
     void switchToChapter(int chapterIndex);

    // ========== UI相关 ==========
    Ui::ReadingView *ui;

    // ========== 书籍管理 ==========
    BookManager *m_bookManager = nullptr;
    BookInfo m_currentBook;
    int m_currentChapterIndex = -1;
    QStringList m_chapterContents;

    // ========== 定时器 ==========
    QTimer *m_autoScrollTimer;              // 自动滚动定时器
    QTimer *m_tokenRefreshTimer = nullptr;  // token刷新定时器
    QTimer *m_autoSaveTimer = nullptr;      // 自动保存定时器

    // ========== 查词功能 ==========
    DictionaryService* m_dictionaryService;
    TooltipPopup* m_tooltipPopup;

    // ========== 书签功能 ==========
    QTextCharFormat m_bookmarkFormat;

    // ========== 搜索功能 ==========

    QPushButton* m_noteButton;
    QPushButton* m_keywordBtn;
    QString m_lastSearchText;
    bool m_lastSearchCaseSensitive;
    int m_currentChapter;
    int m_currentPosition;

    // ========== 章节控制 ==========
    QPushButton* m_toggleChapterButton;
    bool m_chapterListVisible;

    // ========== 关键词高亮 ✅ (只声明一次!) ==========
    KeyWordHighlighter *m_highlighter = nullptr;

    // ========== 听书功能 ==========
    TtsPlayer *m_ttsPlayer;
    QNetworkAccessManager *m_networkManager;
    QMediaPlayer *m_player;
   // QBuffer* m_currentAudioBuffer = nullptr;  // ✅ 音频buffer
    QString m_accessToken;
    QStringList m_sentences;
    int m_currentSentenceIndex;
    int m_totalSentences;
    QAudioOutput* m_audioOutput = nullptr;  // ✅ 添加这个
    bool m_isWaitingForNextSentence = false; // ✅ 添加这个
    QList<QBuffer*> m_audioBuffers;  // ✅ 用于管理所有buffer的生命周期

    QMediaPlayer::PlaybackState m_previousPlaybackState = QMediaPlayer::StoppedState; // <-- 新增
    // ========== 新增：侧边栏状态变量 ==========
    bool m_settingsPanelVisible;    // 设置面板是否可见
    // m_chapterListVisible 已经存在，保持不变

};

#endif // READINGVIEW_H
