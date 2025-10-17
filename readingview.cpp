
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
#include <QtNetwork>
#include <QRegularExpression>
#include <QMediaPlayer>
#include <QTextCursor>//高亮部分
#include <QTextCharFormat>

#include "keyworddialog.h"
#include "ConfigManager.h"

ReadingView::ReadingView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ReadingView),
    m_currentChapter(0),
    m_currentPosition(0),
    m_lastSearchCaseSensitive(false),
    m_chapterListVisible(true) // 默认显示章节列表
{
    ui->setupUi(this);

    // 全局设置连接
    connect(&SettingsManager::instance(), &SettingsManager::settingsChanged, this, &ReadingView::applySettings);

    // 书签右键菜单连接
    ui->textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, &ReadingView::showContextMenu);

    // 自动阅读定时器
    m_autoScrollTimer = new QTimer(this);
    connect(m_autoScrollTimer, &QTimer::timeout, this, &ReadingView::onAutoScrollTimerTimeout);

    // // 查词服务
    // m_dictionaryService = new DictionaryService(this);
    // connect(m_dictionaryService, &DictionaryService::querySuccess, this, &ReadingView::onQuerySuccess);
    // connect(m_dictionaryService, &DictionaryService::queryError, this, &ReadingView::onQueryError);


    // ★★★ 关键修复1: 在构造函数中创建 TooltipPopup ★★★
    m_tooltipPopup = new TooltipPopup(this);

    // ★★★ 关键修复2: 创建查词服务并连接信号 ★★★
    m_dictionaryService = new DictionaryService(this);
    connect(m_dictionaryService, &DictionaryService::querySuccess,
            this, &ReadingView::onQuerySuccess);
    connect(m_dictionaryService, &DictionaryService::queryError,
            this, &ReadingView::onQueryError);

    // ★★★ 关键修复3: 为 QTextBrowser 安装事件过滤器 ★★★
    ui->textBrowser->viewport()->installEventFilter(this);
    // 初始化UI
    auto& settings = SettingsManager::instance();
    ui->speedSlider->setRange(10, 500);
    ui->speedSlider->setInvertedAppearance(true);
    ui->speedSlider->setValue(settings.autoScrollSpeed());
    setAutoScrollSpeed(settings.autoScrollSpeed());
    applySettings();

    // 初始化书签格式
    m_bookmarkFormat.setBackground(QColor(255, 255, 0, 50));
    m_bookmarkFormat.setUnderlineStyle(QTextCharFormat::DashUnderline);
    m_bookmarkFormat.setUnderlineColor(Qt::gray);

    // 连接UI中的上一页/下一页按钮
    connect(ui->prevPageButton, &QPushButton::clicked, this, &ReadingView::previousPage);
    connect(ui->nextPageButton, &QPushButton::clicked, this, &ReadingView::nextPage);

    // 创建搜索控件
    m_searchLineEdit = new QLineEdit(this);
    m_searchLineEdit->setPlaceholderText("搜索内容...");
    m_searchLineEdit->setFixedWidth(200);

    m_searchButton = new QPushButton("搜索", this);
    connect(m_searchButton, &QPushButton::clicked, this, [this]() {
        findText(m_searchLineEdit->text(), false);
   });
    // //听书播放器
    //     // 1. 监听播放器的请求
    //     connect(m_ttsPlayer, &TtsPlayer::playRequested, this, &ReadingView::onSpeak);
    //     connect(m_ttsPlayer, &TtsPlayer::pauseRequested, m_tts, &QTextToSpeech::pause);
    //     connect(m_ttsPlayer, &TtsPlayer::resumeRequested, m_tts, &QTextToSpeech::resume);
    //     connect(m_ttsPlayer, &TtsPlayer::stopRequested, this, &ReadingView::onStop);
    //     connect(m_ttsPlayer, &TtsPlayer::sentenceChangeRequested, this, &ReadingView::onSpeak);

    //     // 2. 监听TTS引擎的状态变化，以更新播放器UI
    //     connect(m_tts, &QTextToSpeech::stateChanged, this, &ReadingView::onTtsStateChanged);

    //     // 3. 监听播放器关闭事件
    //     connect(m_ttsPlayer, &TtsPlayer::closed, this, &ReadingView::onStop);

        // --- 新增：连接“听书”按钮 ---
        connect(ui->listenButton, &QPushButton::clicked, this, &ReadingView::onListenButtonClicked);
        // --- 初始化网络和播放器 ---
        m_networkManager = new QNetworkAccessManager(this);
        m_player = new QMediaPlayer(this);

        // --- 初始化播放器 UI ---
        m_ttsPlayer = new TtsPlayer(this);
        m_ttsPlayer->hide();

        // --- 连接 TtsPlayer 的信号 ---
        connect(m_ttsPlayer, &TtsPlayer::playRequested, this, &ReadingView::onPlayerPlayRequested);
        connect(m_ttsPlayer, &TtsPlayer::pauseRequested, this, &ReadingView::onPlayerPauseRequested);
        connect(m_ttsPlayer, &TtsPlayer::resumeRequested, this, &ReadingView::onPlayerResumeRequested);
        connect(m_ttsPlayer, &TtsPlayer::stopRequested, this, &ReadingView::onPlayerStopRequested);
        connect(m_ttsPlayer, &TtsPlayer::sentenceChangeRequested, this, &ReadingView::onPlayerSentenceChangeRequested);
        connect(m_ttsPlayer, &TtsPlayer::closed, this, &ReadingView::onPlayerStopRequested); // 当播放器关闭时也停止播放

        // --- 连接播放器的状态变化 ---
        connect(m_player, &QMediaPlayer::playbackStateChanged, this, &ReadingView::onPlayerStateChanged);

        // --- 启动时先获取一次 Access Token ---
        fetchAccessToken();

        // 构造函数中
        connect(m_player, &QMediaPlayer::errorOccurred, this, &ReadingView::onPlayerError);




    m_searchNextButton = new QPushButton("下一个", this);
    connect(m_searchNextButton, &QPushButton::clicked, this, &ReadingView::findNext);

    m_searchPrevButton = new QPushButton("上一个", this);
    connect(m_searchPrevButton, &QPushButton::clicked, this, &ReadingView::findPrevious);



    connect(ui->noteButton, &QPushButton::clicked, this, &ReadingView::notepad);


    connect(ui->keyButton, &QPushButton::clicked, this, &ReadingView::showKeyword);

    // // 创建章节切换按钮
    // m_toggleChapterButton = new QPushButton("隐藏章节", this);
    // connect(m_toggleChapterButton, &QPushButton::clicked, this, &ReadingView::toggleChapterList);

    // 将搜索控件添加到布局中
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_searchNextButton);
    searchLayout->addWidget(m_searchPrevButton);
    searchLayout->addStretch();
    //searchLayout->addWidget(m_toggleChapterButton);

    // 将搜索布局添加到界面上方
    QVBoxLayout* mainLayout = qobject_cast<QVBoxLayout*>(layout());
    if (mainLayout) {
        mainLayout->insertLayout(0, searchLayout);
    }

    // 设置章节列表宽度更窄
    ui->chaptersListWidget->setMaximumWidth(150);
    applySettings();
}

ReadingView::~ReadingView()
{
    delete ui;
}

void ReadingView::onPlayerError(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "QMediaPlayer Error:" << error << errorString;
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

void ReadingView::on_autoScrollButton_toggled(bool checked)
{
    if (checked) {
        startAutoScroll();
        ui->autoScrollButton->setText("停止阅读");
    } else {
        stopAutoScroll();
        ui->autoScrollButton->setText("自动阅读");
    }
}

void ReadingView::on_speedSlider_valueChanged(int value)
{
    SettingsManager::instance().setAutoScrollSpeed(value); // 更新全局设置
    setAutoScrollSpeed(value); // 立即更新当前视图的速度
}
//上面截至到


void ReadingView::loadBook(const BookInfo &book)
{
    //书签！！！
    m_currentBook = book; // ★★★ 记下当前的书籍 ★★★

    // 确保书籍有唯一ID，如果没有则使用文件路径作为ID
    if (m_currentBook.id.isEmpty()) {
        m_currentBook.id = m_currentBook.filePath;
    }
    QFile file(book.filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        ui->textBrowser->setText("错误：无法打开文件 " + book.filePath);
        return;
    }

    QTextStream in(&file);
    // 在 Qt 6 中，QTextStream 默认使用 UTF-8，不再需要 setCodec

    QString content = in.readAll();
    file.close();

    parseChapters(content);

    QApplication::processEvents();//!!!
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



void ReadingView::on_chaptersListWidget_itemClicked(QListWidgetItem *item)
{
    if (!item) return;
    int index = ui->chaptersListWidget->row(item);
    if (index >= 0 && index < m_chapterContents.size()) {
        m_currentChapterIndex = index;
        ui->textBrowser->setText(m_chapterContents.at(index));
        applyBookmarksToChapter(index);

        // --- 新增：检查文本是否为空，并据此设置按钮状态 ---
        bool hasText = !ui->textBrowser->toPlainText().isEmpty();
        ui->listenButton->setEnabled(hasText);
    }
}

// readingview.cpp

void ReadingView::applySettings()
{
    // 获取全局设置的单例对象
    auto& settings = SettingsManager::instance();

    // 1. --- 设置字体 ---
    // 创建一个 QFont 对象，并从 SettingsManager 获取字体家族和大小
    QFont font(settings.fontFamily());
    font.setPointSize(settings.fontSize());

    // 将这个字体同时应用到正文显示区和章节列表
    ui->textBrowser->setFont(font);
    ui->chaptersListWidget->setFont(font);

    // 2. --- 设置背景和前景颜色 ---
    QString browserStyle, listStyle;

    // 判断当前是否是自定义图片背景模式
    if (settings.backgroundMode() == SettingsManager::CustomImage && !settings.customImagePath().isEmpty())
    {
        // --- 图片背景模式 ---
        QString imagePath = settings.customImagePath();
        imagePath.replace("\\", "/"); // 确保路径格式正确

        // 为 QTextBrowser 准备样式表
        browserStyle = QString(
                           "QTextBrowser {"
                           "  border-image: url(%1) 0 0 0 0 stretch stretch;" // 设置背景图片
                           "  background-color: transparent;"              // 背景色必须设为透明
                           "  color: %2;"                                   // 从全局设置获取文字颜色
                           "  border: none;"                                // 去掉边框
                           "}"
                           ).arg(imagePath).arg(settings.textColor().name());

        // 为 QListWidget 准备样式表
        listStyle = QString(
                        "QListWidget {"
                        "  border-image: url(%1) 0 0 0 0 stretch stretch;"
                        "  background-color: transparent;"
                        "  color: %2;"
                        "  border: none;"
                        "}"
                        // 保持选中项的高亮样式
                        "QListWidget::item:selected { background-color: #4a6984; color: white; }"
                        ).arg(imagePath).arg(settings.textColor().name());
    }
    else
    {
        // --- 纯色背景模式 ---
        // 为 QTextBrowser 准备样式表
        browserStyle = QString("QTextBrowser { background-color: %1; color: %2; border: none; }")
                           .arg(settings.backgroundColor().name(QColor::HexRgb))
                           .arg(settings.textColor().name(QColor::HexRgb));

        // 为 QListWidget 准备样式表
        listStyle = QString(
                        "QListWidget { background-color: %1; color: %2; border: none; }"
                        "QListWidget::item:selected { background-color: #4a6984; color: white; }"
                        ).arg(settings.backgroundColor().name(QColor::HexRgb)).arg(settings.textColor().name(QColor::HexRgb));
    }

    // 3. --- 将生成的样式表应用到控件 ---
    ui->textBrowser->setStyleSheet(browserStyle);
    ui->chaptersListWidget->setStyleSheet(listStyle);
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

    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);
    BookmarkInfo newBookmark;
    newBookmark.chapterIndex = m_currentChapterIndex;
    newBookmark.startPos = cursor.selectionStart();
    newBookmark.endPos = cursor.selectionEnd();
    newBookmark.iconPath = SettingsManager::instance().bookmarkIconPath();
    newBookmark.backgroundColor = QColor(255, 255, 0, 50);
    newBookmark.underlineStyle = QTextCharFormat::DashUnderline;
    newBookmark.underlineColor = Qt::gray;

    bookmarks.append(newBookmark);
    m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

    on_chaptersListWidget_itemClicked(ui->chaptersListWidget->currentItem());
}

void ReadingView::removeBookmark()
{
    if (!m_bookManager) return;
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) return;

    int dbId = action->data().toInt();
    QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

    // 我们需要通过一个唯一ID来找到书签，而不是位置
    // (这是一个更高级的实现，暂时我们还是用索引)
    // 假设 dbId 就是索引
    if(dbId >= 0 && dbId < bookmarks.size()){
        bookmarks.removeAt(dbId);
        m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);
        on_chaptersListWidget_itemClicked(ui->chaptersListWidget->currentItem());
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
            QTextCharFormat format;
            format.setBackground(bookmark.backgroundColor);
            format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(bookmark.underlineStyle));
            format.setUnderlineColor(bookmark.underlineColor);
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


void ReadingView::findText(const QString& text, bool caseSensitive)
{
    if (text.isEmpty()) return;

    m_lastSearchText = text;
    m_lastSearchCaseSensitive = caseSensitive;

    QTextDocument::FindFlags flags;
    if (caseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    // 先在当前章节搜索
    QTextCursor cursor = ui->textBrowser->textCursor();
    cursor.setPosition(0); // 从文档开头开始搜索
    ui->textBrowser->setTextCursor(cursor);

    bool found = ui->textBrowser->find(text, flags);
    if (found) {
        highlightFoundText(ui->textBrowser->textCursor());
        return;
    }

    // 如果当前章节未找到，则在其他章节中搜索
    int originalChapter = m_currentChapter;

    // 从下一章开始搜索
    for (int i = 1; i < m_chapterContents.size(); i++) {
        int chapterToSearch = (originalChapter + i) % m_chapterContents.size();

        // 检查该章节内容是否包含搜索文本
        if (m_chapterContents[chapterToSearch].contains(text,
                                                        caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {
            // 切换到包含文本的章节
            m_currentChapter = chapterToSearch;
            ui->chaptersListWidget->setCurrentRow(chapterToSearch);
            ui->textBrowser->setText(m_chapterContents.at(chapterToSearch));

            // 在新章节中搜索
            cursor = ui->textBrowser->textCursor();
            cursor.setPosition(0);
            ui->textBrowser->setTextCursor(cursor);

            if (ui->textBrowser->find(text, flags)) {
                highlightFoundText(ui->textBrowser->textCursor());
                QMessageBox::information(this, "查找结果",
                                         QString("在第 %1 章找到匹配文本").arg(chapterToSearch + 1));
                return;
            }
        }
    }

    // 如果所有章节都未找到
    QMessageBox::information(this, "查找结果", "在全书中未找到匹配文本");

    // 恢复到原始章节
    if (m_currentChapter != originalChapter) {
        m_currentChapter = originalChapter;
        ui->chaptersListWidget->setCurrentRow(originalChapter);
        ui->textBrowser->setText(m_chapterContents.at(originalChapter));
    }
}

void ReadingView::findNext()
{
    clearHighlights(); // *** 调用新的清除函数 ***
    if (m_lastSearchText.isEmpty()) return;

    QTextDocument::FindFlags flags;
    if (m_lastSearchCaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    // 先在当前章节的当前位置之后搜索
    bool found = ui->textBrowser->find(m_lastSearchText, flags);
    if (found) {
        highlightFoundText(ui->textBrowser->textCursor());
        return;
    }

    // 如果当前章节没有找到，搜索后续章节
    int originalChapter = m_currentChapter;

    for (int i = 1; i < m_chapterContents.size(); i++) {
        int chapterToSearch = (originalChapter + i) % m_chapterContents.size();

        // 检查该章节内容是否包含搜索文本
        if (m_chapterContents[chapterToSearch].contains(m_lastSearchText,
                                                        m_lastSearchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {

            // 切换到包含文本的章节
            m_currentChapter = chapterToSearch;
            ui->chaptersListWidget->setCurrentRow(chapterToSearch);
            ui->textBrowser->setText(m_chapterContents.at(chapterToSearch));

            // 在新章节中从头搜索
            QTextCursor cursor = ui->textBrowser->textCursor();
            cursor.setPosition(0);
            ui->textBrowser->setTextCursor(cursor);

            if (ui->textBrowser->find(m_lastSearchText, flags)) {
                highlightFoundText(ui->textBrowser->textCursor());
                return;
            }
        }
    }

    // 如果所有后续章节都没找到，从第一章重新开始搜索
    if (originalChapter > 0) {
        for (int i = 0; i < originalChapter; i++) {
            if (m_chapterContents[i].contains(m_lastSearchText,
                                              m_lastSearchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {

                m_currentChapter = i;
                ui->chaptersListWidget->setCurrentRow(i);
                ui->textBrowser->setText(m_chapterContents.at(i));

                QTextCursor cursor = ui->textBrowser->textCursor();
                cursor.setPosition(0);
                ui->textBrowser->setTextCursor(cursor);

                if (ui->textBrowser->find(m_lastSearchText, flags)) {
                    highlightFoundText(ui->textBrowser->textCursor());
                    return;
                }
            }
        }
    }

    // 如果全文都没找到更多匹配
    QMessageBox::information(this, "查找结果", "已到达最后一个匹配项");
}

void ReadingView::findPrevious()
{
    clearHighlights(); // ★★★ 调用新的清除函数 ★★★
    if (m_lastSearchText.isEmpty()) return;

    QTextDocument::FindFlags flags = QTextDocument::FindBackward;
    if (m_lastSearchCaseSensitive) {
        flags |= QTextDocument::FindCaseSensitively;
    }

    // 先在当前章节的当前位置之前搜索
    bool found = ui->textBrowser->find(m_lastSearchText, flags);
    if (found) {
        highlightFoundText(ui->textBrowser->textCursor());
        return;
    }

    // 如果当前章节没有找到，搜索前面的章节
    int originalChapter = m_currentChapter;

    for (int i = 1; i <= originalChapter; i++) {
        int chapterToSearch = originalChapter - i;

        // 检查该章节内容是否包含搜索文本
        if (m_chapterContents[chapterToSearch].contains(m_lastSearchText,
                                                        m_lastSearchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {

            // 切换到包含文本的章节
            m_currentChapter = chapterToSearch;
            ui->chaptersListWidget->setCurrentRow(chapterToSearch);
            ui->textBrowser->setText(m_chapterContents.at(chapterToSearch));

            // 在新章节中从尾部向前搜索
            QTextCursor cursor = ui->textBrowser->textCursor();
            cursor.setPosition(ui->textBrowser->document()->characterCount() - 1);
            ui->textBrowser->setTextCursor(cursor);

            if (ui->textBrowser->find(m_lastSearchText, flags)) {
                highlightFoundText(ui->textBrowser->textCursor());
                return;
            }
        }
    }

    // 如果所有前面章节都没找到，从最后一章开始向前搜索
    if (originalChapter < m_chapterContents.size() - 1) {
        for (int i = m_chapterContents.size() - 1; i > originalChapter; i--) {
            if (m_chapterContents[i].contains(m_lastSearchText,
                                              m_lastSearchCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive)) {

                m_currentChapter = i;
                ui->chaptersListWidget->setCurrentRow(i);
                ui->textBrowser->setText(m_chapterContents.at(i));

                QTextCursor cursor = ui->textBrowser->textCursor();
                cursor.setPosition(ui->textBrowser->document()->characterCount() - 1);
                ui->textBrowser->setTextCursor(cursor);

                if (ui->textBrowser->find(m_lastSearchText, flags)) {
                    highlightFoundText(ui->textBrowser->textCursor());
                    return;
                }
            }
        }
    }

    // 如果全文都没找到更多匹配
    QMessageBox::information(this, "查找结果", "已到达第一个匹配项");
}

void ReadingView::highlightFoundText(const QTextCursor& cursor)
{
    // 获取当前设置管理器
    auto& settings = SettingsManager::instance();

    // ★★★ 修复高亮显示重复和排版混乱问题 ★★★483-489

    // 1. 先清除所有现有的高亮格式
    QTextCursor resetCursor(ui->textBrowser->document());
    resetCursor.select(QTextCursor::Document);
    QTextCharFormat clearFormat;
    clearFormat.setBackground(Qt::transparent);
    clearFormat.setForeground(settings.textColor()); // 恢复默认文本颜色
    resetCursor.mergeCharFormat(clearFormat);

    // 2. 设置新的高亮颜色，确保在夜间模式黑色背景下也能清晰可见
    QColor highlightColor;
    QColor highlightTextColor;

    // 根据是否为夜间模式选择更合适的高亮颜色
    if (settings.isNightMode()) {
        // 更改为亮青色背景和深色文字，确保在任何深色夜间背景下都清晰可见
        highlightColor = QColor("#40E0D0");  // ***绿松石/青色 (Turquoise)，高对比度
        highlightTextColor = QColor("#000000");  // 黑色文字，与亮青色形成强烈对比
    } else {
        highlightColor = QColor("#FBBF24");  // ***明亮的黄色背景
        highlightTextColor = QColor("#000000");  // 黑色文字
    }

    // 3. 设选中文本的格式
    QTextCharFormat format;
    format.setBackground(highlightColor);
    format.setForeground(highlightTextColor);

    // 4. 创建一个新的光标来设置格式，只高亮当前找到的文本
    QTextCursor highlightCursor = cursor;

    // ***514-526***
    if (!highlightCursor.hasSelection()) {
        // 如果没有选中文本，选中当前单词
        highlightCursor.select(QTextCursor::WordUnderCursor);
    }

    // 应用高亮格式
    highlightCursor.mergeCharFormat(format);

    // ★★★ 核心修复：清除选中状态，防止系统选中颜色覆盖自定义高亮 ★★★
    highlightCursor.clearSelection();

    // 设置文本浏览器的当前光标为高亮光标
    ui->textBrowser->setTextCursor(highlightCursor);

    // 5. 确保找到的文本可见
    ui->textBrowser->ensureCursorVisible();

    // 6. 更新当前位置
    m_currentPosition = ui->textBrowser->verticalScrollBar()->value();
}
// readingview.cpp (在文件任意合适位置添加)

void ReadingView::clearHighlights()//****536-549
{
    // 使用新的光标对象来操作整个文档
    QTextCursor cursor(ui->textBrowser->document());

    // 创建一个默认的格式
    QTextCharFormat defaultFormat;

    // 移动光标选择整个文档
    cursor.select(QTextCursor::Document);

    // 应用默认格式，这会移除所有自定义字符格式（包括搜索高亮）
    cursor.setCharFormat(defaultFormat);
}

// 翻页功能
void ReadingView::nextPage()
{
    QScrollBar* scrollBar = ui->textBrowser->verticalScrollBar();
    int currentValue = scrollBar->value();
    int pageStep = scrollBar->pageStep();

    // 如果已经到达当前章节底部，切换到下一章
    if (currentValue + pageStep >= scrollBar->maximum()) {
        if (m_currentChapter < m_chapterContents.size() - 1) {
            m_currentChapter++;
            ui->chaptersListWidget->setCurrentRow(m_currentChapter);
            ui->textBrowser->setText(m_chapterContents.at(m_currentChapter));
            scrollBar->setValue(0); // 从新章节顶部开始
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
        if (m_currentChapter > 0) {
            m_currentChapter--;
            ui->chaptersListWidget->setCurrentRow(m_currentChapter);
            ui->textBrowser->setText(m_chapterContents.at(m_currentChapter));
            scrollBar->setValue(scrollBar->maximum()); // 从上一章节底部开始
        }
    } else {
        // 否则向上滚动一页
        scrollBar->setValue(qMax(0, currentValue - pageStep));
    }

    // 更新当前位置
    m_currentPosition = scrollBar->value();
}




//听书
void ReadingView::fetchAccessToken()
{
    // --- 替换为你在百度AI平台申请的真实 Key ---
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
        m_accessToken = doc.object().value("access_token").toString();
        qDebug() << "Access Token acquired:" << m_accessToken;
    } else {
        qDebug() << "Failed to get access token:" << reply->errorString();
    }
    reply->deleteLater();
}
//实现请求语音合成和播放的逻辑
void ReadingView::requestSpeech(const QString &text)
{
    if (m_accessToken.isEmpty()) {
        qDebug() << "Access Token is not ready!";
        return;
    }

    QUrl url("https://tsn.baidu.com/text2audio");
    QUrlQuery params;
    params.addQueryItem("tex", text);             // 要合成的文本
    params.addQueryItem("tok", m_accessToken);    // Access Token
    params.addQueryItem("cuid", "some-random-id"); // 客户端唯一标识，随便填
    params.addQueryItem("ctp", "1");              // 客户端类型，填1
    params.addQueryItem("lan", "zh");             // 语言，zh中文
    params.addQueryItem("per", "4");              // 音色选择，4是度逍遥（情感男声），可以换成0,1,3,5等
    params.addQueryItem("spd", "5");              // 语速，0-15，默认5
    params.addQueryItem("aue", "3");              // 音频格式，3是mp3

    QByteArray postData = params.query(QUrl::FullyEncoded).toUtf8();

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply = m_networkManager->post(request, postData);
    connect(reply, &QNetworkReply::finished, this, [this, reply](){
        onTtsReplyFinished(reply);
    });
}

// readingview.cpp

void ReadingView::onTtsReplyFinished(QNetworkReply *reply)
{
    // 检查返回的 Content-Type，如果不是音频则说明出错了
    if (reply->header(QNetworkRequest::ContentTypeHeader).toString().contains("audio/")) {
        // 成功获取 MP3 数据
        QByteArray audioData = reply->readAll();

        // --- 核心修复：为 QBuffer 设置 parent ---
        QBuffer *buffer = new QBuffer(m_player); // <--- 将 m_player 作为 buffer 的父对象！

        // // 确保旧的源设备被正确清理
        // if (m_player->sourceDevice()) {
        //     m_player->sourceDevice()->deleteLater();
        // }

        buffer->setData(audioData);
        buffer->open(QIODevice::ReadOnly);

        m_player->setSourceDevice(buffer);
        m_player->play();

    } else {
        // 出错了，打印错误信息
        qDebug() << "TTS request failed:" << reply->readAll();
        // ★★★ 调试技巧：当 TTS 请求失败时，也应该让播放器停止 ★★★
        onPlayerStopRequested();
    }
    reply->deleteLater();
}

// 当 TtsPlayer 的播放按钮被点击
void ReadingView::onPlayerPlayRequested(int index)
{
    if(m_sentences.isEmpty()) {
        // 第一次播放，准备文本
        QString fullText = ui->textBrowser->toPlainText();
       m_sentences = fullText.split(QRegularExpression("[。？！]"), Qt::SkipEmptyParts); // <-- 正确的 Qt6 写法
        if(m_sentences.isEmpty()) return;
        m_ttsPlayer->setTotalSentences(m_sentences.size());
    }
    onPlayerSentenceChangeRequested(index);
}

// 切换句子的核心函数
void ReadingView::onPlayerSentenceChangeRequested(int newIndex)
{
    if (newIndex < 0 || newIndex >= m_sentences.size()) return;

    m_currentSentenceIndex = newIndex;
    m_ttsPlayer->setCurrentIndex(newIndex);//更新索引，按钮状态
    // 更新进度条等UI
    // --- 新增：计算并更新进度条 ---
    if (m_totalSentences > 0) {
        // 使用浮点数进行计算以保证精度，避免整数除法得到0
        double progress = (static_cast<double>(m_currentSentenceIndex + 1) / m_totalSentences) * 100.0;
        m_ttsPlayer->updateProgress(static_cast<int>(progress)); // 发送更新指令
    }
    highlightSentence(newIndex);
    requestSpeech(m_sentences.at(newIndex));
}

void ReadingView::onPlayerPauseRequested() { m_player->pause(); }
void ReadingView::onPlayerResumeRequested() { m_player->play(); }
void ReadingView::onPlayerStopRequested() {
    m_player->stop();
    m_ttsPlayer->updateProgress(0); // <-- 新增：停止时将进度条归零
    unhighlightAll();
}

// 监听 QMediaPlayer 的状态，实现自动播放下一句
void ReadingView::onPlayerStateChanged(QMediaPlayer::PlaybackState  state)
{
    if (state == QMediaPlayer::PlayingState) {
        m_ttsPlayer->onStateChanged(true, false);
    } else if (state == QMediaPlayer::PausedState) {
        m_ttsPlayer->onStateChanged(false, true);
    } else if (state == QMediaPlayer::StoppedState) {
        m_ttsPlayer->onStateChanged(false, false);

        // 判断是否是正常播放结束
        if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
            // 自动播放下一句
            if (m_currentSentenceIndex < m_sentences.size() - 1) {
                onPlayerSentenceChangeRequested(m_currentSentenceIndex + 1);
            } else {
                qDebug() << "All sentences finished.";
            }
        }
    }
    //播放结束的设置
    if (state == QMediaPlayer::StoppedState) {
        m_ttsPlayer->onStateChanged(false, false);

        if (m_player->mediaStatus() == QMediaPlayer::EndOfMedia) {
            if (m_currentSentenceIndex < m_sentences.size() - 1) {
                onPlayerSentenceChangeRequested(m_currentSentenceIndex + 1);
            } else {
                qDebug() << "All sentences finished.";
                m_ttsPlayer->updateProgress(100); // <-- 优化：确保结束时进度为100%
            }
        }
    }

}

//阅读中高亮

void ReadingView::highlightSentence(int index)
{
    // 先取消之前所有的高亮
    unhighlightAll();

    if (index < 0 || index >= m_sentences.size()) return;

    // 这只是一个简单的实现，可能需要根据你的实际文本结构进行微调
    QString sentenceToFind = m_sentences.at(index);

    QTextCursor cursor(ui->textBrowser->document());
    cursor = ui->textBrowser->document()->find(sentenceToFind, cursor);

    if (!cursor.isNull()) {
        QTextCharFormat format;
        format.setBackground(Qt::yellow); // 设置高亮背景色为黄色
        cursor.setCharFormat(format);
    }
}

void ReadingView::unhighlightAll()
{
    QTextCursor cursor(ui->textBrowser->document());
    cursor.select(QTextCursor::Document); // 选中全部文档

    QTextCharFormat format;
    format.setBackground(Qt::transparent); // 设置背景为透明
    cursor.setCharFormat(format);

    cursor.clearSelection(); // 取消选中
    ui->textBrowser->setTextCursor(cursor); // 更新光标
}



// --- 在这里添加缺失的 onListenButtonClicked 函数实现 ---

void ReadingView::onListenButtonClicked()
{
    // 检查播放器是否已经可见，如果可见，则隐藏它并停止播放
    if (m_ttsPlayer->isVisible()) {
        onPlayerStopRequested(); // 调用我们写好的停止函数
        m_ttsPlayer->hide();
        return;
    }

    // --- 准备文本 ---
    QString fullText = ui->textBrowser->toPlainText();
    m_sentences = fullText.split(QRegularExpression("[。？！]"), Qt::SkipEmptyParts);

    if (m_sentences.isEmpty()) {
        qDebug() << "No text to read.";
        // 在这里可以弹出一个提示框告诉用户没有内容
        QMessageBox::information(this, "提示", "当前没有可供朗读的文本。");
        return;
    }

    // --- 初始化状态 ---
    m_totalSentences = m_sentences.size();
    m_currentSentenceIndex = 0;

    // --- 初始化播放器 UI ---
    m_ttsPlayer->setTotalSentences(m_totalSentences);
    m_ttsPlayer->setCurrentIndex(m_currentSentenceIndex);
    m_ttsPlayer->updateProgress(0); // 确保进度条从0开始
    m_ttsPlayer->onStateChanged(false, false); // 确保按钮是“播放”状态

    // --- 显示播放器并开始播放 ---
    m_ttsPlayer->show();

    // 自动开始播放第一句
    onPlayerSentenceChangeRequested(0);
}


void ReadingView::onSliderChanged()
{
    double hCur = ui->textBrowser->verticalScrollBar()->value();
    double hMax = ui->textBrowser->verticalScrollBar()->maximum();
    if(hMax <= 0) {
        ui->label_3->setText(QString::number(0) + "%");
    } else {
        ui->label_3->setText(QString::number(hCur * 100.0 / hMax) + "%");
    }
}
void ReadingView::notepad()
{
    QDialog* qdialog = new QDialog();
    qdialog->setMinimumSize(280,350);
    NotePadWidget* qwidget = new NotePadWidget(qdialog);
    qdialog->setAttribute(Qt::WA_DeleteOnClose);
    qdialog->exec();
}

void ReadingView::showKeyword()
{
    KeywordDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {}
}


void ReadingView::saveProgress()
{
    ConfigManager::instance().setLastPage(m_currentBook.filePath, ui->textBrowser->verticalScrollBar()->value());
    ConfigManager::instance().setTotalPage(m_currentBook.filePath, ui->textBrowser->verticalScrollBar()->maximum());
}
