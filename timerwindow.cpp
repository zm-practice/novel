#include "timerwindow.h"
#include <QMouseEvent>
#include <QPainter>
#include <QUrl>
#include <QMessageBox>
#include <QScreen>
#include <QGuiApplication>

TimerWindow::TimerWindow(QWidget *parent)
    : QWidget(parent)
    , m_timer(new QTimer(this))
    , m_timeLabel(new QLabel(this))
    , m_modeLabel(new QLabel(this))
    , m_stateLabel(new QLabel(this))
    , m_startButton(new QPushButton("开始", this))
    , m_pauseButton(new QPushButton("暂停", this))
    , m_resetButton(new QPushButton("重置", this))
    , m_closeButton(new QPushButton("关闭", this))
    , m_modeSelector(new QComboBox(this))
    , m_minutesInput(new QSpinBox(this))
    , m_secondsInput(new QSpinBox(this))

    , m_currentMode(NORMAL_TIMER)
    , m_isRunning(false)
    , m_seconds(0)
    , m_countdownSeconds(0)
    , m_isWorkState(true)
    , m_workMinutes(25)
    , m_breakMinutes(5)
    , m_pomodoroCount(0)
{
    // 设置窗口属性：无边框、始终置顶
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    // 设置窗口大小
    resize(300, 200);

    // 设置窗口位置（屏幕右上角）
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    move(screenGeometry.width() - width() - 20, 20);

    // 设置音频播放器
    // m_player->setAudioOutput(m_audioOutput);
    // m_audioOutput->setVolume(1.0);

    setupUI();
    setupConnections();
    updateDisplay();
}

TimerWindow::~TimerWindow()
{
}

void TimerWindow::setupUI()
{
    // 设置字体
    QFont timeFont("Arial", 24, QFont::Bold);
    QFont labelFont("Arial", 12);

    // 设置时间标签
    m_timeLabel->setFont(timeFont);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet("color: #40E0D0;");

    // 设置模式标签
    m_modeLabel->setFont(labelFont);
    m_modeLabel->setAlignment(Qt::AlignCenter);
    m_modeLabel->setStyleSheet("color: white;");
    m_modeLabel->setText("正计时模式");

    // 设置状态标签（用于番茄钟）
    m_stateLabel->setFont(labelFont);
    m_stateLabel->setAlignment(Qt::AlignCenter);
    m_stateLabel->setStyleSheet("color: #FF6347;");
    m_stateLabel->setText("工作时间");
    m_stateLabel->setVisible(false);

    // 设置模式选择器
    m_modeSelector->addItem("正计时", NORMAL_TIMER);
    m_modeSelector->addItem("倒计时", COUNTDOWN_TIMER);
    m_modeSelector->addItem("番茄钟", POMODORO_TIMER);

    // 设置时间输入控件
    m_minutesInput->setRange(0, 99);
    m_minutesInput->setValue(25);
    m_minutesInput->setSuffix(" 分");
    m_minutesInput->setVisible(false);

    m_secondsInput->setRange(0, 59);
    m_secondsInput->setValue(0);
    m_secondsInput->setSuffix(" 秒");
    m_secondsInput->setVisible(false);

    // 设置按钮样式
    QString buttonStyle = "QPushButton {"
                          "    background-color: #40E0D0;"
                          "    color: #303030;"
                          "    border-radius: 4px;"
                          "    padding: 5px;"
                          "}"
                          "QPushButton:hover {"
                          "    background-color: #48D1CC;"
                          "}"
                          "QPushButton:pressed {"
                          "    background-color: #20B2AA;"
                          "}";

    m_startButton->setStyleSheet(buttonStyle);
    m_pauseButton->setStyleSheet(buttonStyle);
    m_resetButton->setStyleSheet(buttonStyle);

    // 设置关闭按钮样式
    m_closeButton->setStyleSheet("QPushButton {"
                                 "    background-color: #FF6347;"
                                 "    color: white;"
                                 "    border-radius: 4px;"
                                 "    padding: 5px;"
                                 "    font-weight: bold;"
                                 "}"
                                 "QPushButton:hover {"
                                 "    background-color: #FF4500;"
                                 "}"
                                 "QPushButton:pressed {"
                                 "    background-color: #B22222;"
                                 "}");
    m_closeButton->setFixedSize(20, 20);
    m_closeButton->setText("×");

    m_pauseButton->setEnabled(false);

    // 创建布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // 创建顶部布局（包含关闭按钮）
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->addStretch();
    topLayout->addWidget(m_closeButton);
    mainLayout->addLayout(topLayout);

    // 添加模式选择器
    mainLayout->addWidget(m_modeSelector);

    // 添加模式标签
    mainLayout->addWidget(m_modeLabel);

    // 添加状态标签（番茄钟使用）
    mainLayout->addWidget(m_stateLabel);

    // 添加时间标签
    mainLayout->addWidget(m_timeLabel);

    // 创建时间输入布局
    QHBoxLayout *timeInputLayout = new QHBoxLayout();
    timeInputLayout->addWidget(m_minutesInput);
    timeInputLayout->addWidget(m_secondsInput);
    mainLayout->addLayout(timeInputLayout);

    // 创建按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_startButton);
    buttonLayout->addWidget(m_pauseButton);
    buttonLayout->addWidget(m_resetButton);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}

void TimerWindow::setupConnections()
{
    // 连接定时器
    connect(m_timer, &QTimer::timeout, this, &TimerWindow::updateTime);

    // 连接按钮
    connect(m_startButton, &QPushButton::clicked, this, &TimerWindow::startTimer);
    connect(m_pauseButton, &QPushButton::clicked, this, &TimerWindow::pauseTimer);
    connect(m_resetButton, &QPushButton::clicked, this, &TimerWindow::resetTimer);
    connect(m_closeButton, &QPushButton::clicked, this, &TimerWindow::close);

    // 连接模式选择器
    connect(m_modeSelector, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &TimerWindow::switchMode);
}

void TimerWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void TimerWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void TimerWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 绘制半透明背景
    QColor bgColor(40, 40, 40, 200); // 深灰色，半透明

    // 如果是番茄钟模式，根据工作/休息状态改变背景色
    if (m_currentMode == POMODORO_TIMER) {
        if (m_isWorkState) {
            bgColor = QColor(48, 48, 48, 200); // 工作状态：深色
        } else {
            bgColor = QColor(0, 100, 0, 200); // 休息状态：绿色
        }
    }

    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(rect(), 10, 10);

    QWidget::paintEvent(event);
}

void TimerWindow::updateTime()
{
    switch (m_currentMode) {
    case NORMAL_TIMER:
        m_seconds++;
        break;

    case COUNTDOWN_TIMER:
        if (m_seconds > 0) {
            m_seconds--;
        } else {
            pauseTimer();
            // playAlarm();
            QMessageBox::information(this, "倒计时结束", "设定的时间已到！");
        }
        break;

    case POMODORO_TIMER:
        if (m_seconds > 0) {
            m_seconds--;
        } else {
            // 切换番茄钟状态
            switchPomodoroState();
        }
        break;
    }

    updateDisplay();
}

void TimerWindow::startTimer()
{
    if (m_currentMode == COUNTDOWN_TIMER && m_seconds == 0) {
        setCountdownTime();
    }

    if (m_seconds > 0 || m_currentMode == NORMAL_TIMER) {
        m_timer->start(1000);
        m_isRunning = true;
        m_startButton->setEnabled(false);
        m_pauseButton->setEnabled(true);

        // 隐藏时间输入控件
        m_minutesInput->setVisible(false);
        m_secondsInput->setVisible(false);
    }
}

void TimerWindow::pauseTimer()
{
    m_timer->stop();
    m_isRunning = false;
    m_startButton->setEnabled(true);
    m_pauseButton->setEnabled(false);
}

void TimerWindow::resetTimer()
{
    pauseTimer();

    switch (m_currentMode) {
    case NORMAL_TIMER:
        m_seconds = 0;
        break;

    case COUNTDOWN_TIMER:
        m_seconds = 0;
        m_minutesInput->setVisible(true);
        m_secondsInput->setVisible(true);
        break;

    case POMODORO_TIMER:
        m_seconds = m_workMinutes * 60;
        m_isWorkState = true;
        m_stateLabel->setText("工作时间");
        m_stateLabel->setStyleSheet("color: #FF6347;"); // 红色
        update(); // 重绘背景
        break;
    }

    updateDisplay();
}

void TimerWindow::switchMode(int index)
{
    pauseTimer();

    m_currentMode = static_cast<TimerMode>(m_modeSelector->itemData(index).toInt());

    // 重置界面
    m_minutesInput->setVisible(false);
    m_secondsInput->setVisible(false);
    m_stateLabel->setVisible(false);

    switch (m_currentMode) {
    case NORMAL_TIMER:
        m_modeLabel->setText("正计时模式");
        m_seconds = 0;
        break;

    case COUNTDOWN_TIMER:
        m_modeLabel->setText("倒计时模式");
        m_seconds = 0;
        m_minutesInput->setVisible(true);
        m_secondsInput->setVisible(true);
        break;

    case POMODORO_TIMER:
        m_modeLabel->setText("番茄钟模式");
        m_stateLabel->setVisible(true);
        m_stateLabel->setText("工作时间");
        m_stateLabel->setStyleSheet("color: #FF6347;"); // 红色
        m_isWorkState = true;
        m_seconds = m_workMinutes * 60;
        m_pomodoroCount = 0;
        break;
    }

    updateDisplay();
    update(); // 重绘背景
}

void TimerWindow::setCountdownTime()
{
    int minutes = m_minutesInput->value();
    int seconds = m_secondsInput->value();

    m_seconds = minutes * 60 + seconds;
    updateDisplay();
}


void TimerWindow::switchPomodoroState()
{
    pauseTimer();
    // playAlarm();

    m_isWorkState = !m_isWorkState;

    if (m_isWorkState) {
        // 切换到工作状态
        m_seconds = m_workMinutes * 60;
        m_stateLabel->setText("工作时间");
        m_stateLabel->setStyleSheet("color: #FF6347;"); // 红色
        QMessageBox::information(this, "休息结束", "休息时间结束，开始工作吧！");
    } else {
        // 切换到休息状态
        m_seconds = m_breakMinutes * 60;
        m_stateLabel->setText("休息时间");
        m_stateLabel->setStyleSheet("color: #32CD32;"); // 绿色
        m_pomodoroCount++;
        QMessageBox::information(this, "工作结束",
                                 QString("已完成 %1 个番茄钟，休息一下吧！").arg(m_pomodoroCount));
    }

    updateDisplay();
    update(); // 重绘背景
    startTimer(); // 自动开始下一个阶段
}

void TimerWindow::updateDisplay()
{
    m_timeLabel->setText(formatTime(m_seconds));
}

QString TimerWindow::formatTime(int seconds)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;

    if (hours > 0) {
        return QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    } else {
        return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'));
    }
}
