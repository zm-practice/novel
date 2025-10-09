// #include "readingview.h"
// #include "ui_readingview.h"
// #include "settingsmanager.h" // ★★★ 包含 SettingsManager 头文件 ★★★
// #include"bookmanager.h"
// #include "bookmarkstyledialog.h" // 包含头文件
// #include <QFile>
// #include <QTextStream>
// #include <QListWidgetItem>
// #include <QRegularExpression> // 2. 包含 Qt 6 的正则表达式头文件
// #include <QMenu> // 包含 QMenu



// ReadingView::ReadingView(QWidget *parent) :
//     QWidget(parent),
//     ui(new Ui::ReadingView)
// {
//     ui->setupUi(this);
//     // ★★★ 核心连接：监听全局设置的变化 ★★★
//     connect(&SettingsManager::instance(), &SettingsManager::settingsChanged, this, &ReadingView::applySettings);
// //书签！！！
//     // ★★★ 开启自定义右键菜单功能 ★★★
//     ui->textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
//     connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, &ReadingView::showContextMenu);


//     // ★★★ 在构造函数中初始化书签格式 ★★★
//     m_bookmarkFormat.setBackground(QColor(255, 255, 0, 50)); // 半透明黄色背景
//     m_bookmarkFormat.setUnderlineStyle(QTextCharFormat::DashUnderline); // 虚线
//     m_bookmarkFormat.setUnderlineColor(Qt::gray);
//     // 第一次创建时，立即应用一次当前设置
//     applySettings();
// }

// ReadingView::~ReadingView()
// {
//     delete ui;
// }

// void ReadingView::loadBook(const BookInfo &book)
// {
//     //书签！！！
//      m_currentBook = book; // ★★★ 记下当前的书籍 ★★★
//     QFile file(book.filePath);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         ui->textBrowser->setText("错误：无法打开文件 " + book.filePath);
//         return;
//     }

//     QTextStream in(&file);
//     // 在 Qt 6 中，QTextStream 默认使用 UTF-8，不再需要 setCodec

//     QString content = in.readAll();
//     file.close();

//     parseChapters(content);
// }

// void ReadingView::parseChapters(const QString &content)
// {
//     ui->chaptersListWidget->clear();
//     m_chapterContents.clear();

//     // 使用 Qt 6 的 QRegularExpression
//     QStringList chapters = content.split(QRegularExpression("第[〇一二三四五六七八九十百千万\\d]+章"));

//     if (chapters.size() <= 1) { // 如果没有匹配到章节分隔符
//         ui->chaptersListWidget->addItem("全文");
//         m_chapterContents.append(content);
//     } else {
//         // 处理可能存在的序言部分（在第一个“第...章”之前的内容）
//         if(!chapters.at(0).trimmed().isEmpty()) {
//             ui->chaptersListWidget->addItem("序章");
//             m_chapterContents.append(chapters.at(0));
//         }

//         // 提取所有的章节标题，例如 "第一章", "第二章"...
//         QStringList chapterTitles;
//         QRegularExpression rx("第[〇一二三四五六七八九十百千万\\d]+章");
//         QRegularExpressionMatchIterator i = rx.globalMatch(content);
//         while (i.hasNext()) {
//             QRegularExpressionMatch match = i.next();
//             chapterTitles.append(match.captured(0));
//         }

//         // 遍历分割后的章节内容
//         for (int i = 1; i < chapters.size(); ++i) {
//             // 将提取出的章节标题与内容一一对应
//             if(i - 1 < chapterTitles.size()){
//                 ui->chaptersListWidget->addItem(chapterTitles.at(i-1));
//             } else {
//                 // 如果内容比标题多，则使用通用标题
//                 ui->chaptersListWidget->addItem(QString("章节 %1").arg(i));
//             }
//             // `split` 之后，`chapters` 列表的第一项是序言，所以内容从索引1开始
//             m_chapterContents.append(chapters.at(i));
//         }
//     }

//     // 默认显示第一项内容
//     if (ui->chaptersListWidget->count() > 0) {
//         ui->chaptersListWidget->setCurrentRow(0);
//         // 直接调用槽函数来显示内容，避免代码重复
//         on_chaptersListWidget_itemClicked(ui->chaptersListWidget->item(0));
//     }
// }

// void ReadingView::on_chaptersListWidget_itemClicked(QListWidgetItem *item)
// {
//     if (!item) return; // 安全检查

//     int index = ui->chaptersListWidget->row(item);
// //书签！！！！
//     m_currentChapterIndex = index; // ★★★ 记下当前的章节 ★★★
//     if (index >= 0 && index < m_chapterContents.size()) {
//         m_currentChapterIndex = index;
//         // 先设置纯文本
//         ui->textBrowser->setText(m_chapterContents.at(index));
//         // 然后再应用书签格式
//         applyBookmarksToChapter(index);
//     }
// }


// // ★★★ 实现 applySettings 槽函数 ★★★
// void ReadingView::applySettings()
// {
//     auto& settings = SettingsManager::instance();

//     // 1. 设置字体
//     QFont font(settings.fontFamily());
//     font.setPointSize(settings.fontSize());
//     ui->textBrowser->setFont(font);
//      ui->chaptersListWidget->setFont(font); // ★★★ 新增：为目录设置相同的字体 ★★★

//     // 2. ★★★ 全新的背景和颜色设置逻辑 ★★★
//     QString browserStyle, listStyle;

//     if (settings.backgroundMode() == SettingsManager::CustomImage && !settings.customImagePath().isEmpty())
//     { // --- 图片背景模式 ---
//         // ★★★ 核心修正 ★★★
//         QString imagePath = settings.customImagePath();
//         // 将 Windows 风格的反斜杠替换为 CSS 兼容的正斜杠
//         imagePath.replace("\\", "/");

//         browserStyle = QString(
//                            "QTextBrowser {"
//                            // 不再使用 QUrl::fromLocalFile，直接使用修正后的路径字符串
//                            "  border-image: url(%1) 0 0 0 0 stretch stretch;"
//                            "  background-color: transparent;"
//                            "  color: %2;"
//                            "}"
//                            ).arg(imagePath).arg(settings.textColor().name());

//         listStyle = QString(
//                         "QListWidget {"
//                         "  border-image: url(%1) 0 0 0 0 stretch stretch;"
//                         "  background-color: transparent;"
//                         "  color: %2;"
//                         "}"
//                         "QListWidget::item:selected { background-color: #4a6984; }"
//                         ).arg(imagePath).arg(settings.textColor().name());
//     }
//     else
//     {
//         // --- 纯色背景模式 (和以前一样) ---
//         browserStyle = QString("QTextBrowser { background-color: %1; color: %2; border: none; }")
//                            .arg(settings.backgroundColor().name())
//                            .arg(settings.textColor().name());

//         listStyle = QString(
//                         "QListWidget { background-color: %1; color: %2; border: none; }"
//                         "QListWidget::item:selected { background-color: #4a6984; }"
//                         ).arg(settings.backgroundColor().name()).arg(settings.textColor().name());
//     }

// //书签！！！
//     // ★★★ 新增：定义书签的样式 ★★★
//     QString bookmarkStyle =
//         "span.bookmark {"
//         "  background-color: rgba(255, 255, 0, 0.2);" // 半透明黄色背景作为“方框”
//         "  border-bottom: 1px dashed #c0c0c0;"
//         "}";

//     // 将新样式附加到已有的样式表上
//     browserStyle += bookmarkStyle;
//     listStyle += bookmarkStyle; // 也可以给目录加上

//     ui->textBrowser->setStyleSheet(browserStyle);
//     ui->chaptersListWidget->setStyleSheet(listStyle);
// }




// //书签！！！！！
// // --- 实现右键菜单槽函数 ---
// // readingview.cpp
// void ReadingView::showContextMenu(const QPoint &pos)
// {
//     QTextCursor cursor = ui->textBrowser->cursorForPosition(pos);
//     bool inBookmark = cursor.charFormat().background() == m_bookmarkFormat.background(); // 通过背景色判断

//     QMenu contextMenu(this);

//     if (inBookmark) {
//         QAction *removeBookmarkAction = new QAction("取消书签", this);
//         // ★★★ 新增：“编辑样式...” 菜单项 ★★★
//         QAction *editStyleAction = new QAction("编辑样式...", this);
//         editStyleAction->setData(cursor.position());
//         connect(editStyleAction, &QAction::triggered, this, &ReadingView::changeBookmarkStyle);
//         contextMenu.addAction(editStyleAction);
//         // 将当前光标位置关联到Action
//         removeBookmarkAction->setData(cursor.position());
//         connect(removeBookmarkAction, &QAction::triggered, this, &ReadingView::removeBookmark);
//         contextMenu.addAction(removeBookmarkAction);
//     }
//     else if (ui->textBrowser->textCursor().hasSelection()) {
//         QAction *addBookmarkAction = new QAction("添加书签", this);
//         connect(addBookmarkAction, &QAction::triggered, this, &ReadingView::addBookmarkFromSelection);
//         contextMenu.addAction(addBookmarkAction);
//     }

//     contextMenu.exec(ui->textBrowser->mapToGlobal(pos));
// }



// // --- 实现添加书签的逻辑 ---
// // readingview.cpp

// void ReadingView::addBookmarkFromSelection()
// {
//     if (!m_bookManager || m_currentBook.filePath.isEmpty()) return;

//     // 1. 获取代表用户选区的光标
//     QTextCursor cursor = ui->textBrowser->textCursor();
//     if (!cursor.hasSelection()) return;

//     // 2. 更新数据模型
//     QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);
//     BookmarkInfo newBookmark;
//     newBookmark.chapterIndex = m_currentChapterIndex;
//     newBookmark.startPos = cursor.selectionStart();
//     newBookmark.endPos = cursor.selectionEnd();
//     bookmarks.append(newBookmark);
//     m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

//     // 3. --- 立即更新UI，渲染新添加的书签 ---

//     // a. 对当前选区应用背景和下划线格式
//     cursor.mergeCharFormat(m_bookmarkFormat);

//     // b. 在选区的开头插入羽毛图标
//     //    (注意：插入图片会改变后续文本的位置，但因为我们已经保存了原始位置，所以没关系)
//     cursor.setPosition(newBookmark.startPos);
//     cursor.insertImage(QImage(":/icon1/icons/羽毛笔.png").scaled(16, 16, Qt::KeepAspectRatio));

//     // c. 清除选区，让光标停在书签末尾，避免文本保持选中状态
//     cursor.setPosition(newBookmark.endPos + 1); // +1 是因为我们插入了一个图片字符
//     ui->textBrowser->setTextCursor(cursor);
// }


// // ★★★ 实现新增的接口 ★★★
//     void ReadingView::setBookManager(BookManager *manager)
// {
//     m_bookManager = manager;
// }


// // readingview.cpp

// void ReadingView::removeBookmark()
// {
//     if (!m_bookManager || m_currentBook.filePath.isEmpty()) return;

//     QAction* action = qobject_cast<QAction*>(sender());
//     if (!action) return;

//     // 从 Action 中获取我们之前存入的、右键点击时的光标位置
//     int position = action->data().toInt();

//     // 1. --- 更新数据模型 ---
//     QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);
//     int bookmarkToRemove = -1;

//     // 遍历书签列表，找到那个包含了我们点击位置(position)的书签
//     for (int i = 0; i < bookmarks.size(); ++i) {
//         const auto& bookmark = bookmarks[i];
//         if (bookmark.chapterIndex == m_currentChapterIndex &&
//             position >= bookmark.startPos && position < bookmark.endPos)
//         {
//             bookmarkToRemove = i;
//             break; // 找到后就跳出循环
//         }
//     }

//     if (bookmarkToRemove != -1) {
//         // 如果找到了匹配的书签
//         bookmarks.removeAt(bookmarkToRemove); // 从列表中移除
//         m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks); // 保存更新后的列表

//         // 2. --- 立即更新UI ---
//         // 重新加载并渲染当前章节是目前最简单、最可靠的方法，
//         // 它会自动清除掉已删除书签的格式。
//         on_chaptersListWidget_itemClicked(ui->chaptersListWidget->item(m_currentChapterIndex));
//     }
// }




// void ReadingView::applyBookmarksToChapter(int chapterIndex)
// {
//     if (!m_bookManager) return;

//     QTextCursor cursor(ui->textBrowser->document());
//     const auto& bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);

//     for (const auto& bookmark : bookmarks) {
//         if (bookmark.chapterIndex == chapterIndex) {
//             // 1. 创建一个临时的格式对象
//             QTextCharFormat format;
//             format.setBackground(bookmark.backgroundColor);
//             format.setUnderlineStyle(static_cast<QTextCharFormat::UnderlineStyle>(bookmark.underlineStyle));
//             format.setUnderlineColor(bookmark.underlineColor);

//             // 2. 应用格式
//             cursor.setPosition(bookmark.startPos, QTextCursor::MoveAnchor);
//             cursor.setPosition(bookmark.endPos, QTextCursor::KeepAnchor);
//             cursor.mergeCharFormat(format);

//             // 3. 插入图标
//             if (!bookmark.iconPath.isEmpty()) {
//                 cursor.setPosition(bookmark.startPos);
//                 cursor.insertImage(QImage(bookmark.iconPath).scaled(16, 16, Qt::KeepAspectRatio));
//             }
//         }
//     }
//     cursor.clearSelection();
// }
// void ReadingView::changeBookmarkStyle()
// {
//     if (!m_bookManager) return;

//     QAction* action = qobject_cast<QAction*>(sender());
//     if (!action) return;

//     // 1. 从 Action 中获取用户右键点击时的文本位置
//     int position = action->data().toInt();

//     // 2. 找到当前点击的书签
//     QList<BookmarkInfo> bookmarks = m_bookManager->getBookmarks(m_currentBook.filePath);
//     int targetIndex = -1; // 用于记录我们找到的书签在列表中的索引

//     for (int i = 0; i < bookmarks.size(); ++i) {
//         if (bookmarks[i].chapterIndex == m_currentChapterIndex &&
//             position >= bookmarks[i].startPos && position < bookmarks[i].endPos)
//         {
//             targetIndex = i;
//             break;
//         }
//     }

//     if (targetIndex == -1) {
//         // 如果因为某些原因没找到，就直接返回
//         return;
//     }

//     // 3. 创建并显示样式编辑对话框
//     //    ★★★ 关键：把当前书签 (bookmarks[targetIndex]) 的样式传给对话框 ★★★
//     BookmarkStyleDialog dialog(bookmarks[targetIndex], this);

//     if (dialog.exec() == QDialog::Accepted) {
//         // 4. 如果用户点击了 "OK"...

//         // a. 从对话框获取用户选择的新样式
//         BookmarkInfo newStyle = dialog.getSelectedStyle();

//         // b. 更新 bookmarks 列表中的那一个书签
//         bookmarks[targetIndex] = newStyle;

//         // c. 将整个更新后的列表交还给 BookManager 保存
//         m_bookManager->addBookmarks(m_currentBook.filePath, bookmarks);

//         // d. 刷新视图以显示最新的样式
//         on_chaptersListWidget_itemClicked(ui->chaptersListWidget->item(m_currentChapterIndex));
//     }
// }









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

// ReadingView::ReadingView(QWidget *parent) :
//     QWidget(parent),
//     ui(new Ui::ReadingView)
// {
//     ui->setupUi(this);
//     connect(&SettingsManager::instance(), &SettingsManager::settingsChanged, this, &ReadingView::applySettings);
//     ui->textBrowser->setContextMenuPolicy(Qt::CustomContextMenu);
//     connect(ui->textBrowser, &QTextBrowser::customContextMenuRequested, this, &ReadingView::showContextMenu);
//     applySettings();

//     // =========================================================
//     // ===            初始化自动阅读控件                   ===
//     // =========================================================
//     auto& settings = SettingsManager::instance();

//     // 1. 初始化定时器 (和之前一样)
//     m_autoScrollTimer = new QTimer(this);
//     connect(m_autoScrollTimer, &QTimer::timeout, this, &ReadingView::onAutoScrollTimerTimeout);

//     // 2. 初始化UI控件的状态，使其反映当前保存的设置
//     ui->speedSlider->setRange(10, 500);
//     ui->speedSlider->setInvertedAppearance(true);
//     ui->speedSlider->setValue(settings.autoScrollSpeed());

//     // 3. 设置定时器的初始速度
//     setAutoScrollSpeed(settings.autoScrollSpeed());

//     // 4. (可选) 监听全局设置变化，如果其他窗口改变了速度，这里也同步更新滑块
//     connect(&settings, &SettingsManager::settingsChanged, this, [this](){
//         auto& s = SettingsManager::instance();
//         if(ui->speedSlider->value() != s.autoScrollSpeed()){
//             ui->speedSlider->setValue(s.autoScrollSpeed());
//         }
//     });

// //在线查词！！！
//     // 1. 创建服务实例
//     m_dictionaryService = new DictionaryService(this);

//     // 2. 连接服务的信号到我们的槽
//     connect(m_dictionaryService, &DictionaryService::querySuccess, this, &ReadingView::onQuerySuccess);
//     connect(m_dictionaryService, &DictionaryService::queryError, this, &ReadingView::onQueryError);
// }

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

    m_searchNextButton = new QPushButton("下一个", this);
    connect(m_searchNextButton, &QPushButton::clicked, this, &ReadingView::findNext);

    m_searchPrevButton = new QPushButton("上一个", this);
    connect(m_searchPrevButton, &QPushButton::clicked, this, &ReadingView::findPrevious);

    // 创建章节切换按钮
    m_toggleChapterButton = new QPushButton("隐藏章节", this);
    connect(m_toggleChapterButton, &QPushButton::clicked, this, &ReadingView::toggleChapterList);

    // 将搜索控件添加到布局中
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchLayout->addWidget(m_searchLineEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_searchNextButton);
    searchLayout->addWidget(m_searchPrevButton);
    searchLayout->addStretch();
    searchLayout->addWidget(m_toggleChapterButton);

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


//在线查词！！！
// --- 实现鼠标双击事件 ---
// void ReadingView::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if (ui->textBrowser->rect().contains(event->pos())) {
//         QTextCursor cursor = ui->textBrowser->cursorForPosition(event->pos());

//         // ★★★ 智能选择：如果是中文，选一个字；如果是英文，选一个词 ★★★
//         QString selectedText;
//         QChar charUnderCursor = ui->textBrowser->document()->characterAt(cursor.position());
//         if (charUnderCursor.unicode() >= 0x4e00 && charUnderCursor.unicode() <= 0x9fa5)  {
//             cursor.select(QTextCursor::WordUnderCursor); // WordUnderCursor 对中文会选中一个字
//             selectedText = cursor.selectedText();
//         } else {
//             cursor.select(QTextCursor::WordUnderCursor);
//             selectedText = cursor.selectedText();
//         }

//         if (!selectedText.trimmed().isEmpty()) {
//             WordDefinition pendingDef;
//             pendingDef.query = selectedText;
//             pendingDef.phonetic = "查询中...";
//             m_tooltipPopup->showDefinition(pendingDef, QCursor::pos());

//             // ★★★ 调用新的 query 函数 ★★★
//             m_dictionaryService->query(selectedText);

//         }
//     }
//     QWidget::mouseDoubleClickEvent(event);
// }


// void ReadingView::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     if (!m_tooltipPopup) {
//         m_tooltipPopup = new TooltipPopup(this);
//     }

//     QTextCursor cursor = ui->textBrowser->cursorForPosition(event->pos());
//     cursor.select(QTextCursor::WordUnderCursor);
//     QString selectedText = cursor.selectedText().trimmed();

//     if (!selectedText.isEmpty()) {
//         WordDefinition pendingDef;
//         pendingDef.displayWord = selectedText;
//         pendingDef.phonetic = "查询中...";
//         m_tooltipPopup->showDefinition(pendingDef, QCursor::pos());
//         m_dictionaryService->query(selectedText);
//     }
//     QWidget::mouseDoubleClickEvent(event);
// }


// --- ★★★ 实现一个全新的、绝对可靠的 mouseDoubleClickEvent ★★★ ---
// void ReadingView::mouseDoubleClickEvent(QMouseEvent *event)
// {
//     // 1. 判断双击是否发生在 QTextBrowser 内部
//     if (!ui->textBrowser->rect().contains(event->pos())) {
//         // 如果点在了别处，执行默认行为并返回
//         QWidget::mouseDoubleClickEvent(event);
//         return;
//     }

//     // 2. 获取双击位置的光标
//     QTextCursor cursor = ui->textBrowser->cursorForPosition(event->pos());

//     // 3. 让光标自动选中其下的单词/汉字
//     cursor.select(QTextCursor::WordUnderCursor);
//     QString selectedText = cursor.selectedText().trimmed();

//     // 4. 如果确实选中了内容
//     if (!selectedText.isEmpty()) {
//         qDebug() << "双击选中内容:" << selectedText; // 添加调试信息

//         // 5. ★★★ 关键：确保弹窗实例存在 ★★★
//         // 如果 m_tooltipPopup 是空的，就创建一个新的实例
//         if (!m_tooltipPopup) {
//             m_tooltipPopup = new TooltipPopup(this);
//         }

//         // 6. 准备一个“查询中...”的提示信息
//         WordDefinition pendingDef;
//         pendingDef.displayWord = selectedText;
//         pendingDef.phonetic = "查询中...";

//         // 7. 显示弹窗并发起查询
//         m_tooltipPopup->showDefinition(pendingDef, event->globalPosition().toPoint()); // 使用事件提供的全局坐标
//         m_dictionaryService->query(selectedText);

//         // 8. 接受事件，阻止 QTextBrowser 执行它自己的默认双击行为
//         event->accept();
//     } else {
//         // 如果没有选中任何内容，执行默认行为
//         QWidget::mouseDoubleClickEvent(event);
//     }
// }


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

    // 根据主题模式设置高亮颜色
    QColor highlightColor;
    QColor highlightTextColor;

    if (settings.isNightMode()) {
        // 夜间模式：使用明亮的黄色高亮，深色文字，确保在深色背景下可见
        highlightColor = QColor("#FFFF00");  // 明亮的黄色背景
        highlightTextColor = QColor("#000000");  // 黑色文字
    } else {
        // 日间模式：使用深色高亮，白色文字，确保在浅色背景下可见
        highlightColor = QColor("#FF4500");  // 橙红色背景
        highlightTextColor = QColor("#FFFFFF");  // 白色文字
    }

    // 设置选中文本的格式
    QTextCharFormat format;
    format.setBackground(highlightColor);
    format.setForeground(highlightTextColor);

    // 创建一个新的光标来设置格式
    QTextCursor highlightCursor = cursor;
    highlightCursor.mergeCharFormat(format);

    // 确保找到的文本可见
    ui->textBrowser->ensureCursorVisible();

    // 更新当前位置
    m_currentPosition = ui->textBrowser->verticalScrollBar()->value();
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
