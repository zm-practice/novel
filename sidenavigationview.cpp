
#include "sidenavigationview.h"
#include "ui_sidenavigationview.h"
#include <QFileDialog> // 用于文件对话框
#include "bookmanager.h"
#include <QMessageBox> // <--- 在文件顶部添加这个头文件
#include "bookeditdialog.h"
#include "readerwindow.h"
SideNavigationView::SideNavigationView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SideNavigationView)
{
    // 这行代码已经创建了 bookListWidget 和所有按钮
    ui->setupUi(this);

    // --- 我们只需要配置 bookListWidget 的视觉属性 ---
    ui->bookListWidget->setViewMode(QListWidget::IconMode);
    ui->bookListWidget->setIconSize(QSize(120, 160));
    ui->bookListWidget->setGridSize(QSize(150, 190));
    ui->bookListWidget->setMovement(QListWidget::Static);
    ui->bookListWidget->setWordWrap(true);

    // 设置默认显示的页面
    ui->contentStackedWidget->setCurrentWidget(ui->bookshelfPage);


    // 创建悬浮时钟按钮

    ui->timerButton->setToolTip("打开悬浮时钟窗口");
    ui->timerButton->setFixedSize(80, 30); // 设置固定大小
    ui->timerButton->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border-radius: 4px; }");


    // 连接按钮的clicked信号到showTimerWindow槽
    connect(ui->timerButton, &QPushButton::clicked, this, &SideNavigationView::showTimerWindow);


}
//time
void SideNavigationView::showTimerWindow()
{
    // 创建新窗口
    m_timerWindow = new TimerWindow(nullptr); // 使用nullptr而不是this，避免父窗口影响
    m_timerWindow->setAttribute(Qt::WA_DeleteOnClose); // 关闭时自动删除

    // 设置窗口标志
    m_timerWindow->setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);

    // 显示窗口
    m_timerWindow->show();

    // 计算并设置位置
    const int MARGIN = 20;
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int x = screenGeometry.width() - m_timerWindow->width() - MARGIN;
    int y = screenGeometry.height() - m_timerWindow->height() - MARGIN;
    m_timerWindow->move(x, y);

    // 确保窗口在前台显示
    m_timerWindow->raise();
    m_timerWindow->activateWindow();
}

SideNavigationView::~SideNavigationView()
{
    delete ui;
}

// 公共接口的实现
void SideNavigationView::setBookManager(BookManager *manager)
{
    m_bookManager = manager;
    // 连接 BookManager 的数据变化信号到我们的刷新槽
    connect(m_bookManager, &BookManager::booksChanged, this, &SideNavigationView::refreshBookList);

    // 第一次设置时，立即刷新一次列表
    refreshBookList();
}

// 槽函数：刷新书架列表
void SideNavigationView::refreshBookList()
{
    if (!m_bookManager) return;

    ui->bookListWidget->clear(); // 清空旧列表
    const auto& books = m_bookManager->getBooks();
    for(const auto& book : books) {
        ui->bookListWidget->addItem(new QListWidgetItem(QIcon(book.coverPath), book.title));
    }
}

// --- 自动连接的槽函数实现 ---

void SideNavigationView::on_bookshelfButton_clicked()
{
    ui->contentStackedWidget->setCurrentWidget(ui->bookshelfPage);
}

void SideNavigationView::on_myProfileButton_clicked()
{
    ui->contentStackedWidget->setCurrentWidget(ui->myProfilePage);
}

void SideNavigationView::on_addButton_clicked()
{
    if (!m_bookManager) return;

    // 1. 让用户选择小说文件
    QString filePath = QFileDialog::getOpenFileName(this, "选择小说文件", "", "Text Files (*.txt)");
    if (filePath.isEmpty()) {
        return; // 用户取消了选择
    }

    // 2. 根据文件路径，创建一个临时的 BookInfo 对象来存储默认信息
    BookInfo defaultInfo;
    defaultInfo.filePath = filePath;
    defaultInfo.title = QFileInfo(filePath).baseName(); // 用文件名作为默认标题
    // 这里可以设置一个默认的封面路径，如果用户不选就用这个
    defaultInfo.coverPath = ":/树木.jpg";

    // 3. 创建并配置我们的编辑对话框
    BookEditDialog dialog(this);
    dialog.setWindowTitle("编辑书籍信息");
    dialog.setBookInfo(defaultInfo);

    // 4. 以模态方式显示对话框，并等待用户操作
    //    exec() 会阻塞程序，直到用户点击 OK 或 Cancel
    if (dialog.exec() == QDialog::Accepted) {
        // 5. 如果用户点击了 "OK"，获取最终的书籍信息
        BookInfo finalBookInfo = dialog.getBookInfo();

        // 6. 将最终信息交给 BookManager 来处理
        m_bookManager->addBook(finalBookInfo);
    }
}

void SideNavigationView::on_deleteButton_clicked()
{
    if (!m_bookManager) return;

    QListWidgetItem *currentItem = ui->bookListWidget->currentItem();
    if (!currentItem) {
        // 如果没有项目被选中，可以弹个提示
        QMessageBox::warning(this, "提示", "请先选择一本要删除的书籍。");
        return;
    }

    // 获取当前选中项的索引
    int currentIndex = ui->bookListWidget->row(currentItem);

    // --- ★★★ 添加确认对话框 ★★★ ---
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "确认删除",
                                  QString("您确定要从书架移除《%1》吗？此操作不可撤销。").arg(currentItem->text()),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 只有当用户点击 "Yes" 时，才执行删除操作
        m_bookManager->deleteBook(currentIndex);
    }
}


// sidenavigationview.cpp (on_bookListWidget_itemDoubleClicked 函数)

void SideNavigationView::on_bookListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (!m_bookManager || !item) return;
    // ...
    // const BookInfo& selectedBook = m_bookManager->getBooks().at(index);

    // ★★★ 添加这行代码来声明和获取 index ★★★
        int index = ui->bookListWidget->row(item);

    if (index >= 0 && index < m_bookManager->getBooks().size()) {
        const BookInfo& selectedBook = m_bookManager->getBooks().at(index);

    // ★★★ 创建 ReaderWindow 时，传入 m_bookManager ★★★
    ReaderWindow *reader = new ReaderWindow(selectedBook, m_bookManager, this);
    reader->setAttribute(Qt::WA_DeleteOnClose);
    reader->show();
    }
}
// void SideNavigationView::on_bookItem_doubleClicked(QListWidgetItem *item)
// {
//     if (!m_bookManager) return;

//     int index = ui->bookListWidget->row(item);
//     if (index >= 0 && index < m_bookManager->getBooks().size()) {
//         const BookInfo& selectedBook = m_bookManager->getBooks().at(index);

//         // 创建并显示阅读器窗口
//         // 我们把它 new 在堆上，并设置 Qt::WA_DeleteOnClose 属性
//         // 这样当用户关闭窗口时，它会自动销毁，避免内存泄漏
//         ReaderWindow *reader = new ReaderWindow(selectedBook, this);
//         reader->setAttribute(Qt::WA_DeleteOnClose);
//         reader->show();
//     }
// }

