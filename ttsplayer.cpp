#include "ttsplayer.h"
#include "ui_ttsplayer.h"
#include <QCloseEvent> // <-- 为了使用 QCloseEvent，需要包含这个头文件

TtsPlayer::TtsPlayer(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TtsPlayer)
{
    ui->setupUi(this);

    //听书播放器的悬浮设置
    // 设置窗口标志，使其成为一个悬浮的工具窗口
    // Qt::Tool: 作为一个工具窗口（通常不会出现在任务栏）
    // Qt::WindowStaysOnTopHint: 保持在所有其他窗口的顶部
    // Qt::WindowCloseButtonHint: 显示关闭按钮
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::WindowCloseButtonHint);
}

TtsPlayer::~TtsPlayer()
{
    delete ui;
}


void TtsPlayer::on_playPauseButton_clicked()
{
    if (!m_isPlaying) {
        // 如果当前是停止或暂停状态，请求播放/恢复
        if (m_isPaused) {
            emit resumeRequested();
        } else {
            emit playRequested(m_currentIndex);
        }
    } else {
        // 如果正在播放，请求暂停
        emit pauseRequested();
    }
}

void TtsPlayer::on_prevButton_clicked()
{
    if (m_currentIndex > 0) {
        emit sentenceChangeRequested(m_currentIndex - 1);
    }
}

void TtsPlayer::on_nextButton_clicked()
{
    if (m_currentIndex < m_totalSentences - 1) {
        emit sentenceChangeRequested(m_currentIndex + 1);
    }
}

void TtsPlayer::on_stopButton_clicked()
{
    emit stopRequested();
}

// 这个函数由 ReadingView 调用，用于更新播放器的UI状态
void TtsPlayer::onStateChanged(bool isPlaying, bool isPaused)
{
    m_isPlaying = isPlaying;
    m_isPaused = isPaused;

    if (m_isPlaying) {
        ui->playPauseButton->setText("暂停");
    } else {
        ui->playPauseButton->setText("播放");
    }
}

// ... 其他函数的实现，例如 setCurrentIndex, setTotalSentences 等
//计数：共有多少句话
void TtsPlayer::setTotalSentences(int count)
{
    m_totalSentences = count;
}

//根据当前句的索引
void TtsPlayer::setCurrentIndex(int index)
{
    if (index < 0 || index >= m_totalSentences) {
        return; // 安全检查，防止索引越界
    }

    m_currentIndex = index;

    // --- 核心逻辑：根据当前索引更新按钮状态 ---

    // 如果是第一句，禁用“上一句”按钮
    ui->prevButton->setEnabled(m_currentIndex > 0);

    // 如果是最后一句，禁用“下一句”按钮
    ui->nextButton->setEnabled(m_currentIndex < m_totalSentences - 1);

}
//进度条的设置
void TtsPlayer::updateProgress(int percentage)
{
    // 安全检查，确保值在0-100之间
    if (percentage < 0) percentage = 0;
    if (percentage > 100) percentage = 100;

    ui->progressBar->setValue(percentage);
}

void TtsPlayer::closeEvent(QCloseEvent *event)
{
    // 发射 closed 信号，通知 ReadingView 我们被关闭了
    emit closed();

    // 调用基类的 closeEvent 来完成标准的关闭流程（例如隐藏窗口）
    QWidget::closeEvent(event);
}
