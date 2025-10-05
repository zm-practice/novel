#include "bookeditdialog.h"
#include "ui_bookeditdialog.h"
#include <QFileDialog>

BookEditDialog::BookEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookEditDialog)
{
    ui->setupUi(this);

    // 连接 OK 和 Cancel 按钮到对话框的标准槽
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

BookEditDialog::~BookEditDialog()
{
    delete ui;
}

// 当对话框打开前，用这个函数来填充默认信息
void BookEditDialog::setBookInfo(const BookInfo &info)
{
    m_bookInfo = info;
    ui->titleEdit->setText(m_bookInfo.title);
    // 我们暂时没有作者信息，所以 authorEdit 为空
}

// 当用户点击 "OK" 后，用这个函数来获取用户修改后的最终信息
BookInfo BookEditDialog::getBookInfo() const
{
    BookInfo finalInfo = m_bookInfo; // 从原始信息开始
    finalInfo.title = ui->titleEdit->text();
    // finalInfo.author = ui->authorEdit->text(); // 如果 BookInfo 有 author 字段
    return finalInfo;
}

void BookEditDialog::on_coverButton_clicked()
{
    QString coverPath = QFileDialog::getOpenFileName(this, "选择封面图片", "", "Image Files (*.png *.jpg *.jpeg)");
    if (!coverPath.isEmpty()) {
        m_bookInfo.coverPath = coverPath;
        // (可选) 可以在这里添加一个 QLabel 来预览封面
        ui->coverButton->setText(QFileInfo(coverPath).fileName());
    }
}
