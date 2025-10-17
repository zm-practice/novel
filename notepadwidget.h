#ifndef NOTEPADWIDGET_H
#define NOTEPADWIDGET_H

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QTextStream>
#include <QVBoxLayout>
#include <QWidget>

/* 一个极简的“笔记区”：
   1. 顶部单行输入 = 笔记标题
   2. 中间多行编辑 = 笔记正文
   3. 底部按钮      = 保存 / 清空
   保存文件默认放在程序运行目录 notes/ 下，文件名 = 标题 + 时间戳.txt
*/
class NotePadWidget : public QWidget
{
    Q_OBJECT
public:
    explicit NotePadWidget(QWidget* parent = nullptr) : QWidget(parent)
    {
        // 控件
        m_title = new QLineEdit(this);
        m_title->setPlaceholderText("请输入笔记标题");
        m_edit = new QTextEdit(this);
        m_edit->setPlaceholderText("在这里写笔记……");
        m_save  = new QPushButton("保存", this);
        m_clear = new QPushButton("清空", this);

        // 底部按钮横条
        auto* hbox = new QHBoxLayout();
        hbox->addWidget(m_save);
        hbox->addWidget(m_clear);

        // 整体竖直布局
        auto* vbox = new QVBoxLayout(this);
        vbox->addWidget(m_title);
        vbox->addWidget(m_edit);
        vbox->addLayout(hbox);
        setLayout(vbox);

        // 信号
        connect(m_save, &QPushButton::clicked, this, &NotePadWidget::saveNote);
        connect(m_clear, &QPushButton::clicked, m_edit, &QTextEdit::clear);
    }

private slots:
    void saveNote()
    {
        QString title = m_title->text().trimmed();
        if (title.isEmpty()) {
            QMessageBox::warning(this, "提示", "标题不能为空！");
            return;
        }
        QDir dir("notes");
        if (!dir.exists())
            dir.mkpath(".");
        QString fileName =
            QString("notes/%1_%2.txt").arg(title).arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this, "错误", "无法保存文件！");
            return;
        }
        QTextStream ts(&file);
        ts << m_edit->toPlainText();
        file.close();
        QMessageBox::information(this, "成功", "笔记已保存到：\n" + fileName);
    }

private:
    QLineEdit*   m_title;
    QTextEdit*   m_edit;
    QPushButton* m_save;
    QPushButton* m_clear;
};

#endif  // NOTEPADWIDGET_H
