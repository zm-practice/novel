#include "KeywordDialog.h"

#include "KeywordManager.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QColor>

KeywordDialog::KeywordDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle("关键词管理");
    resize(500, 400);
    m_table = new QTableWidget(0, 4, this);
    m_table->setHorizontalHeaderLabels({"关键词", "颜色", "正则", "操作"});
    m_table->horizontalHeader()->setStretchLastSection(true);

    auto* btnAdd   = new QPushButton("新增");
    auto* btnClose = new QPushButton("关闭");
    auto* layBtn   = new QHBoxLayout;
    layBtn->addWidget(btnAdd);
    layBtn->addStretch();
    layBtn->addWidget(btnClose);

    auto* lay = new QVBoxLayout(this);
    lay->addWidget(m_table);
    lay->addLayout(layBtn);

    connect(btnAdd, &QPushButton::clicked, this, &KeywordDialog::onAdd);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
    connect(this, &QDialog::accepted, this, &KeywordDialog::onApply);

    KeywordManager::instance().load();

    refreshTable();
}

void KeywordDialog::refreshTable()
{
    m_table->setRowCount(0);
    const auto& rules = KeywordManager::instance().rules();
    for (int i = 0; i < rules.size(); ++i) {
        const auto& r   = rules.at(i);
        int         row = m_table->rowCount();
        m_table->insertRow(row);

        auto* le = new QLineEdit(r.word);
        le->setProperty("row", row);
        m_table->setCellWidget(row, 0, le);

        auto* btnColor = new QPushButton;
        btnColor->setFixedSize(24, 24);
        btnColor->setStyleSheet(QString("background:%1; border:none;").arg(r.color.name()));
        btnColor->setProperty("row", row);
        m_table->setCellWidget(row, 1, btnColor);

        auto* cb = new QCheckBox;
        cb->setChecked(r.useRegex);
        cb->setProperty("row", row);
        cb->setCheckable(true);
        m_table->setCellWidget(row, 2, cb);

        auto* btnDel = new QPushButton("删除");
        btnDel->setProperty("row", row);
        m_table->setCellWidget(row, 3, btnDel);

        connect(btnColor, &QPushButton::clicked, this, [=] {
            QColor c = QColorDialog::getColor(Qt::red, this, "选择颜色");
            if (c.isValid()) {
                btnColor->setStyleSheet(QString("background:%1; border:none;").arg(c.name()));
            }
        });
        connect(btnDel, &QPushButton::clicked, this, [=] {
            int idx = btnDel->property("row").toInt();
            KeywordManager::instance().removeRule(idx);
            refreshTable();  // 实时刷新
        });
    }
}

void KeywordDialog::onAdd()
{
    KeywordRule r;
    r.word     = "新关键词";
    r.color    = Qt::red;
    r.useRegex = false;
    KeywordManager::instance().addRule(r);
    refreshTable();
}

void KeywordDialog::onApply()
{
    /* 把表格写回 Manager */
    QVector<KeywordRule> tmp;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        QString word = qobject_cast<QLineEdit*>(m_table->cellWidget(i, 0))->text().trimmed();
        if (word.isEmpty()) {
            continue;
        }

        QString sheet = qobject_cast<QPushButton*>(m_table->cellWidget(i, 1))->styleSheet();          // 例如 "background:#ff0000"
        QRegularExpression re("#([0-9a-fA-F]{6})");
        QRegularExpressionMatch match = re.match(sheet);
        QColor color;
        if (match.hasMatch())
            color = QColor(match.captured(0));

        bool   regex = qobject_cast<QCheckBox*>(m_table->cellWidget(i, 2))->isChecked();
        tmp.append({word, color, regex});
    }
    KeywordManager::instance().setRules(tmp);
}
