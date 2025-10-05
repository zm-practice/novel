#include "readerwindow.h"
#include "ui_readerwindow.h"
#include "settingsmanager.h"
// ★★★ 包含所有需要手动创建的控件的头文件 ★★★
#include <QFontComboBox>
#include <QSpinBox>
#include <QPushButton>
#include <QToolButton> // 使用 ToolButton 来创建带菜单的按钮
#include <QMenu>       // 菜单
#include <QAction>     // 菜单项
#include <QWidgetAction> // 可以把QWidget放入菜单的Action
#include <QFileDialog>
ReaderWindow::ReaderWindow(const BookInfo& book, BookManager* bookManager, QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ReaderWindow)
{
    ui->setupUi(this);
    setWindowTitle(book.title); // 设置窗口标题为书名

    // 将书籍信息传递给内部的 readingView
    ui->readingView->loadBook(book);

    // =========================================================
    // ===       动态创建和添加设置控件到工具栏            ===
    // =========================================================
    auto& settings = SettingsManager::instance();

    // 1. 手动创建UI控件
    QFontComboBox *fontComboBox = new QFontComboBox(ui->settingsToolBar);
    QSpinBox *fontSizeSpinBox = new QSpinBox(ui->settingsToolBar);
    QPushButton *nightModeButton = new QPushButton("夜间模式", ui->settingsToolBar);
    nightModeButton->setCheckable(true); // 设为可切换状态

    // 2. 将这些控件添加到工具栏中
    ui->settingsToolBar->addWidget(fontComboBox);
    ui->settingsToolBar->addWidget(fontSizeSpinBox);
    ui->settingsToolBar->addSeparator(); // 添加一个分隔符
    ui->settingsToolBar->addWidget(nightModeButton);

    // 3. 初始化控件状态
    fontComboBox->setCurrentFont(QFont(settings.fontFamily()));
    fontSizeSpinBox->setMinimum(9);
    fontSizeSpinBox->setMaximum(72);
    fontSizeSpinBox->setValue(settings.fontSize());
    nightModeButton->setChecked(settings.isNightMode());

    // 4. 连接信号和槽
    connect(fontComboBox, &QFontComboBox::currentFontChanged, this, [&](const QFont& font){
        settings.setFontFamily(font.family());
    });
    connect(fontSizeSpinBox, &QSpinBox::valueChanged, this, [&](int size){
        settings.setFontSize(size);
    });
    connect(nightModeButton, &QPushButton::toggled, this, [&](bool checked){
        settings.setNightMode(checked);
    });

    // 连接 SettingsManager 的 settingsChanged 信号到一个 Lambda 函数
    // 这个 Lambda 负责在每次设置变更时更新按钮的文本
    connect(&settings, &SettingsManager::settingsChanged, this, [&, nightModeButton](){
        if (settings.isNightMode()) {
            nightModeButton->setText("日间模式");
        } else {
            nightModeButton->setText("夜间模式");
        }
    });

    // =========================================================
        // ===            添加背景选择菜单                     ===
        // =========================================================

        // 1. 创建一个带菜单的 ToolButton
        QToolButton *backgroundButton = new QToolButton(ui->settingsToolBar);
    backgroundButton->setText("背景");
    backgroundButton->setPopupMode(QToolButton::InstantPopup); // 点击立即弹出菜单

    // 2. 创建菜单
    QMenu *backgroundMenu = new QMenu(backgroundButton);

    // 3. 创建菜单项 (Action)
    QAction *defaultAction = new QAction("默认", this);
    QAction *paperYellowAction = new QAction("米黄", this);
    QAction *greenBeanAction = new QAction("豆绿", this);
    QAction *customImageAction = new QAction("自定义...", this);

    // 4. 将 Action 添加到菜单
    backgroundMenu->addAction(defaultAction);
    backgroundMenu->addAction(paperYellowAction);
    backgroundMenu->addAction(greenBeanAction);
    backgroundMenu->addSeparator();
    backgroundMenu->addAction(customImageAction);

    // 5. 将菜单设置给按钮
    backgroundButton->setMenu(backgroundMenu);

    // 6. 将按钮添加到工具栏 (使用一个弹簧把它推到右边)
    QWidget* spacer = new QWidget();
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    ui->settingsToolBar->addWidget(spacer);
    ui->settingsToolBar->addWidget(backgroundButton);

    // 7. 连接菜单项的信号到 SettingsManager
    connect(defaultAction, &QAction::triggered, this, [&](){
        settings.setBackgroundMode(SettingsManager::Default);
    });
    connect(paperYellowAction, &QAction::triggered, this, [&](){
        settings.setBackgroundMode(SettingsManager::PaperYellow);
    });
    connect(greenBeanAction, &QAction::triggered, this, [&](){
        settings.setBackgroundMode(SettingsManager::GreenBean);
    });
    connect(customImageAction, &QAction::triggered, this, [&](){
        QString imagePath = QFileDialog::getOpenFileName(this, "选择背景图片", "", "Image Files (*.png *.jpg *.jpeg)");
        if (!imagePath.isEmpty()) {
            settings.setCustomImagePath(imagePath);
            settings.setBackgroundMode(SettingsManager::CustomImage); // 别忘了切换模式！
        }
    });

    // ★★★ 将指针传递给内部的 readingView ★★★
    ui->readingView->setBookManager(bookManager);
    ui->readingView->loadBook(book);
}

ReaderWindow::~ReaderWindow()
{
    delete ui;
}
