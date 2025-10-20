// #ifndef TTSPLAYER_H
// #define TTSPLAYER_H

// #include <QWidget>
// #include <QCloseEvent>
// namespace Ui {
// class TtsPlayer;
// }

// class TtsPlayer : public QWidget
// {
//     Q_OBJECT

// public:
//     explicit TtsPlayer(QWidget *parent = nullptr);
//     ~TtsPlayer();
//     // 公共槽函数，用于被 ReadingView 调用来更新UI
// public slots:
//     void onStateChanged(bool isPlaying, bool isPaused);
//     void setTotalSentences(int count);
//     void setCurrentIndex(int index);
//     void updateProgress(int percentage);

// signals:
//     // 发送给 ReadingView 的信号
//     void playRequested(int index);
//     void pauseRequested();
//     void resumeRequested();
//     void stopRequested();
//     void sentenceChangeRequested(int newIndex); // 上一句/下一句
//     void closed(); // 当窗口被关闭时发出信号

// private slots:
//     // 内部UI控件的槽函数
//     void on_playPauseButton_clicked();
//     void on_prevButton_clicked();
//     void on_nextButton_clicked();
//     void on_stopButton_clicked();

// private:
//     void closeEvent(QCloseEvent *event) override; // 重写关闭事件

//     Ui::TtsPlayer *ui;
//     bool m_isPlaying = false;
//     bool m_isPaused = false;
//     int m_currentIndex = 0;
//     int m_totalSentences = 0;
// };

// #endif // TTSPLAYER_H


// ttsplayer.h

#ifndef TTSPLAYER_H
#define TTSPLAYER_H

#include <QWidget>
#include <QCloseEvent>

namespace Ui {
class TtsPlayer;
}

class TtsPlayer : public QWidget
{
    Q_OBJECT

public:
    explicit TtsPlayer(QWidget *parent = nullptr);
    ~TtsPlayer();

    // 设置总句子数
    void setTotalSentences(int total);

    // 设置当前句子索引
    void setCurrentIndex(int index);

    // 更新播放进度 (0-100)
    void updateProgress(int progress);

    // 更新播放状态
    void onStateChanged(bool isPlaying, bool isPaused);

signals:
    // 请求播放指定句子
    void playRequested(int index);

    // 请求暂停
    void pauseRequested();

    // 请求继续播放
    void resumeRequested();

    // 请求停止
    void stopRequested();

    // 请求切换句子
    void sentenceChangeRequested(int newIndex);

    // 窗口关闭
    void closed();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onPlayClicked();
    void onPauseClicked();
    void onPrevClicked();
    void onNextClicked();
    void onCloseClicked();
    void onSliderPressed();
    void onSliderReleased();

private:
    Ui::TtsPlayer *ui;
    int m_totalSentences;
    int m_currentIndex;
    bool m_sliderPressed = false;
};

#endif // TTSPLAYER_H
