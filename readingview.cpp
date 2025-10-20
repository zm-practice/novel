
// readingview.cpp
#include "readingview.h"
#include "ui_readingview.h"
#include "settingsmanager.h"
#include "bookmanager.h"
#include "bookmarkstyledialog.h"
#include <QFile>
#include <QTextStream>
#include <QListWidgetItem>
#include <QRegularExpression>
#include <QMenu>
#include <QAction>
#include <QTextCursor>
#include <QImage>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
//自动阅读！！！！
#include <QScrollBar> // 包含滚动条头文件
#include <QTimer>

//在线查词！！！
#include "tooltippopup.h" // 包含头文件
#include <QMouseEvent>
#include <QTextCursor>
#include <QApplication> // 用于获取屏幕信息
#include <QSettings>
#include <QScrollBar>
#include <QMessageBox>
#include <QInputDialog>
#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QDebug>
#include <QFileInfo>
//听书
#include "ttsplayer.h"
//#include <QTextToSpeech>
#include <QAudioOutput>  // ✅ Qt 6 必须包含这个头文件!
#include <QtNetwork>
#include <QRegularExpression>
#include <QMediaPlayer>
#include <QTextCursor>//高亮部分
#include <QTextCharFormat>

#include "keyworddialog.h"
#include "ConfigManager.h"
#include <utility>  // for std::as_const

#include <QFileDialog>
// readingview.cpp

ReadingView::ReadingView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReadingView),
    m_currentPosition(0),
    m_lastSearchCaseSensitive(false),
    m_chapterListVisible(true),
 m_settingsPanelVisible(false)  // 默认隐藏设置面板
{
    ui->setupUi(this);

    // 强制使用布局使目录从顶部开始排列
    if (!ui->chapterPanel->layout()) {
        auto *chapterLayout = new QVBoxLayout(ui->chapterPanel);
        chapterLayout->setContentsMargins(8, 8, 8, 8);
        chapterLayout->setSpacing(8);
        chapterLayout->addWidget(ui->chapterTitle, 0, Qt::AlignLeft);
        chapterLayout->addWidget(ui->chaptersListWidget);
        chapterLayout->setStretch(1, 1);
    }

    // ========== 新增：侧边栏初始化 ==========
    // 初始状态：目录显示，设置面板隐藏
    ui->settingsPanel->hide();

    // 连接折叠按钮
    connect(ui->toggleChapterButton, &QPushButton::clicked, this, &ReadingView::toggleChapterPanel);
    connect(ui->toggleSettingsButton, &QPushButton::clicked, this, &ReadingView::toggleSettingsPanel);

    // 更新进度标签的信号连接
    connect(ui->textBrowser->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &ReadingView::updateProgressLabel);



    auto& settings = SettingsManager::instance();

    // ===================================================================
    // 1. 将所有设置更改的信号，统一连接到一个槽函数来更新UI
    //    【核心修正】使用正确的信号名 `settingsChanged` 并只连接一次！
    // ===================================================================
    connect(&settings, &SettingsManager::settingsChanged, this, [this]() {
        // 当任何设置改变时，这个 lambda 会被调用

        // (A) 调用 applySettings 来更新颜色、字体、背景等
        applySettings();

        // (B) 更新那些 applySettings 不负责的UI元素，比如按钮文本
        if (SettingsManager::instance().isNightMode()) {
            ui->nightModeButton->setText("日间模式");
        } else {
            ui->nightModeButton->setText("夜间模式");
        }
        // (C) 同步控件的状态
        ui->nightModeButton->setChecked(SettingsManager::instance().isNightMode());
        ui->fontComboBox->setCurrentFont(QFont(SettingsManager::instance().fontFamily()));
        ui->fontSizeSpinBox->setValue(SettingsManager::instance().fontSize());
    });


    // ===================================================================
    // 2. 连接UI控件的信号，来修改全局设置 (SettingsManager)
    // ===================================================================
    // 字体变化 -> 修改设置
    connect(ui->fontComboBox, &QFontComboBox::currentFontChanged, this, [&](const QFont& font){
        settings.setFontFamily(font.family());
    });

    // 字号变化 -> 修改设置
    connect(ui->fontSizeSpinBox, &QSpinBox::valueChanged, this, [&](int size){
        settings.setFontSize(size);
    });

    // 夜间模式切换 -> 修改设置
    connect(ui->nightModeButton, &QPushButton::toggled, this, [&](bool checked){
        settings.setNightMode(checked);
    });

    // 背景选择菜单
    QMenu *backgroundMenu = new QMenu(this);
    backgroundMenu->addAction(ui->actionDefault);
    backgroundMenu->addAction(ui->actionPaperYellow);
    backgroundMenu->addAction(ui->actionGreenBean);
    backgroundMenu->addSeparator();
    backgroundMenu->addAction(ui->actionCustomImage);
    ui->backgroundButton->setMenu(backgroundMenu);
    ui->backgroundButton->setPopupMode(QToolButton::InstantPopup);

    connect(ui->actionDefault, &QAction::triggered, this, [&](){ settings.setBackgroundMode(SettingsManager::Default); });
    connect(ui->actionPaperYellow, &QAction::triggered, this, [&](){ settings.setBackgroundMode(SettingsManager::PaperYellow); });
    connect(ui->actionGreenBean, &QAction::triggered, this, [&](){ settings.setBackgroundMode(SettingsManager::GreenBean); });
    connect(ui->actionCustomImage, &QAction::triggered, this, [&](){
        QString imagePath = QFileDialog::getOpenFileName(this, "选择背景图片", "", "Image Files (*.png *.jpg *.jpeg)");
        if (!imagePath.isEmpty()) {
            settings.setCustomImagePath(imagePath);
            settings.setBackgroundMode(SettingsManager::CustomImage);
        }
    });

    // ===================================================================
    // 3. 连接其他功能的信号和槽 (保持不变)
    // ===================================================================
    // 右键菜单
    ui->textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, &ReadingView::showContextMenu);

    // 自动滚动
    m_autoScrollTimer = new QTimer(this);
    connect(m_autoScrollTimer, &QTimer::timeout, this, &ReadingView::onAutoScrollTimerTimeout);
    ui->speedSlider->setRange(10, 500);
    ui->speedSlider->setInvertedAppearance(true);
    ui->speedSlider->setValue(settings.autoScrollSpeed());
    setAutoScrollSpeed(settings.autoScrollSpeed());

    // 在线查词
    m_tooltipPopup = new TooltipPopup(this);
    m_dictionaryService = new DictionaryService(this);
    connect(m_dictionaryService, &DictionaryService::querySuccess, this, &ReadingView::onQuerySuccess);
    connect(m_dictionaryService, &DictionaryService::queryError, this, &ReadingView::onQueryError);
    ui->textBrowser->viewport()->installEventFilter(this);

    // 关键词高亮
    m_highlighter = new KeyWordHighlighter(ui->textBrowser, this);
    connect(&KeywordManager::instance(), &KeywordManager::rulesChanged, m_highlighter, &KeyWordHighlighter::highlight);

    // 听书功能
    m_networkManager = new QNetworkAccessManager(this);
    m_player = new QMediaPlayer(this);
    m_audioOutput = new QAudioOutput(this);
    m_player->setAudioOutput(m_audioOutput);
    m_audioOutput->setVolume(1.0);
    connect(m_player, &QMediaPlayer::playbackStateChanged, this, &ReadingView::onPlayerStateChanged);
    connect(m_player, &QMediaPlayer::errorOccurred, this, &ReadingView::onPlayerError);

    m_ttsPlayer = new TtsPlayer(this);
    m_ttsPlayer->hide();
    connect(m_ttsPlayer, &TtsPlayer::playRequested, this, &ReadingView::onPlayerPlayRequested);
    connect(m_ttsPlayer, &TtsPlayer::pauseRequested, this, &ReadingView::onPlayerPauseRequested);
    connect(m_ttsPlayer, &TtsPlayer::resumeRequested, this, &ReadingView::onPlayerResumeRequested);
    connect(m_ttsPlayer, &TtsPlayer::stopRequested, this, &ReadingView::onPlayerStopRequested);
    connect(m_ttsPlayer, &TtsPlayer::sentenceChangeRequested, this, &ReadingView::onPlayerSentenceChangeRequested);
    connect(m_ttsPlayer, &TtsPlayer::closed, this, &ReadingView::onPlayerStopRequested);
    connect(ui->listenButton, &QPushButton::clicked, this, &ReadingView::onListenButtonClicked);

    // Token管理
    m_tokenRefreshTimer = new QTimer(this);
    connect(m_tokenRefreshTimer, &QTimer::timeout, this, &ReadingView::fetchAccessToken);
    m_tokenRefreshTimer->start(24 * 60 * 60 * 1000);
    fetchAccessToken();

    // 自动保存
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &ReadingView::saveProgress);
    m_autoSaveTimer->start(30000);

    // 翻页按钮
    connect(ui->prevPageButton, &QPushButton::clicked, this, &ReadingView::previousPage);
    connect(ui->nextPageButton, &QPushButton::clicked, this, &ReadingView::nextPage);

    // 搜索功能
    connect(ui->searchButton, &QPushButton::clicked, this, [this](){ findText(ui->searchLineEdit->text(), false); });
    connect(ui->searchNextButton, &QPushButton::clicked, this, &ReadingView::findNext);
    connect(ui->searchPrevButton, &QPushButton::clicked, this, &ReadingView::findPrevious);
    connect(ui->searchLineEdit, &QLineEdit::returnPressed, this, [this](){ findText(ui->searchLineEdit->text(), false); });

    // 其他按钮
    connect(ui->noteButton, &QPushButton::clicked, this, &ReadingView::notepad);
    connect(ui->keyButton, &QPushButton::clicked, this, &ReadingView::showKeyword);


    // ===================================================================
    // 4. 初始化UI状态
    //    在所有连接都设置好之后，手动发一个信号来确保UI是最新状态
    // ===================================================================
    emit settings.settingsChanged(); // 触发一次信号，让上面的 connect 生效，从而初始化所有UI
}
// 2. 修复析构函数 - 添加保存进度 ✅
ReadingView::~ReadingView()
{
    saveProgress(); // 关闭时保存进度 ✅

    // // 清理音频buffer
    // if (m_currentAudioBuffer) {
    //     m_currentAudioBuffer->deleteLater();
    //     m_currentAudioBuffer = nullptr;
    // }

    delete ui;
}
// void ReadingView::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
// {
//     qDebug() << "QMediaPlayer Error:" << error << errorString;
// }



// ========== 新增：侧边栏控制函数 ==========

void ReadingView::toggleChapterPanel()
{
    m_chapterListVisible = !m_chapterListVisible;

    if (m_chapterListVisible) {
        ui->chapterPanel->show();
        ui->toggleChapterButton->setText("☰");
        ui->toggleChapterButton->setToolTip("隐藏目录");
    } else {
        ui->chapterPanel->hide();
        ui->toggleChapterButton->setText("☰");
        ui->toggleChapterButton->setToolTip("显示目录");
    }
}

void ReadingView::toggleSettingsPanel()
{
    m_settingsPanelVisible = !m_settingsPanelVisible;

    if (m_settingsPanelVisible) {
        ui->settingsPanel->show();
        ui->toggleSettingsButton->setText("✕");
        ui->toggleSettingsButton->setToolTip("关闭设置");
    } else {
        ui->settingsPanel->hide();
        ui->toggleSettingsButton->setText("⚙");
        ui->toggleSettingsButton->setToolTip("打开设置");
    }
}

void ReadingView::updateProgressLabel()
{
    double hCur = ui->textBrowser->verticalScrollBar()->value();
    double hMax = ui->textBrowser->verticalScrollBar()->maximum();

    int progress = 0;
    if (hMax > 0) {
        progress = static_cast<int>((hCur * 100.0) / hMax);
    }

    ui->progressLabel->setText(QString("阅读进度: %1%").arg(progress));
}

// 修改现有的 onSliderChanged 函数，改为调用新函数
void ReadingView::onSliderChanged()
{
    updateProgressLabel();
}

// ★★★ 关键修复4: 使用事件过滤器捕获双击事件 ★★★
bool ReadingView::eventFilter(QObject *obj, QEvent *event)
{
    // 只处理 textBrowser 的 viewport 上的鼠标双击事件
    if (obj == ui->textBrowser->viewport() &&
        event->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        handleDoubleClick(mouseEvent);
        return true; // 事件已处理,阻止传播
    }
    return QWidget::eventFilter(obj, event);
}

// ★★★ 关键修复5: 独立的双击处理函数 ★★★
void ReadingView::handleDoubleClick(QMouseEvent *event)
{
    qDebug() << "双击事件触发"; // 调试信息

    // 获取点击位置的光标
    QTextCursor cursor = ui->textBrowser->cursorForPosition(event->pos());
    int clickPos = cursor.position();

    QString selectedText;

    // ★★★ 核心修复: 智能选择单字或单词 ★★★
    // 获取光标位置的字符
    QChar charAtPos = ui->textBrowser->document()->characterAt(clickPos);

    qDebug() << "点击位置的字符:" << charAtPos << "Unicode:" << QString::number(charAtPos.unicode(), 16);

    // 判断是否为中文字符 (CJK统一汉字范围)
    if (charAtPos.unicode() >= 0x4E00 && charAtPos.unicode() <= 0x9FFF) {
        // 是中文 - 只选择单个字符
        cursor.setPosition(clickPos);
        cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
        selectedText = cursor.selectedText();
        qDebug() << "识别为中文,选中单字:" << selectedText;
    }
    else if (charAtPos.isLetter()) {
        // 是英文字母 - 选择整个单词
        cursor.select(QTextCursor::WordUnderCursor);
        selectedText = cursor.selectedText();
        qDebug() << "识别为英文,选中单词:" << selectedText;
    }
    else {
        qDebug() << "既不是中文也不是英文字母";
        return;
    }

    selectedText = selectedText.trimmed();

    if (selectedText.isEmpty()) {
        qDebug() << "未选中任何文本";
        return;
    }

    // 显示"查询中..."提示
    WordDefinition pendingDef;
    pendingDef.displayWord = selectedText;
    pendingDef.phonetic = "查询中...";
    pendingDef.definition = "";

    // 计算弹窗位置(在鼠标下方偏移一点)
    QPoint globalPos = ui->textBrowser->mapToGlobal(event->pos());
    m_tooltipPopup->showDefinition(pendingDef, globalPos);

    // 发起查询
    m_dictionaryService->query(selectedText);
    qDebug() << "已发起查询请求:" << selectedText;
}


// --- 实现响应槽函数 ---
void ReadingView::onQuerySuccess(const WordDefinition &result)
{
    qDebug() << "查询成功:" << result.displayWord;
    if (m_tooltipPopup) {
        m_tooltipPopup->showDefinition(result, QCursor::pos());
    }
}

// 响应查询失败
void ReadingView::onQueryError(const QString &errorString)
{
    qDebug() << "查询失败:" << errorString;
    if (m_tooltipPopup) {
        WordDefinition errorResult;
        errorResult.displayWord = "查询失败";
        errorResult.phonetic = "";
        errorResult.definition = errorString;
        m_tooltipPopup->showDefinition(errorResult, QCursor::pos());
    }
}

//自动阅读！！！
// ★★★ 实现公共控制接口 ★★★
void ReadingView::startAutoScroll()
{
    if (!m_autoScrollTimer->isActive()) {
        m_autoScrollTimer->start();
    }
}

void ReadingView::stopAutoScroll()
{
    m_autoScrollTimer->stop();
}

void ReadingView::setAutoScrollSpeed(int interval)
{
    // 设置定时器的触发间隔（毫秒）
    m_autoScrollTimer->setInterval(interval);
}

// ★★★ 实现定时器槽函数：这是滚动的核心 ★★★
void ReadingView::onAutoScrollTimerTimeout()
{
    QScrollBar *scrollBar = ui->textBrowser->verticalScrollBar();
    // 每次滚动 1 个像素
    scrollBar->setValue(scrollBar->value() + 1);

    // 如果已经滚动到底部，则停止滚动
    if (scrollBar->value() >= scrollBar->maximum()) {
        stopAutoScroll();
        // (可选) 发射一个信号，告诉 ReaderWindow 滚动结束了
    }
}


// ★★★ 实现新增的槽函数 ★★★

// ========== 修改自动阅读按钮文本 ==========

void ReadingView::on_autoScrollButton_toggled(bool checked)
{
    if (checked) {
        startAutoScroll();
        ui->autoScrollButton->setText("⏸ 停止阅读");
    } else {
        stopAutoScroll();
        ui->autoScrollButton->setText("▶ 自动阅读");
    }
}
void ReadingView::on_speedSlider_valueChanged(int value)
{
    SettingsManager::instance().setAutoScrollSpeed(value); // 更新全局设置
    setAutoScrollSpeed(value); // 立即更新当前视图的速度
}
//上面截至到


// 3. 修复loadBook - 添加恢复进度 ✅
void ReadingView::loadBook(const BookInfo &book)
{
    m_currentBook = book;
    if (m_currentBook.id.isEmpty()) {
        m_currentBook.id = m_currentBook.filePath;
    }

    QFile file(book.filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->textBrowser->setText("错误：无法打开文件 " + book.filePath);
        return;
    }

    QTextStream in(&file);
    QString content = in.readAll();
    file.close();

    parseChapters(content);

    // ✅ 恢复上次阅读进度
    double lastPage = ConfigManager::instance().lastPage(m_currentBook.filePath);
    if (lastPage > 0) {
        QTimer::singleShot(100, this, [this, lastPage]() {
            ui->textBrowser->verticalScrollBar()->setValue(static_cast<int>(lastPage));
        });
    }

    QApplication::processEvents();
}

void ReadingView::parseChapters(const QString &content)
{
    ui->chaptersListWidget->clear();
    m_chapterContents.clear();

    // 使用 Qt 6 的 QRegularExpression
    QStringList chapters = content.split(QRegularExpression("第[〇一二三四五六七八九十百千万\\d]+章"));

    if (chapters.size() <= 1) { // 如果没有匹配到章节分隔符
        ui->chaptersListWidget->addItem("全文");
        m_chapterContents.append(content);
    } else {
        // 处理可能存在的序言部分（在第一个“第...章”之前的内容）
        if(!chapters.at(0).trimmed().isEmpty()) {
            ui->chaptersListWidget->addItem("序章");
            m_chapterContents.append(chapters.at(0));
        }

        // 提取所有的章节标题，例如 "第一章", "第二章"...
        QStringList chapterTitles;
        QRegularExpression rx("第[〇一二三四五六七八九十百千万\\d]+章");
        QRegularExpressionMatchIterator i = rx.globalMatch(content);
        while (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            chapterTitles.append(match.captured(0));
        }

        // 遍历分割后的章节内容
        for (int i = 1; i < chapters.size(); ++i) {
            // 将提取出的章节标题与内容一一对应
            if(i - 1 < chapterTitles.size()){
                ui->chaptersListWidget->addItem(chapterTitles.at(i-1));
            } else {
                // 如果内容比标题多，则使用通用标题
                ui->chaptersListWidget->addItem(QString("章节 %1").arg(i));
            }
            // `split` 之后，`chapters` 列表的第一项是序言，所以内容从索引1开始
            m_chapterContents.append(chapters.at(i));
        }
    }
}
void ReadingView::setBookManager(BookManager *manager)
{
     m_bookManager = manager;
}



// 4. 修复章节切换 - 添加保存进度和关键词高亮 ✅
void ReadingView::on_chaptersListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;

    // // 先保存当前进度 ✅
    // saveProgress();

    int index = ui->chaptersListWidget->row(item);
    if (index >= 0 && index < m_chapterContents.size()) {
        m_currentChapterIndex = index;
        ui->textBrowser->setText(m_chapterContents.at(index));

        ui->textBrowser->verticalScrollBar()->setValue(ConfigManager::instance().lastPage(m_currentBook.filePath));
        ui->label_3->setText(QString::number(ConfigManager::instance().lastPage(m_currentBook.filePath) * 100.0 / ConfigManager::instance().totalPage(m_currentBook.filePath)) + "%");

        // 阅读进度
        connect(ui->textBrowser->verticalScrollBar(), &QScrollBar::valueChanged, this, &ReadingView::onSliderChanged);


        applyBookmarksToChapter(index);

        // 应用关键词高亮 ✅
        if (m_highlighter) {
            m_highlighter->highlight();
        }

        bool hasText = !ui->textBrowser->toPlainText().isEmpty();
        ui->listenButton->setEnabled(hasText);
    }
}

// readingview.cpp

//     void ReadingView::applySettings()
//     {
//         // 获取全局设置的单例对象
//         auto& settings = SettingsManager::instance();

//         // // 1. --- 设置字体 ---
//         // // 创建一个 QFont 对象，并从 SettingsManager 获取字体家族和大小
//         // QFont font(settings.fontFamily());
//         // font.setPointSize(settings.fontSize());

//         // // 将这个字体同时应用到正文显示区和章节列表
//         // ui->textBrowser->setFont(font);
//         // ui->chaptersListWidget->setFont(font);

//         // 2. --- 设置背景和前景颜色 ---
//         QString browserStyle, listStyle;

//         // 判断当前是否是自定义图片背景模式
//         if (settings.backgroundMode() == SettingsManager::CustomImage && !settings.customImagePath().isEmpty())
//         {
//             // --- 图片背景模式 ---
//             QString imagePath = settings.customImagePath();
//             imagePath.replace("\\", "/"); // 确保路径格式正确

//             // 为 QTextBrowser 准备样式表
//             browserStyle = QString(
//                                "QTextBrowser {"
//                                "  border-image: url(%1) 0 0 0 0 stretch stretch;" // 设置背景图片
//                                "  background-color: transparent;"              // 背景色必须设为透明
//                                "  color: %2;"                                   // 从全局设置获取文字颜色
//                                "  border: none;"
//                                "  font-family: '%3';"         // 【新增】字体家族
//                                "  font-size: %4pt;"           // 【新增】字体大小 (注意单位是 pt)                               // 去掉边框
//                                "}"
//                                ).arg(imagePath)
//                                .arg(settings.textColor().name())
//                                .arg(settings.fontFamily())       // 【新增】字体家族参数
//                                .arg(settings.fontSize());       // 【新增】字体大小参数;

//             // 为 QListWidget 准备样式表
//             listStyle = QString(
//                             "QListWidget {"
//                             "  border-image: url(%1) 0 0 0 0 stretch stretch;"
//                             "  background-color: transparent;"
//                             "  color: %2;"
//                             "  border: none;"
//                             "  font-family: '%3';"             // 【新增】
//                             "  font-size: %4pt;"
//                             "}"
//                             // 保持选中项的高亮样式
//                             "QListWidget::item:selected { background-color: #4a6984; color: white; }"
//                             ).arg(imagePath)
//                             .arg(settings.textColor().name())
//                             .arg(settings.fontFamily())           // 【新增】
//                             .arg(settings.fontSize()); ;
//         }
//         else
//         {
//             // --- 纯色背景模式 ---
//             // 为 QTextBrowser 准备样式表
//             browserStyle = QString(
//                                "QTextBrowser { "
//                                "background-color: %1;"
//                                 " color: %2;"
//                                 "border: none; "
//                                 "font-family: '%3';"         // 【新增】
//                            "  font-size: %4pt;"           // 【新增】
//                                "}")
//                                .arg(settings.backgroundColor().name(QColor::HexRgb))
//                                .arg(settings.textColor().name(QColor::HexRgb))
//                                 .arg(settings.fontFamily())       // 【新增】
//                                 .arg(settings.fontSize());       // 【新增】

//             // 为 QListWidget 准备样式表
//             listStyle = QString(
//                             "QListWidget  {"
//                         "  background-color: %1;"
//                         "  color: %2;"
//                         "  border: none;"
//                         "  font-family: '%3';"             // 【新增】
//                         "  font-size: %4pt;"               // 【新增】
//                         "}"
//                             "QListWidget::item:selected { background-color: #4a6984; color: white; }"
//                             ).arg(settings.backgroundColor().name(QColor::HexRgb))
//                             .arg(settings.textColor().name(QColor::HexRgb))
//                             .arg(settings.fontFamily())           // 【新增】
//                             .arg(settings.fontSize());           // 【新增】;
//         }

//         // 3. --- 将生成的样式表应用到控件 ---
//         ui->textBrowser->setStyleSheet(browserStyle);
//     ui->chaptersListWidget->setStyleSheet(listStyle);
// }

    // readingview.cpp 中

    void ReadingView::applySettings()
    {
        auto& settings = SettingsManager::instance();

        // --- 步骤 1: 优先处理自定义图片背景 ---
        if (settings.backgroundMode() == SettingsManager::CustomImage && !settings.customImagePath().isEmpty())
        {
            QString imagePath = settings.customImagePath().replace("\\", "/");
            QString imageStyle = QString(
                                     "border-image: url(%1) 0 0 0 0 stretch stretch;"
                                     "background-color: transparent;"
                                     "color: %2;" // 自定义图片模式下的文字颜色，也可以从settings里读取
                                     "border: none;"
                                     "font-family: '%3';"
                                     "font-size: %4pt;"
                                     ).arg(imagePath)
                                     .arg(settings.textColor().name()) // 建议为自定义图片模式也设置一个文字颜色
                                     .arg(settings.fontFamily())
                                     .arg(settings.fontSize());

            ui->textBrowser->setStyleSheet(imageStyle);
            ui->chaptersListWidget->setStyleSheet(imageStyle);

            return; // 处理完毕，直接返回
        }

        // --- 步骤 2: 如果不是图片背景，则根据模式设置颜色 ---
        QString bgColor, textColor;
        QString fontFamily = settings.fontFamily();
        int fontSize = settings.fontSize();

        // 【核心逻辑修正】
        if (settings.isNightMode()) {
            // 夜间模式拥有最高优先级
            bgColor = "#2C3E50";   // 深邃炭黑蓝
            textColor = "#BDC3C7"; // 柔和浅灰
        } else {
            // 如果是日间模式，则根据选择的具体背景来决定颜色
            switch (settings.backgroundMode()) {
            case SettingsManager::PaperYellow:
                bgColor = "#F5F5DC";   // 米黄
                textColor = "#5B4636"; // 搭配的深棕色文字，更护眼
                break;
            case SettingsManager::GreenBean:
                bgColor = "#C7EDCC";   // 豆绿
                textColor = "#3E5841"; // 搭配的深绿色文字
                break;
            case SettingsManager::Default:
            default: // 默认情况
                bgColor = "#FFFFFF";   // 默认使用纯白背景
                textColor = "#333333"; // 标准深灰色文字
                break;
            }
        }

        // --- 步骤 3: 构建最终的QSS样式并应用 ---
        QString style = QString(
                            "background-color: %1;"
                            "color: %2;"
                            "border: none;" // 去掉全局QSS的边框，让阅读区更沉浸
                            "font-family: '%3';"
                            "font-size: %4pt;"
                            ).arg(bgColor)
                            .arg(textColor)
                            .arg(fontFamily)
                            .arg(fontSize);

        ui->textBrowser->setStyleSheet(style);
        ui->chaptersListWidget->setStyleSheet(style);

        // 为侧边栏面板设置样式
        QString panelStyle;
        if (settings.isNightMode()) {
            panelStyle = "QWidget#chapterPanel, QWidget#settingsPanel { "
                         "background-color: #2C3E50; "
                         "border-right: 1px solid #34495E; "
                         "}";
        } else {
            panelStyle = "QWidget#chapterPanel, QWidget#settingsPanel { "
                         "background-color: #F5F5F5; "
                         "border-right: 1px solid #E0E0E0; "
                         "}";
        }

        ui->chapterPanel->setStyleSheet(panelStyle);
        ui->settingsPanel->setStyleSheet(panelStyle.replace("border-right", "border-left"));

        // 顶部工具栏样式
        QString toolbarStyle;
        if (settings.isNightMode()) {
            toolbarStyle = "QWidget#topToolBar { "
                           "background-color: #34495E; "
                           "border-bottom: 1px solid #2C3E50; "
                           "}";
        } else {
            toolbarStyle = "QWidget#topToolBar { "
                           "background-color: #FFFFFF; "
                           "border-bottom: 1px solid #E0E0E0; "
                           "}";
        }
        ui->topToolBar->setStyleSheet(toolbarStyle);
    }



void ReadingView::showContextMenu(const QPoint &pos)
{
    if (!m_bookManager) return;
    QMenu contextMenu(this);
    QTextCursor cursor = ui->textBrowser->cursorForPosition(pos);

    QTextCharFormat fmt = cursor.charFormat();
    bool inBookmark = !fmt.property(QTextFormat::UserProperty).isNull();

    if (inBookmark) {
        int bookmarkDbId = fmt.property(QTextFormat::UserProperty).toInt();
        QAction *removeAction = new QAction("取消书签", this);
        removeAction->setData(bookmarkDbId);
        connect(removeAction, &QAction::triggered, this, &ReadingView::removeBookmark);
        contextMenu.addAction(removeAction);

        QAction *editAction = new QAction("编辑样式...", this);
        editAction->setData(bookmarkDbId);
        connect(editAction, &QAction::triggered, this, &ReadingView::changeBookmarkStyle);
        contextMenu.addAction(editAction);
    } else if (ui->textBrowser->textCursor().hasSelection()) {
        QAction *addAction = new QAction("添加书签", this);
        connect(addAction, &QAction::triggered, this, &ReadingView::addBookmarkFromSelection);
        contextMenu.addAction(addAction);
    }
    contextMenu.exec(ui->textBrowser->mapToGlobal(pos));
}

void ReadingView::addBookmarkFromSelection()
{
    if (!m_bookManager) return;
    QTextCursor cursor = ui->textBrowser->textCursor();
    if (!cursor.hasSelection()) return;

    // 保存当前滚动位置
    int scrollPosition = ui->textBrowser->verticalScrollBar()->value();

    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);
    BookmarkInfo newBookmark;
    newBookmark.chapterIndex = m_currentChapterIndex;
    newBookmark.startPos = cursor.selectionStart();
    newBookmark.endPos = cursor.selectionEnd();
    newBookmark.iconPath = SettingsManager::instance().bookmarkIconPath();
    newBookmark.backgroundColor = QColor(255, 255, 0, 50);
    newBookmark.underlineStyle = QTextCharFormat::DashUnderline;
    newBookmark.underlineColor = Qt::gray;
    newBookmark.type = BookmarkInfo::Bookmark; // 明确设置为书签类型
    newBookmark.content = cursor.selectedText(); // 保存选中的文本内容

    bookmarks.append(newBookmark);
    m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

    // 重新应用书签，但不切换章节
    applyBookmarksToChapter(m_currentChapterIndex);
    
    // 恢复滚动位置
    QTimer::singleShot(0, this, [this, scrollPosition]() {
        ui->textBrowser->verticalScrollBar()->setValue(scrollPosition);
    });
}

void ReadingView::removeBookmark()
{
    if (!m_bookManager) return;
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    // 保存当前滚动位置
    int scrollPosition = ui->textBrowser->verticalScrollBar()->value();

    int dbId = action->data().toInt();
    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

    // 我们需要通过一个唯一ID来找到书签，而不是位置
    // (这是一个更高级的实现，暂时我们还是用索引)
    // 假设 dbId 就是索引
    if(dbId >= 0 && dbId < bookmarks.size()){
        bookmarks.removeAt(dbId);
        m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);
        
        // 清除所有格式并重新应用书签
        ui->textBrowser->document()->clearUndoRedoStacks();
        QTextCursor cursor = ui->textBrowser->textCursor();
        cursor.select(QTextCursor::Document);
        QTextCharFormat plainFormat;
        cursor.setCharFormat(plainFormat);
        cursor.clearSelection();
        
        // 重新应用书签，但不切换章节
        applyBookmarksToChapter(m_currentChapterIndex);
        
        // 恢复滚动位置
        QTimer::singleShot(0, this, [this, scrollPosition]() {
            ui->textBrowser->verticalScrollBar()->setValue(scrollPosition);
        });
    }
}

void ReadingView::changeBookmarkStyle()
{
    if (!m_bookManager) return; // 安全检查

    // 1. 获取触发此槽的 QAction 对象
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    // 2. 从 Action 的用户数据中恢复我们之前存入的【书签索引】
    //    这里我们假设 UserProperty 存的是书签在列表中的索引
    int targetIndex = action->data().toInt();

    // 3. 从 BookManager 获取完整的书签列表
    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

    // 4. 再次进行安全检查，确保索引有效
    if (targetIndex < 0 || targetIndex >= bookmarks.size()) {
        return;
    }

    // 5. 创建并显示样式编辑对话框
    //    关键：把当前书签 (bookmarks[targetIndex]) 的样式信息传递给对话框的构造函数
    BookmarkStyleDialog dialog(bookmarks[targetIndex], this);

    // 6. 以模态方式运行对话框，并检查用户的操作结果
    if (dialog.exec() == QDialog::Accepted) {
        // 7. 如果用户点击了 "OK"...

        // a. 从对话框获取用户选择的、包含所有修改的新样式
        BookmarkInfo newStyle = dialog.getSelectedStyle();

        // b. 用新样式【替换】掉列表中旧的书签信息
        bookmarks[targetIndex] = newStyle;

        // c. 将整个【更新后】的列表交还给 BookManager 进行保存
        m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

        // d. 立即、完整地刷新视图，以显示最新的样式
        on_chaptersListWidget_itemClicked(ui->chaptersListWidget->currentItem());
    }
}

void ReadingView::applyBookmarksToChapter(int chapterIndex)
{
    if (!m_bookManager) return;
    QTextCursor cursor(ui->textBrowser->document());
    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

    for (int i = 0; i < bookmarks.size(); ++i) {
        const auto& bookmark = bookmarks[i];
        if (bookmark.chapterIndex == chapterIndex) {
            // 检查该位置是否已经有搜索高亮
            cursor.setPosition(bookmark.startPos);
            QTextCharFormat existingFormat = cursor.charFormat();
            bool hasSearchHighlight = existingFormat.property(QTextFormat::UserProperty + 1).toString() == "search_highlight";
            
            QTextCharFormat format;
            if (!hasSearchHighlight) {
                // 只有在没有搜索高亮时才应用书签格式
                format.setBackground(bookmark.backgroundColor);
                format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(bookmark.underlineStyle));
                format.setUnderlineColor(bookmark.underlineColor);
            } else {
                // 如果有搜索高亮，只应用下划线样式，保留搜索高亮的背景色
                format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(bookmark.underlineStyle));
                format.setUnderlineColor(bookmark.underlineColor);
            }
            // ★★★ 关键：用 UserProperty 存一个唯一ID (这里用索引代替) ★★★
            format.setProperty(QTextFormat::UserProperty, i);

            cursor.setPosition(bookmark.startPos, QTextCursor::MoveAnchor);
            cursor.setPosition(bookmark.endPos, QTextCursor::KeepAnchor);
            cursor.mergeCharFormat(format);

            if (!bookmark.iconPath.isEmpty()) {
                cursor.setPosition(bookmark.startPos);
                cursor.insertImage(QImage(bookmark.iconPath).scaled(16, 16, Qt::KeepAspectRatio));
            }
        }
    }
    cursor.clearSelection();
}


// 1. findText - 初始搜索
void ReadingView::findText(const QString& text, bool caseSensitive)
{
    if (text.isEmpty()) return;

    // 保存搜索参数
    m_lastSearchText = text;
    m_lastSearchCaseSensitive = caseSensitive;

    // 先清除所有搜索高亮
    clearSearchHighlights();

    QTextDocument::FindFlags flags;
    if (caseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    // 在当前章节从头开始搜索
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.setPosition(0);
    ui->textBrowser->setTextCursor(cursor);

    bool found = ui->textBrowser->find(text, flags);
    if (found) {
        highlightFoundText(ui->textBrowser->textCursor());
        return;
    }

    // 如果当前章节未找到，搜索其他章节
    int originalChapter = m_currentChapterIndex;

    // 从下一章开始搜索
    for (int i = 1; i < m_chapterContents.size(); i++) {
        int chapterToSearch = (originalChapter + i) % m_chapterContents.size();

        if (m_chapterContents[chapterToSearch].contains(text,
                                                        caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            // 切换章节
            switchToChapter(chapterToSearch);

            // 从头搜索
            cursor = ui->textBrowser->textCursor();
            cursor.setPosition(0);
            ui->textBrowser->setTextCursor(cursor);

            if (ui->textBrowser->find(text, flags)) {
                highlightFoundText(ui->textBrowser->textCursor());
                return;
            }
        }
    }

    // 所有章节都未找到（静默返回）
    return;
}

    // 7. clearHighlights - 清除所有高亮
    void ReadingView::clearHighlights()
    {
        // 只清除搜索高亮
        clearSearchHighlights();

        // 重新应用书签和关键词高亮
        if (m_currentChapterIndex >= 0) {
            applyBookmarksToChapter(m_currentChapterIndex);
        }

        if (m_highlighter) {
            QTimer::singleShot(0, m_highlighter, &KeyWordHighlighter::highlight);
        }
    }

    // readingview.cpp

    void ReadingView::findNext()
    {
        if (m_lastSearchText.isEmpty()) return;

        QTextDocument::FindFlags flags;
        if (m_lastSearchCaseSensitive) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        // 向前查找；未找到则静默回到文档开头
        if (!ui->textBrowser->find(m_lastSearchText, flags)) {
            QTextCursor c = ui->textBrowser->textCursor();
            c.movePosition(QTextCursor::Start);
            ui->textBrowser->setTextCursor(c);
            ui->textBrowser->find(m_lastSearchText, flags);
        }

        // 统一高亮当前选中结果
        highlightFoundText(ui->textBrowser->textCursor());
    }
    // 3. findPrevious - 查找上一个（完全重写，修复无响应问题）
    // readingview.cpp

    void ReadingView::findPrevious()
    {
        if (m_lastSearchText.isEmpty()) return;

        QTextDocument::FindFlags flags = QTextDocument::FindBackward;
        if (m_lastSearchCaseSensitive) {
            flags |= QTextDocument::FindCaseSensitively;
        }

        // 向后查找；未找到则静默回到文档末尾
        if (!ui->textBrowser->find(m_lastSearchText, flags)) {
            QTextCursor c = ui->textBrowser->textCursor();
            c.movePosition(QTextCursor::End);
            ui->textBrowser->setTextCursor(c);
            ui->textBrowser->find(m_lastSearchText, flags);
        }

        // 统一高亮当前选中结果
        highlightFoundText(ui->textBrowser->textCursor());
    }
    // 4. switchToChapter - 新增辅助函数，安全地切换章节
    void ReadingView::switchToChapter(int chapterIndex)
    {
        if (chapterIndex < 0 || chapterIndex >= m_chapterContents.size()) {
            return;
        }

        m_currentChapterIndex = chapterIndex;
        m_currentChapter = chapterIndex; // 确保两个变量同步更新

        // 更新章节列表选中状态（不触发信号）
        ui->chaptersListWidget->blockSignals(true);
        ui->chaptersListWidget->setCurrentRow(chapterIndex);
        ui->chaptersListWidget->blockSignals(false);

        // 完全清空并重新设置文本
        ui->textBrowser->clear();
        ui->textBrowser->setPlainText(m_chapterContents.at(chapterIndex));

        // 重新应用书签格式
        applyBookmarksToChapter(chapterIndex);

        // 重新应用关键词高亮
        if (m_highlighter) {
            m_highlighter->highlight();
        }
    }
void ReadingView::applyKeywordHighlighting()
{
    if (m_highlighter) {
        m_highlighter->highlight();
    }
}

// 5. highlightFoundText - 高亮找到的文本（使用 ExtraSelections 叠加，不修改正文格式）
void ReadingView::highlightFoundText(const QTextCursor& cursor)
{
    if (!cursor.hasSelection()) {
        return;
    }

    // 验证选中的文本是否匹配搜索内容
    QString selectedText = cursor.selectedText();
    bool isMatch = m_lastSearchCaseSensitive
                       ? (selectedText == m_lastSearchText)
                       : (selectedText.compare(m_lastSearchText, Qt::CaseInsensitive) == 0);
    if (!isMatch) {
        return;
    }

    // 构造叠加高亮选区（不更改文档的字符格式）
    QTextEdit::ExtraSelection sel;
    sel.cursor = cursor;
    sel.format.setBackground(QColor("#FFE58F"));

    QList<QTextEdit::ExtraSelection> selections;
    selections << sel; // 只高亮当前匹配
    ui->textBrowser->setExtraSelections(selections);

    // 保持光标并确保可见
    ui->textBrowser->setTextCursor(cursor);
    ui->textBrowser->ensureCursorVisible();
}
// 翻页功能
void ReadingView::nextPage()
{
    QScrollBar* scrollBar = ui->textBrowser->verticalScrollBar();
    int currentValue = scrollBar->value();
    int pageStep = scrollBar->pageStep();

    // 如果已经到达当前章节底部，切换到下一章
    if (currentValue + pageStep >= scrollBar->maximum()) {
        if (m_currentChapterIndex < m_chapterContents.size() - 1) {  // ✅ 使用正确的变量
            switchToChapter(m_currentChapterIndex + 1);  // ✅ 使用 switchToChapter
            ui->textBrowser->verticalScrollBar()->setValue(0);
        }
    } else {
        // 否则向下滚动一页
        scrollBar->setValue(currentValue + pageStep);
    }

    // 更新当前位置
    m_currentPosition = scrollBar->value();
}

void ReadingView::previousPage()
{
    QScrollBar* scrollBar = ui->textBrowser->verticalScrollBar();
    int currentValue = scrollBar->value();
    int pageStep = scrollBar->pageStep();

    // 如果已经到达当前章节顶部，切换到上一章
    if (currentValue <= 0) {
        if (m_currentChapterIndex > 0) {  // ✅ 使用正确的变量
            switchToChapter(m_currentChapterIndex - 1);  // ✅ 使用 switchToChapter
            QTimer::singleShot(50, this, [this]() {
                ui->textBrowser->verticalScrollBar()->setValue(
                    ui->textBrowser->verticalScrollBar()->maximum()
                    );
            });
        }
    } else {
        // 否则向上滚动一页
        scrollBar->setValue(qMax(0, currentValue - pageStep));
    }

    // 更新当前位置
    m_currentPosition = scrollBar->value();
}





/////////////////////////////////////
//听书
// 2. 修复 fetchAccessToken - 添加调试和错误处理
// ==================== readingview.cpp - 完整修复版 ====================

// 1. 获取 Token (添加详细日志)
void ReadingView::fetchAccessToken()
{

    QString apiKey = "MTHZ5WTDHRpZOPySBgZW2M3K";
    QString secretKey = "HAVi9Qoif8BSK4X9PjKOKSqUSM6Bw4n7";

    QUrl url("https://aip.baidubce.com/oauth/2.0/token");
    QUrlQuery params;
    params.addQueryItem("grant_type", "client_credentials");
    params.addQueryItem("client_id", apiKey);
    params.addQueryItem("client_secret", secretKey);
    url.setQuery(params);


    QNetworkRequest request(url);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onAccessTokenReplyFinished(reply);
    });
}

void ReadingView::onAccessTokenReplyFinished(QNetworkReply *reply)
{

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();

        QJsonDocument doc = QJsonDocument::fromJson(data);
        QJsonObject obj = doc.object();

        if (obj.contains("access_token")) {
            m_accessToken = obj.value("access_token").toString();
            qDebug() << "✅✅✅ Token 获取成功! ✅✅✅";
        }
    }
    reply->deleteLater();
}

// 2. 请求语音合成 (关键修复 - 添加详细日志)
void ReadingView::requestSpeech(const QString &text)
{

    if (m_accessToken.isEmpty()) {
        qDebug() << "❌❌❌ Token 为空，无法合成语音！ ❌❌❌";
        QMessageBox::warning(this, "错误", "语音服务未就绪,请稍后再试。\n(Token未获取)");
        return;
    }


    QUrl url("https://tsn.baidu.com/text2audio");
    QUrlQuery params;
    params.addQueryItem("tex", text);
    params.addQueryItem("tok", m_accessToken);
    params.addQueryItem("cuid", "reader-app-001");
    params.addQueryItem("ctp", "1");
    params.addQueryItem("lan", "zh");
    params.addQueryItem("per", "4");
    params.addQueryItem("spd", "5");
    params.addQueryItem("pit", "5");
    params.addQueryItem("vol", "15"); // 最大音量
    params.addQueryItem("aue", "3");

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();


    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_networkManager->post(request, postData);

    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onTtsReplyFinished(reply);
    });
}

// 3. 处理 TTS 响应
void ReadingView::onTtsReplyFinished(QNetworkReply *reply)
{

    if (reply->error() != QNetworkReply::NoError) {
        QMessageBox::warning(this, "错误", "网络请求失败: " + reply->errorString());
        onPlayerStopRequested();
        reply->deleteLater();
        return;
    }

    QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString();
    QByteArray responseData = reply->readAll();


    // 判断是否为音频
    if (contentType.contains("audio/") || responseData.size() > 1000) {

        // 停止当前播放
        if (m_player->playbackState() != QMediaPlayer::StoppedState) {
            qDebug() << "停止当前播放...";
            m_player->stop();
            QThread::msleep(100);
        }

        // 清理旧 buffer
        while (m_audioBuffers.size() > 2) {
            QBuffer* old = m_audioBuffers.takeFirst();
            old->deleteLater();
        }

        // 创建新 buffer
        QBuffer *newBuffer = new QBuffer(this);
        newBuffer->setData(responseData);

        if (!newBuffer->open(QIODevice::ReadOnly)) {
            delete newBuffer;
            reply->deleteLater();
            return;
        }

        m_audioBuffers.append(newBuffer);

        // 设置音频源
        m_player->setSourceDevice(newBuffer);

        // 确保音量
        if (m_audioOutput) {
            m_audioOutput->setVolume(1.0);
        }

        // 延迟播放
        QTimer::singleShot(200, this, [this]() {

            m_player->play();

            // 检查播放
            QTimer::singleShot(300, this, [this]() {

                if (m_player->error() != QMediaPlayer::NoError) {
                    qDebug() << "❌ 错误:" << m_player->errorString();
                } else {
                    qDebug() << "✅ 状态正常";
                }

                if (m_audioOutput) {
                    qDebug() << "音量:" << m_audioOutput->volume();
                    qDebug() << "是否静音:" << m_audioOutput->isMuted();
                } else {
                    qDebug() << "❌ audioOutput 是 null";
                }
            });
        });

    } else {
        // 不是音频

        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (!doc.isNull()) {
            QJsonObject obj = doc.object();
            QString errorMsg = obj.value("err_msg").toString();
            int errorNo = obj.value("err_no").toInt();
            qDebug() << "错误码:" << errorNo << "错误信息:" << errorMsg;
            QMessageBox::warning(this, "TTS错误",
                                 QString("错误码: %1\n错误信息: %2").arg(errorNo).arg(errorMsg));
        }

        onPlayerStopRequested();
    }

    reply->deleteLater();
}

// 3. 播放状态改变 (核心修复 - 自动播放下一句)
// readingview.cpp

void ReadingView::onPlayerStateChanged(QMediaPlayer::PlaybackState state)
{
    qDebug() << "【播放状态改变】" << m_previousPlaybackState << "->" << state;

    // 检查是否是“正常播放结束”的转换
    // 条件：上一个状态是 Playing，当前状态是 Stopped
    bool isPlaybackFinished = (m_previousPlaybackState == QMediaPlayer::PlayingState &&
                               state == QMediaPlayer::StoppedState);

    // 更新UI（这部分逻辑不变）
    if (state == QMediaPlayer::PlayingState) {
        m_ttsPlayer->onStateChanged(true, false);
    } else if (state == QMediaPlayer::PausedState) {
        m_ttsPlayer->onStateChanged(false, true);
    } else if (state == QMediaPlayer::StoppedState) {
        m_ttsPlayer->onStateChanged(false, false);
    }

    // 更新上一个状态的记录
    m_previousPlaybackState = state;

    // --- 核心修复：在这里处理自动播放下一句 ---
    if (isPlaybackFinished && !m_isWaitingForNextSentence) {
        qDebug() << "✅✅✅ 检测到正常播放结束，准备自动播放下一句 ✅✅✅";

        // 检查是否还有下一句
        if (m_currentSentenceIndex < m_sentences.size() - 1) {
            m_isWaitingForNextSentence = true; // 设置状态锁

            // 延迟后播放下一句
            QTimer::singleShot(200, this, [this]() { // 延时可以短一点
                qDebug() << "【自动切换】开始请求下一句";
                onPlayerSentenceChangeRequested(m_currentSentenceIndex + 1);
            });
        } else {
            qDebug() << "✅✅✅ 所有句子播放完成! ✅✅✅";
            m_ttsPlayer->updateProgress(100);
        }
    }
}

// 5. 播放器错误
void ReadingView::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{

    QMessageBox::warning(this, "播放错误",
                         QString("播放器错误:\n%1\n错误代码: %2").arg(errorString).arg(error));
}

// 6. 切换句子
void ReadingView::onPlayerSentenceChangeRequested(int newIndex)
{
    m_isWaitingForNextSentence = false; // <-- 在这里重置状态锁！
    if (newIndex < 0 || newIndex >= m_sentences.size()) {
        return;
    }

    m_currentSentenceIndex = newIndex;
    m_ttsPlayer->setCurrentIndex(newIndex);

    if (m_totalSentences > 0) {
        int progress = (m_currentSentenceIndex + 1) * 100 / m_totalSentences;
        m_ttsPlayer->updateProgress(progress);
    }

    highlightSentence(newIndex);

    QString textToSpeak = m_sentences.at(newIndex).trimmed();

    // ✅ 这里调用 requestSpeech
    requestSpeech(textToSpeak);
}

// 7. 停止播放
void ReadingView::onPlayerStopRequested()
{
    m_player->stop();
    m_ttsPlayer->updateProgress(0);
    unhighlightAll();
    m_isWaitingForNextSentence = false;
    m_previousPlaybackState = QMediaPlayer::StoppedState; // <-- 添加在这里！

    for (QBuffer* buffer : m_audioBuffers) {
        buffer->deleteLater();
    }
    m_audioBuffers.clear();

}

// 8. 开始听书
void ReadingView::onListenButtonClicked()
{

    if (m_ttsPlayer->isVisible()) {
        onPlayerStopRequested();
        m_ttsPlayer->hide();
        return;
    }

    QString fullText = ui->textBrowser->toPlainText();
    m_sentences.clear();
    fullText = fullText.simplified();

    QStringList tempSentences = fullText.split(
        QRegularExpression("[。！？；]"),
        Qt::SkipEmptyParts
        );

    for (const QString& sentence : tempSentences) {
        QString cleaned = sentence.trimmed();
        if (cleaned.length() >= 5) {
            m_sentences.append(cleaned);
        }
    }


    if (m_sentences.isEmpty()) {
        QMessageBox::information(this, "提示", "当前没有可供朗读的文本。");
        return;
    }


    m_totalSentences = m_sentences.size();
    m_currentSentenceIndex = 0;
    m_isWaitingForNextSentence = false;

    m_previousPlaybackState = QMediaPlayer::StoppedState; // <-- 添加在这里！

    m_ttsPlayer->setTotalSentences(m_totalSentences);
    m_ttsPlayer->setCurrentIndex(m_currentSentenceIndex);
    m_ttsPlayer->updateProgress(0);
    m_ttsPlayer->onStateChanged(false, false);

    m_ttsPlayer->show();

    // 开始播放第一句
    onPlayerSentenceChangeRequested(0);
}

// 其他函数保持不变...
void ReadingView::onPlayerPauseRequested()
{
    m_player->pause();
}

void ReadingView::onPlayerResumeRequested()
{
    m_player->play();
}

void ReadingView::onPlayerPlayRequested(int index)
{
    onPlayerSentenceChangeRequested(index);
}

void ReadingView::highlightSentence(int index)
{
    unhighlightAll();
    if (index < 0 || index >= m_sentences.size()) return;

    QString sentenceToFind = m_sentences.at(index);
    QTextCursor cursor(ui->textBrowser->document());
    cursor = ui->textBrowser->document()->find(sentenceToFind, cursor);

    if (!cursor.isNull()) {
        QTextCharFormat format;
        format.setBackground(Qt::yellow);
        cursor.setCharFormat(format);
    }
}

void ReadingView::unhighlightAll()
{
    QTextCursor cursor(ui->textBrowser->document());
    cursor.select(QTextCursor::Document);

    QTextCharFormat format;
    format.setBackground(Qt::transparent);
    cursor.setCharFormat(format);

    cursor.clearSelection();
    ui->textBrowser->setTextCursor(cursor);
}

//////////////////

void ReadingView::notepad()
{
    QDialog* dialog = new QDialog(this);
    dialog->setWindowTitle("添加笔记");
    dialog->setMinimumSize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(dialog);

    // 标题输入
    QLabel* titleLabel = new QLabel("笔记标题:", dialog);
    QLineEdit* titleEdit = new QLineEdit(dialog);
    titleEdit->setPlaceholderText("请输入笔记标题");

    // 内容输入
    QLabel* contentLabel = new QLabel("笔记内容:", dialog);
    QTextEdit* contentEdit = new QTextEdit(dialog);
    contentEdit->setPlaceholderText("在这里写笔记内容...");

    // 按钮
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    QPushButton* saveButton = new QPushButton("保存", dialog);
    QPushButton* cancelButton = new QPushButton("取消", dialog);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);

    // 布局
    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(titleEdit);
    mainLayout->addWidget(contentLabel);
    mainLayout->addWidget(contentEdit);
    mainLayout->addLayout(buttonLayout);

    // 连接信号
    connect(saveButton, &QPushButton::clicked, [=]() {
        QString title = titleEdit->text().trimmed();
        QString content = contentEdit->toPlainText().trimmed();

        if (title.isEmpty()) {
            QMessageBox::warning(dialog, "提示", "笔记标题不能为空！");
            return;
        }

        if (content.isEmpty()) {
            QMessageBox::warning(dialog, "提示", "笔记内容不能为空！");
            return;
        }

        // 保存笔记到书签系统
        if (m_bookManager) {
            QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

            BookmarkInfo newNote;
            newNote.type = BookmarkInfo::Note;
            newNote.chapterIndex = m_currentChapterIndex;  // ✅ 使用正确的变量
            newNote.startPos = ui->textBrowser->textCursor().position();
            newNote.endPos = newNote.startPos;
            newNote.content = QString("%1\n\n%2").arg(title).arg(content);

            bookmarks.append(newNote);
            m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

            QMessageBox::information(dialog, "成功", "笔记已保存！");
            dialog->accept();
        }
    });

    connect(cancelButton, &QPushButton::clicked, dialog, &QDialog::reject);

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}

// 6. 修复关键词对话框 ✅
void ReadingView::showKeyword()
{
    KeywordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // 对话框关闭后,触发重新高亮 ✅
        if (m_highlighter) {
            m_highlighter->highlight();
        }
    }
}


void ReadingView::saveProgress()
{
    ConfigManager::instance().setLastPage(m_currentBook.filePath, ui->textBrowser->verticalScrollBar()->value());
    ConfigManager::instance().setTotalPage(m_currentBook.filePath, ui->textBrowser->verticalScrollBar()->maximum());
}

// 清除搜索高亮的专用函数
// 6. clearSearchHighlights - 清除搜索高亮（清空 ExtraSelections）
void ReadingView::clearSearchHighlights()
{
    // 使用 ExtraSelections 叠加的高亮，直接清空即可
    ui->textBrowser->setExtraSelections({});
}
