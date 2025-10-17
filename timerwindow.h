//***
#ifndef TIMERWINDOW_H
#define TIMERWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QComboBox>
#include <QPoint>
#include <QMouseEvent>

class TimerWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TimerWindow(QWidget *parent = nullptr);
    ~TimerWindow();

    enum TimerMode {
        NORMAL_TIMER,    // 正计时
        COUNTDOWN_TIMER, // 倒计时
        POMODORO_TIMER   // 番茄钟
    };

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void updateTime();
    void startTimer();
    void pauseTimer();
    void resetTimer();
    void switchMode(int mode);
    void setCountdownTime();
    void switchPomodoroState();

private:
    void setupUI();
    void setupConnections();
    void updateDisplay();
    QString formatTime(int seconds);

    QTimer *m_timer;
    QLabel *m_timeLabel;
    QLabel *m_modeLabel;
    QLabel *m_stateLabel;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_resetButton;
    QPushButton *m_closeButton; // 添加关闭按钮
    QComboBox *m_modeSelector;
    QSpinBox *m_minutesInput;
    QSpinBox *m_secondsInput;

    TimerMode m_currentMode;
    bool m_isRunning;
    int m_seconds;
    int m_countdownSeconds;
    bool m_isWorkState;
    int m_workMinutes;
    int m_breakMinutes;
    int m_pomodoroCount;

    QPoint m_dragPosition;
};

#endif // TIMERWINDOW_H
