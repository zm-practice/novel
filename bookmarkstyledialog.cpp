#include "bookmarkstyledialog.h"
#include "ui_bookmarkstyledialog.h"
#include <QColorDialog>
#include <QFileDialog>
#include <QTextCharFormat>

BookmarkStyleDialog::BookmarkStyleDialog(const BookmarkInfo& currentStyle, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookmarkStyleDialog),
    m_style(currentStyle)
{
    ui->setupUi(this);
    setWindowTitle("编辑书签样式");

    // --- 初始化UI控件以反映当前样式 ---
    // 填充下划线样式下拉框
    ui->underlineStyleCombo->addItem("无", QTextCharFormat::NoUnderline);
    ui->underlineStyleCombo->addItem("单实线", QTextCharFormat::SingleUnderline);
    ui->underlineStyleCombo->addItem("虚线", QTextCharFormat::DashUnderline);
    ui->underlineStyleCombo->addItem("波浪线", QTextCharFormat::WaveUnderline);
    int index = ui->underlineStyleCombo->findData(m_style.underlineStyle);
    if (index != -1) ui->underlineStyleCombo->setCurrentIndex(index);

    // 更新颜色按钮的预览
    updateColorButton(ui->bgColorButton, m_style.backgroundColor);
    updateColorButton(ui->ulColorButton, m_style.underlineColor);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

BookmarkStyleDialog::~BookmarkStyleDialog()
{
    delete ui;
}

// --- 按钮点击事件 ---
void BookmarkStyleDialog::on_bgColorButton_clicked()
{
    QColor color = QColorDialog::getColor(m_style.backgroundColor, this, "选择背景颜色");
    if (color.isValid()) {
        m_style.backgroundColor = color;
        updateColorButton(ui->bgColorButton, color);
    }
}

void BookmarkStyleDialog::on_ulColorButton_clicked()
{
    QColor color = QColorDialog::getColor(m_style.underlineColor, this, "选择下划线颜色");
    if (color.isValid()) {
        m_style.underlineColor = color;
        updateColorButton(ui->ulColorButton, color);
    }
}

void BookmarkStyleDialog::on_iconButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "选择图标", "", "Images (*.png)");
    if (!path.isEmpty()) {
        m_style.iconPath = path;
        ui->iconButton->setIcon(QIcon(path));
    }
}

// --- 核心函数：获取最终选择的样式 ---
BookmarkInfo BookmarkStyleDialog::getSelectedStyle() const
{
    BookmarkInfo finalStyle = m_style;
    finalStyle.underlineStyle = ui->underlineStyleCombo->currentData().toInt();
    return finalStyle;
}

// --- 辅助函数：更新颜色按钮的背景色作为预览 ---
void BookmarkStyleDialog::updateColorButton(QPushButton *button, const QColor &color)
{
    button->setText(color.name());
    button->setStyleSheet(QString("background-color: %1").arg(color.name()));
}
